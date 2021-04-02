/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: scanner.c,v 1.9 2006/09/16 18:33:36 lordjaxom Exp $
 */

#include "adaptor.h"
#include "scanner.h"
#include "filter.h"
#include "etsi-const.h"
#include "proctools/logger.h"
#include "proctools/format.h"
#include "proctools/functions.h"
#include "genindex/pes.h"
#include "boost/bind.hpp"
#include <cctype>
#include <numeric>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <list>
#include <dirent.h>
#include <sys/stat.h>
#include "vdr/remux.h"
#include "vdr/device.h"

//#define DEBUG(x...) isyslog(x)
//#define DEBUG(x...) printf(x)
#define DEBUG(x...)

namespace vdr_burn
{

    using namespace std;
    using proctools::logger;
    using proctools::format;
    using proctools::convert;
    using proctools::sum;
    using boost::bind;

	// std::isdigit seems to be a macro or something here ?!?
    bool myisdigit(char ch) { return std::isdigit(ch); }

	// --- marks_scanner -------------------------------------------------------

    class marks_scanner
    {
        struct state {
            state(marks_scanner& owner): m_owner( owner ) { }
            virtual ~state() { }
            virtual state* process(int index) = 0;
            marks_scanner& m_owner;
        };

        friend struct state;

        struct inpoint: state {
            inpoint(marks_scanner& owner): state( owner ) { }
            virtual state* process(int index);
        };

        struct outpoint: state {
            outpoint(marks_scanner& owner, int lastIndex):
					state( owner ), m_lastIndex( lastIndex ) { }
            virtual state* process(int index);
            int m_lastIndex;
        };

    public:
        marks_scanner(recording_index& index):
                m_index( index ), m_totalSize( 0 ), m_totalLength( 0 ),
                m_state( new inpoint( *this ) ) { }
        marks_scanner(const marks_scanner& right):
                m_index( right.m_index ), m_totalSize( right.m_totalSize ), m_totalLength( right.m_totalLength ),
                m_state( 0 ) { }

        void operator()(const cMark& mark);

        uint64_t get_total_size() const { return m_totalSize; }
        int get_total_length() const { return m_totalLength; }

    private:
        recording_index& m_index;
        uint64_t m_totalSize;
        int m_totalLength;
        auto_ptr<state> m_state;
    };

    marks_scanner::state* marks_scanner::inpoint::process(int index)
    {
        return new outpoint( m_owner, index );
    }

    marks_scanner::state* marks_scanner::outpoint::process(int index)
    {
    	m_owner.m_totalLength += index - m_lastIndex;
    	m_owner.m_totalSize += m_owner.m_index.get_bytepos( index ) - m_owner.m_index.get_bytepos( m_lastIndex );
        return new inpoint(m_owner);
    }

    void marks_scanner::operator()(const cMark& mark)
    {
        m_state.reset( m_state->process(mark.position) );
    }

 	// --- recording_index -----------------------------------------------------

    recording_index::recording_index(const string& fileName, const bool IsPesRecording):
            m_fileName( fileName ),
#if VDRVERSNUM >= 10703
            m_index( fileName.c_str(), false, IsPesRecording )
#else
            m_index( fileName.c_str(), false )
#endif
    {
        if (!m_index.Ok()) {
            logger::error( format("couldn't read index from {0}") % m_fileName);
            throw user_exception( tr("Couldn't read index") );
        }

        scan_file_sizes();
    }

    uint64_t recording_index::get_bytepos(int index, bool frameEnd)
    {
#if VDRVERSNUM >= 10703
        uint16_t fileNo;
        off_t offset;
        int length;
        bool independant;
        if (!m_index.Get(index, &fileNo, &offset, &independant, &length))
#else
        uchar fileNo;
        int offset;
        int length;
        if (!m_index.Get(index, &fileNo, &offset, 0, &length))
#endif
            return npos;

        --fileNo;
		return accumulate( m_videoFileSizes.begin(), m_videoFileSizes.begin() + fileNo,
				uint64_t(offset) + (frameEnd ? length : 0) );
    }

    pair<int, int> recording_index::get_position(int index)
    {
#if VDRVERSNUM >= 10703
        uint16_t fileNo;
        off_t offset;
        if (!m_index.Get(index, &fileNo, &offset))
            return make_pair(-1, -1);
#else
        uchar fileNo;
        int offset;
        if (!m_index.Get(index, &fileNo, &offset))
            return make_pair(-1, -1);
#endif

        return make_pair(fileNo, offset);
    }

