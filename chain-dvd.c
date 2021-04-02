/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: chain-dvd.c,v 1.31 2006/09/16 18:33:36 lordjaxom Exp $
 */

#include "chain-dvd.h"
#include "burn.h"
#include "logger-vdr.h"
#include "jobs.h"
#include "render.h"
#include "config.h"
#include "menuburn.h"
#include "filter.h"
#include "setup.h"
#include "etsi-const.h"
#include "proctools/shellescape.h"
#include "proctools/shellprocess.h"
#include "proctools/functions.h"
#include "proctools/logger.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <vdr/recording.h>
#include <sys/stat.h>

namespace vdr_burn
{
	using namespace std;
	using boost::bind;
	using proctools::shellescape;
	using proctools::shellprocess;
	using proctools::logger;
	using proctools::process;

	chain_dvd::chain_dvd(job& job_):
			chain_vdr( "DVD", job_ ),
			m_step( build ),
			m_currentRecording( get_recordings().begin() ),
			m_pxAudioIndex( 0 ),
			m_totalDone( 0 ),
			m_lastRecSize( 0 ),
			m_lastProgress( 0 )
	{
	}

	void chain_dvd::process_line(const string& line)
	{
		static const string px_found_id( "++> " );
		static const string px_found_pid( ": PID " );
		static const string px_found_pesid( " / PesID " );
		static const string stat_vobu( "STAT: VOBU " );
		static const string stat_fixing( "STAT: fixing VOBU at " );
		static const string burn_done( "% done, estimate finish" );

		switch (m_step) {
		case build:
			{
				// [demux] ++> Mpg Video: PID 0x006E / PesID 0xE0 / SubID 0x00 :
				// [demux] ++> AC3/DTS Audio: PID 0x007D / PesID 0xBD / SubID 0x00 :
				// [demux] ++> Teletext: PID 0x0082 / PesID 0xBD / SubID 0x10 :
				// [demux] ++> Mpg Audio: PID 0x0079 / PesID 0xC1 / SubID 0x00 :
				// [demux] ++> Mpg Audio: PID 0x0078 / PesID 0xC0 / SubID 0x00 :
				// [demux] ++> Subpicture: PID 0x0083 / PesID 0xBD / SubID 0x20 :

				if (line.substr(0, px_found_id.length()) == px_found_id) {
					size_t found = line.find(px_found_pid);
					string type = line.substr (px_found_id.length(), found - px_found_id.length());
					logger::info( str( boost::format( "px found type: %s" ) % type) );

					istringstream pidparser( line.substr(found + px_found_pid.length()) );
					int pid;
					pidparser >> setbase(16) >> pid;

					found = line.find(px_found_pesid);
					istringstream pesidparser( line.substr(found + px_found_pesid.length()) );
					int pesid;
					pesidparser >> setbase(16) >> pesid;

					if (pesid >= 0xe0 && pesid <= 0xef) { // video stream
						//m_currentRecording->set_track_path( pid ? pid : pesid, "vdrsync.mpv");
						logger::info( str( boost::format( "px found id: 0x%04x/0x%02x, vdrsync.mpv" ) % pid % pesid ) );
					}
					else if (pesid >= 0xc0 && pesid <= 0xdf) {
						ostringstream filename;
						filename << "vdrsync";
						if (m_pxAudioIndex > 0) {
							filename << "-" << setw(2) << setfill('0') << m_pxAudioIndex+1;
						}
						filename << ".mpa";
						m_currentRecording->set_track_path( pid ? pid : pesid, filename.str());
						logger::info( str( boost::format( "px found id: 0x%04x/0x%02x, %s" ) % pid % pesid  % filename.str()) );
						++m_pxAudioIndex;
					}
					else if (pesid == 0xbd && string::npos != type.find ("AC3", 0)) { // private stream
						m_currentRecording->set_track_path( pid ? pid : pesid, "vdrsync.ac3");
						logger::info( str( boost::format( "px found id: 0x%04x/0x%02x, vdrsync.ac3" ) % pid % pesid ) );
					}
					//else
					//	logger::info( str( boost::format( "px found id: 0x%04x/0x%02x (unhandled)" ) % pid % pesid ) );
				}
				else if (line.substr(0, stat_vobu.length()) == stat_vobu) {
					if (!elapsed_since(m_lastProgress, 1))
						return;

					istringstream parser(line.substr(stat_vobu.length()));
					int mbDone;
					parser >> mbDone;
					parser.ignore(4); // " at "
					parser >> mbDone;

					double done = (100.f * m_totalDone
								  + 90.f * get_job().get_requant_factor() * mbDone * MEGABYTE(1))
								  / get_job().get_tracks_size() / 2.f;

					set_progress(static_cast<int>(done));
				}
				else if (line.substr(0, stat_fixing.length()) == stat_fixing) {
					if (!elapsed_since(m_lastProgress, 1))
						return;

					istringstream parser(line.substr(stat_fixing.length()));
					int mbDone;
					parser >> mbDone;

					double done = (100.f * (m_totalDone - m_lastRecSize)
								  + 90.f * m_lastRecSize
								  + 10.f * get_job().get_requant_factor() * mbDone * MEGABYTE(1))
								  / get_job().get_tracks_size() / 2.f;

					set_progress(static_cast<int>(done));
				}
			}
			break;

		case burn:
			{
				istringstream parser(line);
				double percentDone;
				parser >> percentDone;
				protected_array<char> rest( new char[ burn_done.length() + 1 ] );
				parser.get(rest.get(), burn_done.length() + 1, '\n');
				if (rest.get() == burn_done) {
					set_progress(static_cast<int>(50.f + percentDone / 2.f));
					set_burn_progress(static_cast<int>(percentDone));
				}
			}
			break;
		};
	}

