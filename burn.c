/*
 * burn.c: A plugin for the Video Disk Recorder
 *
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 * $Id: burn.c,v 1.26 2006/09/16 18:33:36 lordjaxom Exp $
 */

#include "burn.h"
#include "gdwrapper.h"
#include "logger-vdr.h"
#include "manager.h"
#include "menuburn.h"
#include "setup.h"
#include "skins.h"
#include "proctools/format.h"
#include <sstream>
#include <string>
#include <vdr/config.h>
#include <vdr/plugin.h>
#include <vdr/videodir.h>

// plugin needs VDR 2.0.0 or higher
#if VDRVERSNUM < 20000
#error The BURN plugin requires VDR 2.0.0 or higher
#endif

#ifdef TTXT_SUBTITLES
#warning Compiling WITH teletext subtitle support
#else
#warning Compiling WITHOUT teletext subtitle support
#endif

namespace vdr_burn
{

	using namespace std;
	using proctools::format;

	const char* plugin::VERSION        = "0.3.0";
	const char* plugin::DESCRIPTION    = trNOOP("Versatile convert-and-burn plugin");
	const char* plugin::MAINMENUENTRY  = tr("Create DVDs");

    string plugin::m_configPath;
    string plugin::m_resourcePath;

	plugin::plugin():
			m_mainMenuEntry( tr(MAINMENUENTRY) )
	{
	}

	plugin::~plugin()
	{
	}

	string plugin::get_character_encoding()
	{
		if (cCharSetConv::SystemCharacterTable())
			return cCharSetConv::SystemCharacterTable();
		else
			return "utf-8";
	}

	const char *plugin::CommandLineHelp()
	{
		static string help_str;

		if (help_str.empty())
			help_str = format(
				 "  -t DIR,   --tempdir=DIR   use DIR for temporary files\n"
				 "                            MUST be on UNIX filesystem\n"
				 "                            (default: {0})\n"
				 "  -d DIR,   --datadir=DIR   use DIR for temporary datafiles\n"
				 "                            (default: {1})\n"
				 "  -D DEV    --dvd=DEV       use DEV as DVD Writer\n"
				 "                            (default: {2})\n"
				 "  -i DIR    --iso=DIR       use DIR to store ISO images\n"
				 "                            (default: {3})\n"
				 ) % BurnParameters.TempPath % BurnParameters.DataPath
				   % BurnParameters.DvdDevice % BurnParameters.IsoPath;

		return help_str.c_str();
	}

	bool plugin::ProcessArgs(int argc, char *argv[])
	{
		return BurnParameters.ProcessArgs(argc, argv);
	}

	bool plugin::Start()
	{
        m_configPath = ConfigDirectory(PLUGIN_NAME);
        m_resourcePath = ResourceDirectory(PLUGIN_NAME);

		manager::start();
		logger_vdr::start();
		gdwrapper::setup::set_font_path( get_resource_path() + "/fonts" );
		if ( !skin_list::get().load( get_resource_path() ) )
			return false;

		logger_vdr::startup_finished();
		return true;
	}

	void plugin::Stop()
	{
		manager::stop();
		logger_vdr::stop();
	}

	cString plugin::Active()
	{
		if (manager::get_is_busy())
			return tr("Burn job active");
		return 0;
	}

	const char *plugin::MainMenuEntry()
	{
		ostringstream str;
		str << tr(MAINMENUENTRY);

		if (global_setup().MainMenuStatus) {
			string status( manager::get_status_message() );
			if (!status.empty())
				str << " (" << status << ")";
		}

		m_mainMenuEntry = str.str();
		return m_mainMenuEntry.c_str();
	}

	cOsdObject *plugin::MainMenuAction()
	{
		return new menu::starter;
	}

	cMenuSetupPage *plugin::SetupMenu()
	{
		return new plugin_setup_editor;
	}

	bool plugin::SetupParse(const char *Name, const char *Value)
	{
		return global_setup().set( Name, Value ) || job_defaults().set( Name, Value );
	}

}

VDRPLUGINCREATOR(vdr_burn::plugin); // Don't touch this!
