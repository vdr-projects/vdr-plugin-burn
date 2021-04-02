/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: burn.h,v 1.5 2006/09/16 18:33:36 lordjaxom Exp $
 */

#ifndef VDR_BURN_BURN_H
#define VDR_BURN_BURN_H

#include <string>
#include <vdr/plugin.h>
#include <vdr/i18n.h>

namespace vdr_burn
{

	class plugin: public cPlugin
	{
	public:
		plugin();
		virtual ~plugin();

        static std::string get_config_path() { return m_configPath; }
        static std::string get_resource_path() { return m_resourcePath; }
		static std::string get_character_encoding();

		virtual const char* Version() { return VERSION; }
		virtual const char* Description() { return tr(DESCRIPTION); }
		virtual const char* CommandLineHelp();
		virtual bool ProcessArgs(int argc, char *argv[]);
		virtual bool Start();
		virtual void Stop();
		virtual cString Active();
		virtual const char *MainMenuEntry();
		virtual cOsdObject *MainMenuAction();
		virtual cMenuSetupPage *SetupMenu();
		virtual bool SetupParse(const char *Name, const char *Value);

	private:
		static const char* VERSION;
		static const char* DESCRIPTION;
		static const char* MAINMENUENTRY;

        static std::string m_configPath;
        static std::string m_resourcePath;

		std::string m_mainMenuEntry;
	};

}

#endif // VDR_BURN_BURN_H