	bool chain_dvd::initialize()
	{
		return prepare_job();
	}

	bool chain_dvd::finished(const process* proc)
	{
		if (proc->return_status() == process::ok) {
			// positive return
			if (proc->name() == "demux")
				prepare_mplex();
			else if (proc->name() == "mplex") {
				m_lastRecSize = m_currentRecording->get_tracks_size();
				m_totalDone += m_lastRecSize;
				if (m_step == build) {
					// prevent this to happen again when author quits before mplex
					++m_currentRecording;
					if (m_currentRecording != get_recordings().end())
						prepare_demux();
				}
			}
			else if (proc->name() == "author") {
				m_step = burn;
#ifndef ENABLE_DMH_ARCHIVE
				return prepare_burning();
#else
				if(get_job().get_dmh_archive_mode()) {
					m_currentRecording = get_recordings().begin();
					return prepare_dmh_archive();
				} else {
					// no DMH-archive wanted, so go directly to burning
					return prepare_burning();
				}
#endif
			}
#ifdef ENABLE_DMH_ARCHIVE
			else if (proc->name() == "dmharchive") {
				++m_currentRecording;
				if (m_currentRecording != get_recordings().end()) {
					return prepare_dmh_archive();
				} else {
					return prepare_archive_mark();
				}
			}
			else if (proc->name() == "archivemark") {
				return prepare_burning();
			}
#endif
			return true;
		}

		// process failed
		// XXX determine if growisofs or mkisofs failed
		if (proc->name() == "burn" && prepare_device())
			return prepare_burning();

		return false;
	}

	void chain_dvd::create_files()
	{
		for_each(get_recordings().begin(), get_recordings().end(),
				 bind( &chain_dvd::create_files, this, _1 ));
	}

	void chain_dvd::create_files(const recording& rec)
	{
		// temp folders
		make_dir(rec.get_paths().temp);
		make_dir(rec.get_paths().data);

		// fifos for video track and muxed movie
		make_fifo(rec.get_movie_path());
		make_fifo(rec.get_requant_path());

		// fifos for each audio track
		const_track_filter audioTracks( rec.get_tracks(), track_info::streamtype_audio, track_predicate::used );
		for_each(audioTracks.begin(), audioTracks.end(),
				 bind( &chain_dvd::make_fifo, this,
				 	   bind( &recording::get_track_path, rec, _1 )
				));
	}

