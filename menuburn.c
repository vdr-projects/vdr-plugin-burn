/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: menuburn.c,v 1.43 2006/10/24 17:47:04 lordjaxom Exp $
 */

#include "common.h"
#include "manager.h"
#include "menuburn.h"
#include "menuitems.h"
#include "setup.h"
#include "skins.h"
#include "filter.h"
#include "etsi-const.h"
#include "proctools/logger.h"
#include "proctools/format.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <utility>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <vdr/interface.h>
#include <vdr/videodir.h>
#include <vdr/menu.h>
#include <vdr/status.h>

namespace vdr_burn
{

	using namespace std;
	using proctools::logger;
	using proctools::process;
	using boost::bind;

	namespace menu
	{

		/// --- recording_info -------------------------------------------------

		recording_info::recording_info(const cRecording* recording):
				cOsdMenu( get_recording_name( recording ).c_str() ),
				m_recording( recording )
		{
		}

		void recording_info::Display()
		{
			cOsdMenu::Display();
			DisplayMenu()->SetRecording( m_recording );
			cStatus::MsgOsdTextItem( m_recording->Info()->Description() );
		}

		eOSState recording_info::ProcessKey(eKeys key)
		{
			eOSState state( osUnknown );

			switch (NORMALKEY(key)) {
			case kUp:
			case kDown:
			case kLeft:
			case kRight:
				DisplayMenu()->Scroll(NORMALKEY(key) == kUp || NORMALKEY(key) == kLeft,
									  NORMALKEY(key) == kLeft || NORMALKEY(key) == kRight);
				cStatus::MsgOsdTextItem(NULL, NORMALKEY(key) == kUp);
				state = osContinue;
				break;

			case kOk:
				state = osBack;
				break;

			default:
				break;
			}

			if (state == osUnknown)
				state = cOsdMenu::ProcessKey( key );
			return state;
		}

		/// --- track_editor ---------------------------------------------------

		track_editor::track_editor(pagebase& parent, recording& recording_):
				cOsdMenu( tr("track editor"), 17 ),
				m_parent( parent ),
				m_recording( recording_ ),
				m_tracks( recording_.get_tracks() ),
				m_title( recording_.m_title ),
				m_titleSource( recording_.m_titleSource )
		{
			refresh();
		}

		#define TRACKS_STARTLINE 4

		void track_editor::refresh()
		{
			int current = Current();
			Clear();
			m_indices.clear();

			Add( new menu::text_item(m_recording.m_name) );
			Add( new menu::text_item( "" ) );
			Add( new menu::string_edit_item( tr("title in menu"), m_title, NULL ) );
			Add( new menu::text_item( "" ) );

			for_each(m_tracks.begin(), m_tracks.end(),
					 bind( &track_editor::add_track, this, _1 ));
			SetCurrent(Get(current == -1 ? 2 : current));
			set_help_keys();
			Display();
		}

		bool track_editor::can_move_down()
		{
			if (Current() < TRACKS_STARTLINE)
				return false;
			track_info_list::size_type current( m_indices[Current()-TRACKS_STARTLINE] );
			if (current == m_tracks.size() - 1 || m_tracks[current].get_is_video())
				return false;
			return true;
		}

		void track_editor::set_help_keys()
		{
			const char* red = NULL;
			const char* green = NULL;
			if (Current() == 2)
				red = tr("Choose");
			if (can_move_down())
				green = tr("Move down");
			//isyslog("burn: trackeditor:set_help_keys() %s-%s", red, green);
			SetHelp(red, green);
		}