    void recording_index::scan_file_sizes()
    {
        DIR* videoDir = opendir(m_fileName.c_str());
        if (videoDir == 0) {
            logger::error(format( "couldn't browse {0} while trying to scan recording" ) % m_fileName);
            throw user_exception( tr("Couldn't browse recording") );
        }

        struct dirent* entry;
        char entryBuffer[offsetof(struct dirent, d_name) + NAME_MAX + 1];
        int result;
        while ((result = readdir_r(videoDir, (struct dirent*)entryBuffer, &entry)) == 0 && entry != 0) {
            string fileName( entry->d_name );
            if ((fileName.length() != 8 ||
                    fileName.find_first_not_of( "0123456789" ) != 5 ||
                    fileName.substr(5) != ".ts") &&
                (fileName.length() != 7 ||
                    //find_if(fileName.begin(), fileName.end(), not1(ptr_fun(isdigit))) != fileName.begin() + 3 ||
                    fileName.find_first_not_of( "0123456789" ) != 3 ||
                    fileName.substr(3) != ".vdr"))
                continue;

            string path( format( "{0}/{1}" ) % m_fileName % fileName );
            struct stat statBuffer;
            if (stat(path.c_str(), &statBuffer) != 0) {
                logger::error(format( "couldn't read size of {0}" ) % path);
                throw user_exception( tr("Couldn't browse recording") );
            }

            uint fileNo( convert<uint>(fileName.substr(0, fileName.length()==8 ? 5:3)) );
            if (m_videoFileSizes.size() < fileNo)
                m_videoFileSizes.resize(fileNo);
            m_videoFileSizes[fileNo - 1] = statBuffer.st_size;
        }

        if (result != 0) {
            errno = result;
            logger::error(format( "couldn't browse {0} while trying to scan recording" ) % m_fileName);
            throw user_exception( tr("Couldn't browse recording") );
        }
        closedir(videoDir);
    }

	// --- vdr_file ------------------------------------------------------------

    class vdr_file
    {
    public:
        typedef pair<int, int> position;

        vdr_file(const string& fileName,  const bool IsPesRecording);
        ~vdr_file();


        vdr_file& seek(const position& pos);
        const position& tell() { return m_position; }
        streamsize readsomePes(uchar* buffer, streamsize length);

		bool eof() const { return m_eof; }
		bool fail() const { return m_fail; }
        operator bool() const { return !m_fail; }

	protected:
		void check_eof() { seek( make_pair( m_position.first + 1, 0 ) ); }

    private:
		string m_fileName;
		int m_fd;
		bool m_eof;
		bool m_fail;
		position m_position;
		bool m_isPesRecording;
		uchar * m_tsBuffer;
		streamsize m_tsBufferLength;
		streamsize m_tsResult;
#if VDRVERSNUM >= 10703
		uchar * m_tsBufPtr;
		//cPatPmtParser patPmtParser;
		struct PidConverter {
			int Pid;
			cTsToPes *TsToPes;
			unsigned int counter;
		} ;
		typedef std::list<PidConverter> PidConverter_list;
		PidConverter_list m_PidConverters;
#endif
    };

    vdr_file::vdr_file(const string& fileName, const bool isPesRecording):
			m_fileName( fileName ),
			m_fd( -1 ),
			m_position( -1, -1 ),
			m_isPesRecording( isPesRecording ),
			m_tsBuffer( NULL ),
			m_tsBufferLength( 0 ),
			m_tsResult ( 0 )
    {
    }

	vdr_file::~vdr_file()
	{
		if (m_fd != -1) close(m_fd);
		free(m_tsBuffer); m_tsBuffer = NULL;
#if VDRVERSNUM >= 10703
		for (PidConverter_list::iterator it = m_PidConverters.begin(); it != m_PidConverters.end(); it++)
			delete it->TsToPes;
#endif
	}


