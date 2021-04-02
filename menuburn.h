/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: menuburn.h,v 1.21 2006/09/16 18:33:36 lordjaxom Exp $
 */

#ifndef VDR_BURN_MENUBURN_H
#define VDR_BURN_MENUBURN_H

#include "menubase.h"
#include "jobs.h"
#include <vector>
#include <utility>
#include <string>
#include <vdr/menuitems.h>

class cRecording;

namespace vdr_burn
{

	namespace menu
	{

		// --- recording_info -------------------------------------------------

		class recording_info: public cOsdMenu
		{
		public:
			recording_info(const cRecording* recording);

		protected:
			virtual void Display();
			virtual eOSState ProcessKey(eKeys key);

		private:
			const cRecording* m_recording;
		};

		// --- track_editor ---------------------------------------------------

		class track_editor: public cOsdMenu
		{
		public:
			track_editor(pagebase& parent, recording& recording_);

		protected:
			void refresh();
			void add_track(track_info& track);
			bool can_move_down();
			void set_help_keys();
			void move_down();

			virtual void Store();
			virtual eOSState ProcessKey(eKeys key);

		private:
			pagebase& m_parent;
			recording& m_recording;
			track_info_list m_tracks;
			std::string m_title;
			int m_titleSource;
			std::vector<int> m_indices;
		};

		// --- title_chooser --------------------------------------------------

		class title_chooser: public cOsdMenu
		{

		public:
			title_chooser( const recording& recording_, std::string& title_, int* titleSource_);

		protected:
			void refresh();
			void add_title(const std::string& title);

			virtual eOSState ProcessKey(eKeys key);

		private:
			recording m_recording;
			std::string& m_title;
			int* m_titleSource;
		};


		// --- recordings -----------------------------------------------------

		class recordings: public pagebase
		{
		public:
			recordings();

		protected:
			void display();

			virtual void set_help_keys();
			virtual bool menu_closing();
			virtual eOSState ok_pressed();
			virtual eOSState red_pressed();
#if VDRVERSNUM >= 10729
			virtual eOSState zero_pressed();
#endif

		private:
			std::string m_basePath;
			std::string m_lastPath;
			int m_pathLevel;
			bool m_pathChanged;
			int m_recordingCount;

			std::string get_info_text( const job& job_ );
			std::string get_info_bar( const job& job_ );
		};

		// --- job_editor -----------------------------------------------------

		class job_editor: public pagebase
		{
		public:
			job_editor();

		protected:
			virtual void set_help_keys();
			virtual eOSState menu_update();
			virtual eOSState ok_pressed();
			virtual eOSState red_pressed();
			virtual eOSState green_pressed();
			virtual eOSState yellow_pressed();

		private:
			typedef std::vector< menu::recording_list_item* > recording_items;

			recording_items m_recordingItems;
			menu::size_text_item* m_infoTextItem;
			menu::size_bar_item* m_infoBarItem;
#ifdef ENABLE_DMH_ARCHIVE
			menu::text_item* m_archiveIdItem;
#endif
			menu::text_item* m_jobOptionsItem;

			void show_empty_list();
			bool is_recording_item( cOsdItem* item );
		};

		// --- status ---------------------------------------------------------

		class status: public pagebase
		{
		public:
			status();

		protected:
			void display();

			virtual void set_help_keys();
			virtual eOSState menu_update();
			virtual eOSState green_pressed();
			virtual eOSState yellow_pressed();
		};
	}

}

#endif // VDR_BURN_MENUBURN_H