		void track_editor::add_track(track_info& track)
		{
			int index = m_indices.size() == 0 ? 0 : m_indices.back() + 1;

			Add( new bool_edit_item( track.get_type_description(), track.used, tr("Track$Off"), tr("Track$On") ) );
			m_indices.push_back(index);

			switch (track.type) {
			case track_info::streamtype_audio:
				Add( new list_edit_item( str( boost::format("- %1$s") % tr("Language code") ), track.language, track_info::get_language_codes() ) );
				m_indices.push_back(index);
				break;

			case track_info::streamtype_subtitle:
				Add( new list_edit_item( str( boost::format("- %1$s") % tr("Language code") ), track.language, track_info::get_language_codes() ) );
				m_indices.push_back(index);
				break;

			case track_info::streamtype_video:
				Add( new aspect_item( track.video.aspect ) );
				m_indices.push_back(index);

			default: break;
			}
		}

		void track_editor::move_down()
		{
			if (!can_move_down())
				return;

			track_info_list::size_type current( m_indices[Current()-TRACKS_STARTLINE] );
			std::swap(m_tracks[current], m_tracks[current + 1]);
			SetCurrent(Get(Current() +2));
			refresh();
		}

		void track_editor::Store()
		{
			m_recording.get_tracks() = m_tracks;
			m_recording.m_title = m_title;
		}

		eOSState track_editor::ProcessKey(eKeys key)
		{
			int current = Current();

			bool hadSubMenu = HasSubMenu();
			eOSState state = cOsdMenu::ProcessKey(key);

			if (state == osUnknown)
			{
				 if (HasSubMenu())
					return osContinue;

				switch (key)
				{
					case kGreen:
						move_down();
						state = osContinue;
						break;
					case kRed:
						return AddSubMenu( new title_chooser( m_recording, m_title, &m_titleSource) );
						
					case kOk:
						Store();
						state = osBack;
						break;
					default:
						break;
				}
			}
			
			if (!HasSubMenu()) {
				if (hadSubMenu)
					refresh();
			
				if (key != kNone) {
					menu_item_base& item = dynamic_cast< menu_item_base& >( *Get( current ) );
					//isyslog("burn: track_editor::Processkey !kNone state=%d key=%d edit=%d (%d)", state, key, item.is_editing()?1:0, time(NULL));
					if ( !item.is_editing() )
						set_help_keys();
				}
			}
				
			return state;
		}

		/// --- title_chooser --------------------------------------------------

		title_chooser::title_chooser( const recording& recording_, std::string& title_, int* titleSource_):
				cOsdMenu(tr("Choose title")),
				m_recording( recording_ ),
				m_title( title_ ),
				m_titleSource( titleSource_ )
		{
			refresh();
		}

		void title_chooser::refresh()
		{
			std::string tmp_title;
			
			Clear();
			Add( new menu::text_item(tr("DVD menu title for ") + m_recording.m_name) );
			Add( new menu::text_item( "" ) );

			Add( new menu::text_item( m_recording.m_eventTitle, true ) );

			if (!m_recording.m_eventShortText.empty()) {
				tmp_title = m_recording.m_eventTitle + " - " + m_recording.m_eventShortText;
				Add( new menu::text_item( tmp_title, true ) );
				
				Add( new menu::text_item( m_recording.m_eventShortText, true ) );
			}
			Add( new menu::text_item( m_recording.m_name, true ) );
			
			//TODO: setCurrent from TitleSource ?
			
			Display();
		}

		void title_chooser::add_title(const std::string& title)
		{
			Add(new text_item(title, true));
		}

		eOSState title_chooser::ProcessKey(eKeys key)
		{
			eOSState state = cOsdMenu::ProcessKey(key);
			if (state == osUnknown) {
				switch (key) {
					case kOk:
						m_title = string(Get(Current())->Text());
						//isyslog("burn: titlechooser result: %s %s", m_title.c_str(), Get(Current())->Text());
						state = osBack;
						break;
					default:
						break;
				}
			}
			
			return state;
		}

		/// --- recordings -----------------------------------------------------

		recordings::recordings():
				pagebase( "", 9, 6, 6),
				m_pathLevel(0),
				m_pathChanged(false),
				m_recordingCount(0)
		{
			display();
		}