    vdr_file& vdr_file::seek(const position& pos)
    {
        if (m_position.first != pos.first) {
        	if (m_fd != -1) close(m_fd);
			m_eof = m_fail = false;

			string fileName;
			if (m_isPesRecording)
				fileName = format( "{0}/{1}.vdr" ) % m_fileName % format::width(format::fill(pos.first, '0'), 3);
			else
				fileName = format( "{0}/{1}.ts" ) % m_fileName % format::width(format::fill(pos.first, '0'), 5);

			m_fd = open(fileName.c_str(), O_RDONLY);
			if (m_fd == -1) {
				if (errno == ENOENT)
					m_eof = true;
				else
					m_fail = true;
			}
		}
		DEBUG("seeking to %d\n", pos.second);
		lseek(m_fd, pos.second, SEEK_SET);
        m_position = pos;
        return *this;
    }

    streamsize vdr_file::readsomePes(uchar* buffer, streamsize length)
    {
		if (m_isPesRecording) {
	    	streamsize result = read(m_fd, buffer, length);
			if (result == 0) {
				check_eof();
				if (!m_eof && !m_fail)
					return readsomePes(buffer, length);
			} else if (result == -1) {
				m_fail = true;
				return 0;
			}

			m_position.second += result;
    		return result;
		}
#if VDRVERSNUM >= 10703
		else
		{
			//logger::debug(format("readsomeTS 0"));
			DEBUG("\nreadsomeTS %d\n", length);
			if (m_tsBufferLength < TS_SIZE * (length/184 - 1)) {
				m_tsBufferLength = TS_SIZE * (length/184 - 1);
				m_tsBuffer = (uchar *)realloc (m_tsBuffer, m_tsBufferLength);
				if (!m_tsBuffer) {
					m_tsBufferLength = 0;
					return 0;
				}
			}

			int PesLength;
			const uchar * PesBuffer = NULL;
			streamsize pesresult = 0;
			PidConverter_list::iterator it;

			while (0 == pesresult) {

				if(m_tsResult < TS_SIZE) {
					m_tsResult = read(m_fd, m_tsBuffer, m_tsBufferLength);
					//logger::debug(format("read {0} from file") % m_tsResult );
					if (m_tsResult == 0) {
						check_eof();
						if (!m_eof && !m_fail)
							return readsomePes(buffer, length);
					} else if (m_tsResult == -1) {
						m_fail = true;
						return 0;
					}
					m_tsBufPtr = m_tsBuffer;
					DEBUG("  read %d from file, buffer was %d\n", m_tsResult, oldtsResult);
				}

				int Pid = TsPid(m_tsBufPtr);

				for (it = m_PidConverters.begin(); it != m_PidConverters.end() && it->Pid != Pid; it++);

				if (it==m_PidConverters.end() || it->Pid != Pid) {
					PidConverter newstrm;
					newstrm.Pid = Pid;
					newstrm.TsToPes = new cTsToPes();
					newstrm.counter = 0;
					m_PidConverters.push_back(newstrm);
					it=m_PidConverters.end();
					it--;
				}

				if (TsPayloadStart(m_tsBufPtr)) {
					while ((PesBuffer = it->TsToPes->GetPes(PesLength))) {
						DEBUG("  GetPes1 PID=%4X %5d PesLen=%5d\n", it->Pid, (m_tsBufPtr-m_tsBuffer), PesLength);
						if (pesresult+PesLength > length)
							esyslog("burn:  GetPes BUFFER OVERFLOW %d %d %d\n", pesresult ,PesLength, length);
						else
							memcpy(buffer+pesresult, PesBuffer, PesLength);
							pesresult = pesresult+PesLength > length? length:pesresult+PesLength;
							return pesresult;
					}
					it->TsToPes->Reset();
				}

				it->TsToPes->PutTs(m_tsBufPtr, TS_SIZE);
				it->counter++;

				m_tsBufPtr += TS_SIZE;
				m_tsResult -= TS_SIZE;
				m_position.second += TS_SIZE;
			}

			DEBUG("returning %5d, remaining %5d\n", pesresult, m_tsBufferLength-(m_tsBufPtr-m_tsBuffer));
			return pesresult;
		}
#endif
		return 0;
    }

	// --- pes_scanner ---------------------------------------------------------

    class pes_scanner: public cPES
    {
		struct payload_header
		{
			const uchar* data;
			size_t length;
		};

    public:
        pes_scanner(const std::string& fileName, const bool IsPesRecording);

        track_info_list& scan(const vdr_file::position& first, const vdr_file::position& last);

    protected:
        virtual int Action1(uchar type, uchar* data, int length);
        virtual int Action2(uchar type, uchar* data, int length);
        virtual int Action3(uchar type, uchar* data, int length);