	bool chain_dvd::prepare_job()
	{
		create_files();

		if (get_job().get_disk_type() == disktype_dvd_menu) {
			// render menu's
			renderer render_menus( get_job(), *this );

#warning error handling render_menus
			if ( !render_menus() ) {
				logger::error("rendering menu failed - aborting job");
				return false;
			}
		}

		dvdauthor_xml xml(get_job());
		xml.write();

		// prepare first recording
		prepare_demux();

		// start authoring
		shellprocess* author = new shellprocess( "author", shellescape( "vdrburn-dvd.sh" ) + "author" );
		make_dir(xml.get_author_path());
		author->put_environment("DVDAUTHOR_XML",  xml.get_xml_path());
		author->put_environment("DVDAUTHOR_PATH", xml.get_author_path());
		author->put_environment("ISO_FILE",       get_job().get_iso_path());
		author->put_environment("DVD_DEVICE",     BurnParameters.DvdDevice);
		author->put_environment("VIDEO_FORMAT",   (25.0 == m_currentRecording->get_FramesPerSecond() ? "PAL" : "NTSC"));
		add_process(author);
		return true;
	}

	void chain_dvd::prepare_demux()
	{
		m_pxAudioIndex = 0;

		// processes
		shellprocess* demux = new shellprocess( "demux", shellescape( "vdrburn-dvd.sh" ) + "demux" );
		demux->put_environment("RECORDING_PATH", m_currentRecording->get_filename());
		demux->put_environment("IGNORE_TRACKS",  m_currentRecording->get_ignored_cids());
		demux->put_environment("USED_TRACKS",    m_currentRecording->get_used_cids());
		demux->put_environment("MPEG_TMP_PATH",  m_currentRecording->get_paths().temp);
		demux->put_environment("MPEG_DATA_PATH", m_currentRecording->get_paths().data);
		demux->put_environment("CONFIG_PATH",    plugin::get_config_path());
		if (get_job().get_cut_on_demux() && prepare_cutmarks())
			demux->put_environment("USE_CUTTING", "1");
#ifdef TTXT_SUBTITLES
		demux->put_environment("TTXT_OPTS",  m_currentRecording->get_TtxtPageOpts());
#endif
		add_process(demux);
	}

	void chain_dvd::prepare_mplex()
	{
		// insert requant step if required
		if (get_job().get_requant_factor() > 1) {
			const char* requant_call;
			if ( global_setup().RequantType == requanttype_transcode )
				requant_call = "tcrequant";
			else if ( global_setup().RequantType == requanttype_m2vrequantiser )
				requant_call = "M2Vrequantiser";
			else
				requant_call = "lxrequant";

			shellprocess* requant = new shellprocess( "requant", shellescape( "vdrburn-dvd.sh" ) + requant_call );
			requant->put_environment("VIDEO_FILE",     m_currentRecording->get_video_track_path());
			requant->put_environment("REQUANT_FILE",   m_currentRecording->get_requant_path());
			requant->put_environment("REQUANT_FACTOR", get_job().get_requant_factor());
			requant->put_environment("VIDEO_SIZE",     m_currentRecording->get_tracks_size( track_info::streamtype_video ));
			add_process(requant);
		}

		/// subtitle handling
		const_track_filter subtitleTracks( m_currentRecording->get_tracks(), track_info::streamtype_subtitle, track_predicate::used );
		int subtitletrack_no = 0;
		const_track_filter::iterator subtitleTrack = subtitleTracks.begin();
		while  (subtitleTrack != subtitleTracks.end()) {
			make_fifo( m_currentRecording->get_subtitle_path(subtitletrack_no) );
			shellprocess* subtitle = new shellprocess( str ( boost::format( "subtitle%d" ) % subtitletrack_no), shellescape( "vdrburn-dvd.sh" ) + "subtitle" );
			subtitle->put_environment("NUMBER",         subtitletrack_no);
			if (subtitleTrack->subtitle.type == track_info::subtitletype_teletext)
				subtitle->put_environment("TTXTPAGE",       subtitleTrack->subtitle.teletextpage);
			subtitle->put_environment("MPEG_DATA_PATH", m_currentRecording->get_paths().data);
			subtitle->put_environment("SPUMUX_FILE",    m_currentRecording->get_spumux_path(subtitletrack_no));
			subtitle->put_environment("MOVIE_FILE",     m_currentRecording->get_subtitle_path(subtitletrack_no));
			subtitleTrack++;
			subtitle->put_environment("SUBTITLED_FILE",
				(subtitleTrack == subtitleTracks.end())
					? m_currentRecording->get_movie_path()
					: m_currentRecording->get_subtitle_path(subtitletrack_no+1));
			add_process(subtitle);
			subtitletrack_no++;
		}

		shellprocess* mplex = new shellprocess( "mplex", shellescape( "vdrburn-dvd.sh" ) + "mplex" );
		mplex->put_environment("MOVIE_FILE",
				subtitleTracks.begin() != subtitleTracks.end()
					? m_currentRecording->get_subtitle_path(0)
					: m_currentRecording->get_movie_path());

		mplex->put_environment("MPEG_DATA_PATH", m_currentRecording->get_paths().data);
		mplex->put_environment("MPEG_TMP_PATH",  m_currentRecording->get_paths().temp);
		mplex->put_environment("VIDEO_FILE",
				get_job().get_requant_factor() > 1
					? m_currentRecording->get_requant_path()
					: m_currentRecording->get_video_track_path());
		mplex->put_environment("AUDIO_FILES",    m_currentRecording->get_audio_paths());
		add_process(mplex);
	}