		std::string recordings::get_info_text( const job& job_ )
		{
			const recording_list& recordings( job_.get_recordings() );
			if ( recordings.size() > 0 )
				return str( boost::format( tr("$Recordings: %1$d, total size: %2$.1f MB") )
							% recordings.size() % ( double( job_.get_tracks_size() ) / MEGABYTE(1) ) );
			else
				return tr("No recordings marked for burning");
		}

		std::string recordings::get_info_bar( const job& job_ )
		{
			return progress_bar( job_.get_tracks_size() / MEGABYTE(1), job_.get_disk_size_mb(), 100 );
		}

		void recordings::display()
		{
#if defined(APIVERSNUM) && APIVERSNUM >= 20301
			LOCK_RECORDINGS_WRITE;  // for sort is a write lock required
			GetRecordingsSortMode(m_basePath.c_str());
			Recordings->Sort();
#else
			cThreadLock RecordingsLock(&Recordings);
			GetRecordingsSortMode(m_basePath.c_str());
			Recordings.Sort();
#endif
			int current = Current();
			Clear();
			m_recordingCount = 0;

			if (m_pathChanged) {
				current = -1;
				m_pathChanged = false;
			}

			SetTitle( str( boost::format( "%1$s - %2$s" ) % tr("Write DVDs")
						   % ( m_basePath.empty() ? tr("Recordings") : m_basePath ) ).c_str() );

			job& pending = manager::get_pending();
			Add( new menu::text_item( get_info_text( pending ) ) );
			Add( new menu::text_item( get_info_bar( pending ) ) );
			Add( new menu::text_item() );

			recording_item *lastItem = 0;
			string lastText;
#if defined(APIVERSNUM) && APIVERSNUM >= 20301
			for (cRecording *rec = Recordings->First(); rec != NULL; rec = Recordings->Next(rec)) {
#else
			for (cRecording *rec = Recordings.First(); rec != NULL; rec = Recordings.Next(rec)) {
#endif
				string recName( rec->Name() );

				// find H.264 videos
				bool is_H264 = false;
				bool is_MPEG2 = false;
				if (rec->Info() && rec->Info()->Components()) {
					const cComponents *Components = rec->Info()->Components();
					for (int i = 0; i < Components->NumComponents(); i++) {
						const tComponent *p = Components->Component(i);
						if (p->stream == etsi::sc_video_H264_AVC) {
							is_H264 = true;
							break;
						} 
						if (p->stream == etsi::sc_video_MPEG2) {
							is_MPEG2 = true;
						}
					}
				}

				if ( manager::is_recording_queued( rec->FileName() ) )
					continue;

				if (!m_basePath.empty() &&
						(recName.find(m_basePath) != 0 || recName[m_basePath.length()] != '~'))
					continue;

				menu::recording_item* item = new menu::recording_item(rec, m_pathLevel);
				string itemText( item->Text() );
				if (!item->is_directory() && (is_H264 | !is_MPEG2))
					item->SetSelectable(false);
				if (itemText.length() != 0 && (lastItem == 0 || itemText != lastText)) {
					// select directory we are coming from as current item
					if (recName.find(m_lastPath) == 0 && rec->Name()[m_lastPath.length()] == '~') {
						current = Count();
						m_lastPath.clear();
					}

					Add(item);
					lastText = itemText;
					lastItem = item;
					++m_recordingCount;
				} else
					delete item;

				if (lastItem != 0 && lastItem->is_directory())
					lastItem->add_entry(rec->IsNew());
			}

			set_current(current);
			set_help_keys();
			pagebase::display();
		}

		void recordings::set_help_keys()
		{
			if ( m_recordingCount == 0 )
				return;

			cOsdItem* current = Get(Current());

			string red;
			if (current != 0) {
				if (!dynamic_cast<menu::recording_item*>( current )
						->is_directory())
					red = tr("Choose");
			}
			set_help(red);
		}

		bool recordings::menu_closing()
		{
			if (m_pathLevel == 0)
				return true;

			string newPath;
			if (m_pathLevel > 1) {
				string::size_type pos;
				if ((pos = m_basePath.rfind('~')) != string::npos)
					newPath = m_basePath.substr(0, pos);
			}
			m_lastPath = m_basePath;
			m_basePath = newPath;
			m_pathChanged = true;
			--m_pathLevel;
			display();
			return false;
		}

		eOSState recordings::ok_pressed()
		{
			recording_item* item =
					dynamic_cast<recording_item*>( Get(Current()) );

			if (item == 0)
				return osUnknown;

			if (!item->is_directory())
				return AddSubMenu( new recording_info( item->get_recording() ) );

			++m_pathLevel;
			ostringstream newPath;
			if (!m_basePath.empty())
				newPath << m_basePath << "~";
			newPath << item->get_name();
			m_basePath = newPath.str();
			m_pathChanged = true;
			display();
			return osContinue;
		}

		eOSState recordings::red_pressed()
		{
			try {
				recording_item* item = dynamic_cast<recording_item*>( Get(Current()) );

				if (item == 0 || item->is_directory())
					return osUnknown;

				Skins.Message(mtStatus, tr("Analyzing recording..."));
				manager::get_pending().append_recording(item->get_recording());
				display();
				if ( m_recordingCount == 0 ) {
					if ( m_pathLevel == 0 )
						starter::set_display( display::job );
					if ( menu_closing() )
						return osBack;
				}
			} catch (const user_exception& ex) {
				Skins.Message( mtError, ex.what() );
			}
			return osContinue;
		}

		eOSState recordings::zero_pressed()
		{
			IncRecordingsSortMode(m_basePath.c_str());
			display();
			return osContinue;
		}
		
		/// --- job_editor -----------------------------------------------------

		job_editor::job_editor():
				pagebase( str( boost::format( "%1$s - %2$s" ) % tr("Write DVDs") % tr("Job") ), 16 )
#ifdef ENABLE_DMH_ARCHIVE
				, m_archiveIdItem( 0 )
#endif
		{
			job& pending( manager::get_pending() );
			Add( m_infoTextItem = new menu::size_text_item( pending ) );
			Add( m_infoBarItem = new menu::size_bar_item( pending ) ); // TODO Width of menu
			Add( new menu::text_item( "" ) );

			recording_list& recordings = pending.get_recordings();
			if (recordings.size() == 0) {
				show_empty_list();
				set_help_keys();
				pagebase::display();
				return;
			}

			m_recordingItems.reserve( recordings.size() );
			for ( recording_list::iterator rec = recordings.begin(); rec != recordings.end(); ++rec ) {
				menu::recording_list_item* item;
				Add( item = new menu::recording_list_item( rec ) );
				m_recordingItems.push_back( item );
			}
			SetCurrent(m_recordingItems.front());

			Add( new menu::text_item( "" ) );
			Add( new menu::string_edit_item( tr("DVD title"), pending.get_title(), NULL ) );
#ifdef ENABLE_DMH_ARCHIVE
			Add( m_archiveIdItem = new menu::text_item( str( boost::format( "%1$s:\t%2$s" ) % tr("Archive-ID") % pending.get_archive_id() ), true ) );
#endif
			Add( new menu::text_item( "" ) );
			Add( m_jobOptionsItem = new menu::text_item( str( boost::format( "%1$s..." ) % tr("Job options") ), true ) );

			menu_update();
		}

		void job_editor::show_empty_list()
		{
			while ( Count() > 3 )
				Del( 3 );
			m_recordingItems.clear();
#ifdef ENABLE_DMH_ARCHIVE
			m_archiveIdItem = 0;
#endif
			m_jobOptionsItem = 0;

			text_item* item;
			Add( item = new text_item( tr("No recordings marked"), true ) );
			SetCurrent( item );
		}

		bool job_editor::is_recording_item( cOsdItem* item )
		{
			recording_items::const_iterator it = find( m_recordingItems.begin(), m_recordingItems.end(), item );
			return it != m_recordingItems.end();
		}

		void job_editor::set_help_keys()
		{
			string red, green, yellow;
			if ( m_recordingItems.size() > 0 ) {
				red = tr("Button$Start");

				const cOsdItem* current = Get( Current() );
				if ( find( m_recordingItems.begin(), m_recordingItems.end(), Get( Current() ) ) != m_recordingItems.end() ) {
					yellow = tr("Remove");
					if ( current != m_recordingItems.back() )
						green = tr("Move down");
				}
			}
			set_help(red, green, yellow);
		}

		eOSState job_editor::menu_update()
		{
			m_infoTextItem->update();
			m_infoBarItem->update();

#ifdef ENABLE_DMH_ARCHIVE
			if ( m_archiveIdItem != 0 )
				m_archiveIdItem->SetSelectable( manager::get_pending().get_options().DmhArchiveMode );
#endif
			set_help_keys();
			display();
			return osContinue;
		}

		eOSState job_editor::ok_pressed()
		{
			cOsdItem* current = Get( Current() );

			job& pending = manager::get_pending();
			if ( current == m_jobOptionsItem )
				return AddSubMenu( new job_options_editor( pending ) );

			if ( !is_recording_item( current ) )
				return osContinue;

			recording_list_item& item = static_cast< recording_list_item& >( *current );
#ifdef ENABLE_DMH_ARCHIVE
			if ( pending.get_disk_type() >= disktype_archive )
				return osContinue;
#endif

			return AddSubMenu( new track_editor( *this, *item.get_recording() ) );
		}

		eOSState job_editor::red_pressed()
		{
			job& pending = manager::get_pending();
			recording_list& recordings = pending.get_recordings();

			if (recordings.size() == 0)
				return osContinue;

			recording_list::const_iterator rec;
			for (rec = recordings.begin(); rec != recordings.end(); ++rec) {
				const_track_filter videoTracks( rec->get_tracks(), track_info::streamtype_video, track_predicate::used );
				if (videoTracks.begin() == videoTracks.end()) {
					Skins.Message(mtError, tr("No MPEG2 video track selected!"));
					// XXX Should be moved to job control to determine whether
					// a video track is necessary (audio-dvd or so)
					return osContinue;
				}

				const_track_filter audioTracks( rec->get_tracks(), track_info::streamtype_audio, track_predicate::used );
				if (audioTracks.begin() == audioTracks.end()) {
					Skins.Message(mtError, tr("No audio track selected!"));
					return osContinue;
				}
			}

			if ( pending.get_tracks_size() * 2.1 / MEGABYTE(1) >= FreeDiskSpaceMB( BurnParameters.DataPath.c_str() ) ) {
				Skins.Message(mtError, tr("Not enough free disk space!"));
				return osContinue;
			}

			manager::start_pending();
			starter::set_display(display::status);
			return osBack;
		}

		eOSState job_editor::green_pressed()
		{
			cOsdItem* current = Get( Current() );
			if ( !is_recording_item( current ) || current == m_recordingItems.back() )
				return osContinue;

			recording_list_item& item = static_cast< recording_list_item& >( *current );
			recording_list::iterator thisRec( item.get_recording() );
			recording_list::iterator nextRec( thisRec );
			++nextRec;

			iter_swap(thisRec, nextRec);
			Move( current, Next( current ) );
			set_help_keys();
			pagebase::display();
			return osContinue;
		}

		eOSState job_editor::yellow_pressed()
		{
			cOsdItem* current = Get( Current() );
			if ( !is_recording_item( current ) || !Interface->Confirm( tr("Remove recording from list?") ) )
				return osContinue;

			job& pending = manager::get_pending();
			recording_list_item& item = static_cast< recording_list_item& >( *current );

			pending.erase_recording( item.get_recording() );

			m_recordingItems.erase( find( m_recordingItems.begin(), m_recordingItems.end(), current ) );
			Del( Current() );
			if ( m_recordingItems.size() == 0 )
				show_empty_list();
			set_help_keys();
			pagebase::display();
			return osContinue;
		}

		/// --- status ---------------------------------------------------------

		status::status():
				pagebase( str( boost::format( "%1$s - %2$s" ) % tr("Write DVDs") % tr("Status") ), 16 )
		{
			display();
		}

		void status::display()
		{
			int current = Current();
			Clear();

			manager::lock man_lock;

			int number = 1;
			if (manager::get_active() != 0) {
				Add(new menu::job_item(manager::get_active(), number++));

				int percent;
				if (manager::get_active()->get_is_burning(percent))
					Add( new menu::text_item( str( boost::format( tr("$Job active (Writing: %1$d%%)") ) % percent ) ) );
				else
					Add(new menu::text_item(tr("Job active (Converting)")));

				Add(new menu::text_item(
						progress_bar(manager::get_active()->get_progress(),
									 100, 100)));
				//XXX Width of menu
			} else
				Add(new menu::text_item(tr("No active or waiting jobs present"),
										  true));

			for (job_queue::const_iterator job = manager::get_queued().begin();
				 job != manager::get_queued().end(); ++job)
				Add(new menu::job_item(*job, number++));

			if (manager::get_canceled().size() > 0) {
				Add(new menu::text_item());
				Add(new menu::text_item(tr("Canceled jobs")));
				Add(new menu::text_item());

				number = 1;
				job_queue::const_iterator job;
				for (job = manager::get_canceled().begin();
					 job != manager::get_canceled().end(); ++job)
					Add(new menu::job_item(*job, number++, true));
			}

			if (manager::get_erroneous().size() > 0) {
				Add(new menu::text_item());
				Add(new menu::text_item(tr("Erroneous jobs")));
				Add(new menu::text_item());

				number = 1;
				job_queue::const_iterator job;
				for (job = manager::get_erroneous().begin();
					 job != manager::get_erroneous().end(); ++job) {
					Add(new menu::job_item(*job, number++, true));
				}
			}
			set_current(current);
			set_help_keys();
			pagebase::display();
		}

		void status::set_help_keys()
		{
			job_item* item =
					dynamic_cast<job_item*>( Get(Current()) );

			manager::lock man_lock;

			string green, yellow;
			if (item != 0 && item->get_job()) {
				if (item->get_job()->get_is_canceled())
					green = tr("Restart");
				else if (item->get_job()->get_return_status() == process::error)
					green = tr("Retry");

				if (item->get_job() == manager::get_active())
					yellow = tr("Cancel");
				else
					yellow = tr("Remove");
			}
			set_help(string(), green, yellow);
		}

		eOSState status::menu_update()
		{
			display();
			return osContinue;
		}

		eOSState status::green_pressed()
		{
			job_item* item =
					dynamic_cast<job_item*>( Get(Current()) );

			if (item == 0)
				return osUnknown;

			manager::lock man_lock;

			if (item->get_job() == manager::get_active())
				return osUnknown;

			if (manager::get_pending().get_recordings().size() > 0 &&
					!Interface->Confirm(tr("Replace current job?")))
				return osContinue;

			manager::replace_pending(item->get_job());
			starter::set_display(display::job);
			return osBack;
		}

		eOSState status::yellow_pressed()
		{
			job_item* item =
					dynamic_cast<job_item*>( Get(Current()) );

			if (item == 0)
				return osUnknown;

			manager::lock man_lock;

			if (item->get_job() == manager::get_active()) {
				if (Interface->Confirm(tr("Job still active - really cancel?")))
					item->get_job()->stop();
			}
			else if (Interface->Confirm(tr("Remove job from list?"))) {
				manager::erase_job(item->get_job());
				delete item->get_job();
			}
			display();
			return osContinue;
		}
	}

}