        template<unsigned int N>
        uchar* find_payload(uchar* data, int length, const payload_header(&patterns)[N]);

    private:
        track_info_list m_tracks;
        vdr_file m_vdrFile;
        int m_audioTracks;
    };

	static const int audio_bitrates[][16] =
		// table id: layerBit is bit 0, (typeBits - 1) are bits 1, 2
		  // mpeg 2 layer 3
		{ { -1, 8, 16, 24, 32, 64, 80, 56, 64, 128, 160, 112, 128, 256, 320, -1 },
		  // mpeg 1 layer 3
		  { -1, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1 },
		  // mpeg 2 layer 2
		  { -1, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1 },
		  // mpeg 1 layer 2
		  { -1, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1 },
		  // mpeg 2 layer 1
		  { -1, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1 },
		  // mpeg 1 layer 1
		  { -1, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1 } };

	static const int ac3_bitrates[] =
		{ 32, 32, 40, 40, 48, 48, 56, 56, 64, 64, 80, 80, 96, 96,
		  112, 112, 128, 128, 160, 160, 192, 192, 224, 224, 256, 256,
		  320, 320, 384, 384, 448, 448, 512, 512, 576, 576, 640, 640 };

    pes_scanner::pes_scanner(const std::string& fileName, const bool IsPesRecording ):
            m_vdrFile( fileName, IsPesRecording ),
            m_audioTracks( 0 )
    {
        SetDefaultRule(prPass);
        SetRuleR(0xe0, 0xef, prAct1);
        SetRuleR(0xc0, 0xcf, prAct2);
        SetRule(0xbd, prAct3);
    }

	track_info_list& pes_scanner::scan(const vdr_file::position& first, const vdr_file::position& last)
    {
		logger::debug(format( "scanning positions: {0}/{1}-{2}/{3}") % first.first % first.second % last.first % last.second);

        m_vdrFile.seek(first);
        while (m_vdrFile && !m_vdrFile.eof()) {
            uchar buffer[KILOBYTE(64)];
            int length = m_vdrFile.readsomePes( buffer, sizeof(buffer) );
			if (length > 0)
				Process(buffer, length);

            if (last <= m_vdrFile.tell())
				break;
        }
        return m_tracks;
    }

    template<unsigned int N>
    uchar* pes_scanner::find_payload(uchar* data, int length, const payload_header (&patterns)[N])
    {
		for (unsigned int i = 0; i < N; ++i) {
			uchar* payload = search(data, data + length, patterns[i].data, patterns[i].data + patterns[i].length);
			if (payload != data + length)
				return payload;
		}
		return 0;
    }

    int pes_scanner::Action1(uchar type, uchar* data, int length)
    {
        static const payload_header seqheader[] = { { reinterpret_cast<const uchar*>("\x00\x00\x01\xb3"), 4 } };

        if (m_tracks.find_cid( type ) != m_tracks.end())
            return length;

		uchar* payload = find_payload(data, length, seqheader);
		if (payload == 0 || payload + 16 >= data + length) // no header found or frame too small
			return length;
		payload += seqheader[0].length;

		track_info track( type, track_info::streamtype_video );
        logger::debug(format( "found video stream {0}, vdrsync.mpv" ) % format::base(int( type ), 16));

		ostringstream videoformat;

        ushort aspectBits = (payload[3] & 0xf0) >> 4;
        switch (aspectBits) {
        case 1: track.video.aspect = track_info::aspectratio_quadric; videoformat << "aspect = quadric,"; break;
        case 2: track.video.aspect = track_info::aspectratio_4_3; videoformat << "aspect = 4:3,"; break;
        case 3: track.video.aspect = track_info::aspectratio_16_9; videoformat << "aspect = 16:9,"; break;
        case 7: track.video.aspect = track_info::aspectratio_21_9; videoformat << "aspect = 2.21:1,"; break;
        default: track.video.aspect = track_info::aspectratio( 0 ); videoformat << "aspect = unknown (" << aspectBits << "),";
        }

        ushort fpsBits = (payload[3] & 0x0f);
        switch (fpsBits) {
        case 1: track.video.frameRate = track_info::framerate_23_967; videoformat << " 23.967 frames/s,"; break;
        case 2: track.video.frameRate = track_info::framerate_24; videoformat << " 24 frames/s,"; break;
        case 3: track.video.frameRate = track_info::framerate_25; videoformat << " 25 frames/s,"; break;
        case 4: track.video.frameRate = track_info::framerate_29_97; videoformat << " 29.97 frames/s,"; break;
        case 5: track.video.frameRate = track_info::framerate_30_97; videoformat << " 30.97 frames/s,"; break;
        default: track.video.frameRate = track_info::framerate( 0 ); videoformat << " unknown framerate (" << fpsBits << "),";
        }

        int bitrateBits = ((payload[4] << 10) | (payload[5] << 2) | ((payload[6] & 0xc0) >> 6)) * 400;
        track.bitrate = bitrateBits;
        videoformat << " bitrate = " << bitrateBits/1000000 << " MBits/s";

		logger::debug(format ("{0}") % videoformat.str());
        //XXX
        track.filename = "vdrsync.mpv";

		m_tracks.push_front( track );
        return length;
    }