	bool chain_dvd::prepare_device()
	{
		if (get_job().get_store_mode() == storemode_create)
			return false; // no device needed for this mode

		shellprocess eject( "eject", shellescape( "eject" ) + BurnParameters.DvdDevice );
		execute(eject);
		if (eject.return_status() != process::ok)
			return false;

		Skins.QueueMessage(mtInfo, tr("Please enter the burn plugin"), 10, -1);
		bool result = menu::pagebase::wait_for_user(tr("Please insert an empty disk and press Ok"));
		Skins.QueueMessage(mtInfo, 0, 0, 0);
		return result;
		/*if (get_job().get_store_mode() == smCreateISO)
			return true;

		while (true) {
			wrappers::cdio::device cdio(BurnParameters.DvdDevice);
			if (!cdio && errno != ENOMEDIUM) {
				logger::error(format("error while accessing {0}")
							  % BurnParameters.DvdDevice);
				return false;
			}

			switch (cdio.get_media_status()) {
			case wrappers::cdio::writeable:
			case wrappers::cdio::unknown:
				return true;

			case wrappers::cdio::no_dvd:
			case wrappers::cdio::readable:
				if (!cdio.eject()) {
					logger::error(format("error while ejecting {0}")
								  % BurnParameters.DvdDevice);
					return false;
				}

			case wrappers::cdio::error:
				Skins.QueueMessage(mtInfo, tr("Please enter the burn plugin"),
								   10, -1);
				if (!menu::pagebase::wait_for_user(tr("Please insert an empty "
						 						      "disk and press Ok")))
					return false;
				break;

			case wrappers::cdio::unknown:
				logger::error(format("unknown condition while accessing {0}")
							  % BurnParameters.DvdDevice);
				return false; XXX Double Layer
			}
		}
		return true;*/
	}