    int pes_scanner::Action2(uchar type, uchar* data, int length)
    {
		static const payload_header frameheaders[] = { { reinterpret_cast<const uchar*>("\xff\xfc"), 2 },
													   { reinterpret_cast<const uchar*>("\xff\xfd"), 2 } };

        if (m_tracks.find_cid( type ) != m_tracks.end())
            return length;

		uchar* payload = find_payload(data, length, frameheaders);
		if (payload == 0 || payload + 8 >= data + length) // no header found or frame too small
			return length;

		track_info track( type, track_info::streamtype_audio );
		logger::debug( format("found audio stream {0}, vdrsync{1}.mpa") % format::base( int(type), 16 ) % m_audioTracks);

		ostringstream audioformat;

		ushort typeBits = (payload[1] & 0x18) >> 3;
		switch (typeBits) {
		case 0: track.audio.type = track_info::audiotype_mpeg_2_5; audioformat << "streamtype = mpeg2.5"; break;
		case 2: track.audio.type = track_info::audiotype_mpeg_2; audioformat << "streamtype = mpeg2"; break;
		case 3: track.audio.type = track_info::audiotype_mpeg_1; audioformat << "streamtype = mpeg,"; break;
		default: track.audio.type = track_info::audiotype( 0 ); audioformat << "streamtype = unknown (" << typeBits << "),";
		}

		ushort layerBits = (payload[1] & 0x06) >> 1;
		switch (layerBits) {
		case 1: track.audio.mpeg.layer = track_info::audiolayer_3; audioformat << " layer 3, "; break;
		case 2: track.audio.mpeg.layer = track_info::audiolayer_2; audioformat << " layer 2, "; break;
		case 3: track.audio.mpeg.layer = track_info::audiolayer_1; audioformat << " layer 1, "; break;
		default: track.audio.mpeg.layer = track_info::audiolayer( 0 ); audioformat << " layer unknown (" << layerBits << "), ";
		}

		ushort bitrateBits = (payload[2] & 0xf0) >> 4;
		ushort bittableIndex = ((layerBits - 1) << 1) | typeBits;
		if (bittableIndex >= sizeof(audio_bitrates) / sizeof(audio_bitrates[0]) ||
				audio_bitrates[bittableIndex][bitrateBits] == -1) {
			logger::debug( "no valid bitrate information found, trying again later" );
			return length;
		}
		track.bitrate = audio_bitrates[bittableIndex][bitrateBits];
		audioformat << audio_bitrates[bittableIndex][bitrateBits] << " kBit/s, ";

		ushort modeBits = (payload[3] & 0xc0) >> 6;
		switch (modeBits) {
		case 0: track.audio.mpeg.mode = track_info::audiomode_stereo; audioformat << "stereo"; break;
		case 1: track.audio.mpeg.mode = track_info::audiomode_joint_stereo; audioformat << "joint_stereo"; break;
		case 2: track.audio.mpeg.mode = track_info::audiomode_dual_channel; audioformat << "dual_channel"; break;
		case 3: track.audio.mpeg.mode = track_info::audiomode_mono; audioformat << "mono"; break;
		default: track.audio.mpeg.mode = track_info::audiomode( 0 ); audioformat << "mode unknown (" << modeBits << ")";
		}

		logger::debug(format ("{0}") % audioformat.str());
		// XXX
		track.filename = format( "vdrsync{0}.mpa" ) % m_audioTracks++;
		track.description = tr("analog");

		m_tracks.push_back( track );
        return length;
    }

    int pes_scanner::Action3(uchar type, uchar* data, int length)
    {
    	static const payload_header frameheader[] = { { reinterpret_cast<const uchar*>( "\x0b\x77" ), 2 } };

        if (m_tracks.find_cid( type ) != m_tracks.end())
            return length;

		uchar* payload = find_payload(data, length, frameheader);
		if (payload == 0 || payload + 8 >= data + length) // no header found or frame too small
			return length;

		track_info track( type, track_info::streamtype_audio );
		track.audio.type = track_info::audiotype_ac3;
		logger::debug( format("found ac3 stream {0}, vdrsync.ac3") % format::base( int(type), 16 ) );

		ostringstream audioformat;

		ushort bitrateBits = (payload[4] & 0x3f);
		if (bitrateBits >= sizeof(ac3_bitrates) / sizeof(ac3_bitrates[0])) {
			logger::debug( "no valid bitrate information found, trying again later" );
			return length;
		}
		track.bitrate = ac3_bitrates[bitrateBits];
		audioformat << "bitrate = " << ac3_bitrates[bitrateBits] << " kBit/s";

		ushort modeBits = payload[6] >> 5;
		switch (modeBits) {
		case 0: audioformat << ", mode = stereo"; break;
		case 1: audioformat << ", mode = 1/0"; break;
		case 2: audioformat << ", mode = 2/0"; break;
		case 3: audioformat << ", mode = 3/0"; break;
		case 4: audioformat << ", mode = 2/1"; break;
		case 5: audioformat << ", mode = 3/1"; break;
		case 6: audioformat << ", mode = 2/2"; break;
		case 7: audioformat << ", mode = 3/2"; break;
		default: audioformat << ", mode = unknown (" << modeBits << ")";
		}

		logger::debug(format ("{0}") % audioformat.str());
		//XXX
		track.filename = "vdrsync.ac3";
		track.description = tr("dolby");

		m_tracks.push_back( track );
        return length;
    }

	// --- recording_scanner ---------------------------------------------------

    recording_scanner::recording_scanner(job* owner, const cRecording* recording):
            m_itemToScan( recording ),
            m_totalSize( 0, 0 ),
            m_totalLength( 0, 0 ),
            m_scanResult( owner, recording )
    {
    }

    void recording_scanner::scan()
    {
#if VDRVERSNUM >= 10703
        recording_index index( m_scanResult.get_filename(), m_itemToScan->IsPesRecording() );
        const int frames_into_movie = 10 *  m_itemToScan->FramesPerSecond();
        const int frames_to_scan = 10 *  m_itemToScan->FramesPerSecond();
#else
        recording_index index( m_scanResult.get_filename() );
        static const int frames_into_movie = 10 * FRAMESPERSEC;
        static const int frames_to_scan = 10 * FRAMESPERSEC;
#endif

        cMarks marks;
        if (!marks.Load(m_itemToScan->FileName())) {
            logger::error(format( "couldn't read marks from {0}" ) % m_itemToScan->FileName());
            throw user_exception( tr("Couldn't read marks") );
        }

		int startIndex = frames_into_movie;
		if (marks.Count() > 0) {
			logger::debug( "marks available, skipping 10 seconds from first mark" );

			adaptor::list_iterator<cMark> mark( marks );
			startIndex += mark->position;
		} else
			logger::debug( "no marks found, skipping 10 seconds into movie" );

		if (startIndex + frames_to_scan >= index.get_last_index()) {
			logger::error( format("movie {0} is too short to scan") % m_itemToScan->FileName() );
			throw user_exception( tr("Recording is too short") );
		}

#if VDRVERSNUM >= 10703
		bool isPesRecording = m_itemToScan->IsPesRecording();
#else
		bool isPesRecording = true;
#endif
		pes_scanner pes( m_scanResult.get_filename(), isPesRecording );
		
        track_info_list& tracks =
				pes.scan(index.get_position(startIndex), index.get_position(startIndex + frames_to_scan));

		for_each( tracks.begin(), tracks.end(), bind( &recording_scanner::scan_track_description, this, _1 ) );

		m_scanResult.take_tracks( tracks );

        scan_total_sizes(index, marks);

		track_filter audioTracks( m_scanResult.get_tracks(), track_info::streamtype_audio, track_predicate::all );
		for_each( audioTracks.begin(), audioTracks.end(), bind( &recording_scanner::scan_audio_track_size, this, _1 ) );

		track_filter videoTracks( m_scanResult.get_tracks(), track_info::streamtype_video, track_predicate::all );
		for_each( videoTracks.begin(), videoTracks.end(), bind( &recording_scanner::scan_video_track_size, this, _1 ) );
    }