	bool chain_dvd::prepare_burning()
	{
		switch (get_job().get_store_mode()) {
		case storemode_create:
			{
				shellprocess* burn = new shellprocess( "burn", shellescape( "vdrburn-dvd.sh" ) + "mkiso" );
				burn->put_environment("DVDAUTHOR_PATH",
								  dvdauthor_xml::get_author_path(get_job()));
				burn->put_environment("ISO_FILE", get_job().get_iso_path());
				burn->put_environment("DISC_ID", get_job().get_volume_id());
				add_process(burn);
			}
			break;

		case storemode_burn:
			{
				shellprocess* burn = new shellprocess( "burn", shellescape( "vdrburn-dvd.sh" ) + "burndir" );
				burn->put_environment("DVDAUTHOR_PATH", dvdauthor_xml::get_author_path(get_job()));
				burn->put_environment("DVD_DEVICE", BurnParameters.DvdDevice);
				burn->put_environment("BURN_SPEED", global_setup().BurnSpeed);
				burn->put_environment("DISC_ID", get_job().get_volume_id());
				add_process(burn);
			}
			break;

		case storemode_createburn:
			{
				boost::format fifofmt( "%s/burnfifo" );
				fifofmt % get_paths().temp;
				make_fifo( fifofmt.str() );

				shellprocess* pipe = new shellprocess( "pipe", shellescape( "vdrburn-dvd.sh" ) + "pipeiso" );
				pipe->put_environment("DVDAUTHOR_PATH", dvdauthor_xml::get_author_path(get_job()));
				pipe->put_environment("ISO_FILE",       get_job().get_iso_path());
				pipe->put_environment("ISO_PIPE",       fifofmt.str());
				pipe->put_environment("DISC_ID",        get_job().get_volume_id());
				add_process(pipe);

 				shellprocess* burn = new shellprocess( "burn", shellescape( "vdrburn-dvd.sh" ) + "burniso" );
				burn->put_environment("DVD_DEVICE", BurnParameters.DvdDevice);
				burn->put_environment("ISO_PIPE",   fifofmt.str());
				burn->put_environment("BURN_SPEED", global_setup().BurnSpeed);
				add_process(burn);
			}
			break;
		}
		set_is_burning();
		return true;
	}

	bool chain_dvd::prepare_cutmarks()
	{
		cMarks marks;
		if (!marks.Load(m_currentRecording->get_filename().c_str(), m_currentRecording->get_FramesPerSecond(), m_currentRecording->get_PesRecording()) || !marks.Count())
			return false;

		cIndexFile ifile( m_currentRecording->get_filename().c_str(), false, m_currentRecording->get_PesRecording() );

		// convert marks to bytepos
		ofstream cutfile( str( boost::format("%s/px.cut") % m_currentRecording->get_paths().data ).c_str() );

		for (cMark *mark = marks.First(); mark != 0; mark = marks.Next(mark)) {
			int pos = mark->Position();
			uint16_t filenumber;
			off_t offset;

			ifile.Get(pos, &filenumber, &offset);
			uint64_t bytepos = offset;
			boost::format filefmt( m_currentRecording->get_PesRecording() ? "%s/%03d.vdr" :  "%s/%05d.ts" );

			for (int i = 1; i < filenumber; ++i) {
				boost::format vdrfile( filefmt );
				vdrfile % m_currentRecording->get_filename() % i;

				struct stat filebuf;
				if ( stat( vdrfile.str().c_str(), &filebuf ) < 0 ) {
					logger::error( str( boost::format("error determining size of \"%s\"") % vdrfile ) );
					return false; // so px won't complain about missing marks
				}
				bytepos += filebuf.st_size;
			}
			logger::info( str( boost::format( "mark at %s, bytepos %llu" ) % *IndexToHMSF( pos, true ) % bytepos ) );
			cutfile << bytepos << endl;
		}
		cutfile.close();
		return true;
	}

#ifdef ENABLE_DMH_ARCHIVE
	bool chain_dvd::prepare_dmh_archive()
	{
		shellprocess* dmharchive = new shellprocess( "dmharchive", shellescape( "vdrburn-dvd.sh" ) + "dmharchive" );
		dmharchive->put_environment("DVDAUTHOR_PATH", dvdauthor_xml::get_author_path(get_job()));
		dmharchive->put_environment("RECORDING_PATH", m_currentRecording->get_filename());
		dmharchive->put_environment("TEMP_PATH",  m_currentRecording->get_paths().temp);
		dmharchive->put_environment("CONFIG_PATH", plugin::get_config_path());

		string track = str( boost::format( "%02d" ) % ( std::distance( get_recordings().begin(), m_currentRecording ) + 1 ) );
		dmharchive->put_environment("TRACK_ON_DVD", track);

		add_process(dmharchive);
		return true;
	}

	bool chain_dvd::prepare_archive_mark()
	{
		shellprocess* archivemark = new shellprocess( "archivemark", shellescape( "vdrburn-dvd.sh" ) + "archivemark" );
		archivemark->put_environment("DVDAUTHOR_PATH", dvdauthor_xml::get_author_path(get_job()));
		archivemark->put_environment("CONFIG_PATH",    plugin::get_config_path());

		add_process(archivemark);
		return true;
	}
#endif
}