	bool video_track_finder(const tComponent& component)
	{
		return component.stream == etsi::sc_video;
	}

	bool ac3_track_finder(const tComponent& component)
	{
		return component.stream == etsi::sc_audio && component.type == etsi::cta_surround;
	}

	bool mp2_track_finder(const tComponent& component)
	{
		return component.stream == etsi::sc_audio && component.type != etsi::cta_surround;
	}

    void recording_scanner::scan_track_description(track_info& track)
    {
		adaptor::component_iterator component;

		switch (track.type)
		{
		case track_info::streamtype_video:
			component = find_if( adaptor::component_iterator( m_itemToScan->Info()->Components() ),
							adaptor::component_iterator(), video_track_finder );
			break;

		case track_info::streamtype_audio:

			switch (track.audio.type)
			{
			case track_info::audiotype_ac3:
				component =	find_if( adaptor::component_iterator( m_itemToScan->Info()->Components() ),
								adaptor::component_iterator(), ac3_track_finder );
				break;

			default:
				component =	find_nth( adaptor::component_iterator( m_itemToScan->Info()->Components() ),
								adaptor::component_iterator(), track.cid - 0xc0 + 1, mp2_track_finder );
				break;
			}

		default:
			break;
		}

		if ( component != adaptor::component_iterator() ) {
			if (component->language != 0)
				track.set_language( component->language );

			if (component->description != 0)
				track.description = component->description;
		}
    }

    void recording_scanner::scan_total_sizes(recording_index& index, cMarks& marks)
    {
    	m_totalSize = size_pair( index.get_bytepos(index.get_last_index(), true), 0 );
    	m_totalLength = length_pair( index.get_last_index() + 1, 0 );

		if (marks.Count() == 0) {
			m_totalSize.cut = m_totalSize.uncut;
			m_totalLength.cut = m_totalLength.uncut;
		} else {
			marks_scanner cutLength = for_each(adaptor::list_iterator<cMark>( marks ), adaptor::list_iterator<cMark>( ),
											marks_scanner( index ));

			m_totalSize.cut = cutLength.get_total_size();
			m_totalLength.cut = cutLength.get_total_length();
		}

		m_scanResult.set_total_size( m_totalSize );
		m_scanResult.set_total_length( m_totalLength );

        logger::debug( format("movie size: {0}") % m_totalSize.uncut );
        logger::debug( format("movie size (cut): {0}") % m_totalSize.cut );
        logger::debug( format("movie length: {0}") % *IndexToHMSF(m_totalLength.uncut) );
        logger::debug( format("movie length (cut): {0}") % *IndexToHMSF(m_totalLength.cut) );
    }

    void recording_scanner::scan_audio_track_size(track_info& track)
    {
#if VDRVERSNUM >= 10703
    	track.size.uncut = uint64_t( track.bitrate ) * 1000 * m_totalLength.uncut / m_itemToScan->FramesPerSecond() / 8;
    	track.size.cut = uint64_t( track.bitrate ) * 1000 * m_totalLength.cut / m_itemToScan->FramesPerSecond() / 8;
#else
    	track.size.uncut = uint64_t( track.bitrate ) * 1000 * m_totalLength.uncut / FRAMESPERSEC / 8;
    	track.size.cut = uint64_t( track.bitrate ) * 1000 * m_totalLength.cut / FRAMESPERSEC / 8;
#endif
    }

    void recording_scanner::scan_video_track_size(track_info& track)
    {
		track_filter audioTracks( m_scanResult.get_tracks(), track_info::streamtype_audio, track_predicate::all );
		size_pair audioSizes(
				sum( audioTracks.begin(), audioTracks.end(), uint64_t( 0 ),
						bind( &size_pair::uncut, bind( &track_info::size, _1 ) ) ),
				sum( audioTracks.begin(), audioTracks.end(), uint64_t( 0 ),
						bind( &size_pair::cut, bind( &track_info::size, _1 ) ) ) );

    	track.size.uncut = uint64_t( (double( m_totalSize.uncut ) * 0.995) - audioSizes.uncut );
    	track.size.cut = uint64_t( (double( m_totalSize.cut ) * 0.995) - audioSizes.cut );
    }

}
