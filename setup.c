/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: setup.c,v 1.23 2006/09/16 18:33:37 lordjaxom Exp $
 */

#include "setup.h"
#include "manager.h"
#include "menuburn.h"
#include "menuitems.h"
#include "common.h"
#if APIVERSNUM < 10507
#include "i18n.h"
#endif
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <vdr/menuitems.h>
#include <vdr/videodir.h>
#include <getopt.h>

namespace vdr_burn
{

	using namespace std;

// --- cBurnParameters --------------------------------------------------------

cBurnParameters BurnParameters;

cBurnParameters::cBurnParameters():
		DataPath(TMPDIR),
		TempPath(TMPDIR),
		DvdDevice(DVDDEV),
		IsoPath(ISODIR),
		fixedStoreMode( false ),
		KeepTempFiles (false)
{
}

bool cBurnParameters::ProcessArgs(int argc, char *argv[])
{
	static struct option opts[] = {
			{ "tempdir", required_argument, NULL, 't' },
			{ "datadir", required_argument, NULL, 'd' },
			{ "dvd",     required_argument, NULL, 'D' },
			{ "iso",     required_argument, NULL, 'i' },
			{ "keep",    no_argument,       NULL, 'k' },
			{ NULL }
	};

	int c, optind = 0;
	while ((c = getopt_long(argc, argv, "t:d:D:i:k", opts, &optind)) != -1) {
		switch (c) {
			case 't': TempPath = optarg; break;
			case 'd': DataPath = optarg; break;
			case 'D': DvdDevice = optarg; break;
			case 'i': IsoPath = optarg; break;
			case 'k': KeepTempFiles = true; break;
			default:  return false;
		}
	}

	struct stat sbuf;
	if (stat(DvdDevice.c_str(), &sbuf) != 0) {
		isyslog("burn: couldn't stat %s, assuming iso-creation only", DvdDevice.c_str());
		DvdDevice.clear();
	}

	if (stat(IsoPath.c_str(), &sbuf) != 0) {
		isyslog("burn: couldn't stat %s, assuming writing to disc only", IsoPath.c_str());
		IsoPath.clear();
	}

	if (DvdDevice.empty() && IsoPath.empty()) {
		esyslog("ERROR[burn]: no targets left, check --dvd and --iso");
		fprintf(stderr, "ERROR[burn]: no targets left, check --dvd and --iso\n");
		return false;
	}

	if ( DvdDevice.empty() || IsoPath.empty() )
		fixedStoreMode = true;

	return true;
}

	//!--- plugin_setup -----------------------------------------------------------

	plugin_setup::plugin_setup():
			PROCTOOLS_INIT_PROPERTY( RemovePath,          false ),
			PROCTOOLS_INIT_PROPERTY( CustomDiskSize,      200 ),
			PROCTOOLS_INIT_PROPERTY( BurnSpeed,           0 ),
			PROCTOOLS_INIT_PROPERTY( RequantType,         requanttype_metakine ),
			PROCTOOLS_INIT_PROPERTY( PreserveLogFiles,    false ),
			PROCTOOLS_INIT_PROPERTY( DefaultLanguage,     0 ),
			PROCTOOLS_INIT_PROPERTY( MainMenuStatus,      true ),
			PROCTOOLS_INIT_PROPERTY( OfferDiskType,       true ),
			PROCTOOLS_INIT_PROPERTY( OfferChapters,       true ),
			PROCTOOLS_INIT_PROPERTY( OfferDiskSize,       true ),
			PROCTOOLS_INIT_PROPERTY( OfferStoreMode,      true ),
#ifdef ENABLE_DMH_ARCHIVE
			PROCTOOLS_INIT_PROPERTY( OfferDmhArchiveMode, true ),
#endif
			PROCTOOLS_INIT_PROPERTY( OfferCutOnDemux,     true ),
			PROCTOOLS_INIT_PROPERTY( OfferSkipTitlemenu,  true ),
			PROCTOOLS_INIT_PROPERTY( OfferSkipMainmenu,   true )
	{
	}

	plugin_setup& global_setup()
	{
		static plugin_setup instance;
		return instance;
	}

	//!--- job_options ------------------------------------------------------------

	job_options::job_options():
			PROCTOOLS_INIT_PROPERTY( DiskType,            disktype_dvd_menu ),
			PROCTOOLS_INIT_PROPERTY( SkinIndex,           0 ),
			PROCTOOLS_INIT_PROPERTY( SkinAspectIndex,     0 ),
			PROCTOOLS_INIT_PROPERTY( TitleType,           titletype_epgtitle ),
			PROCTOOLS_INIT_PROPERTY( ChaptersMode,        chaptersmode_10 ),
			PROCTOOLS_INIT_PROPERTY( StoreMode,           storemode_burn ),
#ifdef ENABLE_DMH_ARCHIVE
			PROCTOOLS_INIT_PROPERTY( DmhArchiveMode,      true ),
#endif
			PROCTOOLS_INIT_PROPERTY( DiskSize,            disksize_singlelayer ),
			PROCTOOLS_INIT_PROPERTY( CutOnDemux,          false ),
			PROCTOOLS_INIT_PROPERTY( SkipTitlemenu,       true ),
			PROCTOOLS_INIT_PROPERTY( SkipMainmenu,        true ),
			PROCTOOLS_INIT_PROPERTY( UseSubtitleTracks,   false )
	{
	}

	bool job_options::set( const std::string& name_, const std::string& value_ )
	{
		if ( name_ != StoreMode.name() || !BurnParameters.fixedStoreMode )
			return proctools::property_bag::set( name_, value_ );

		if ( BurnParameters.DvdDevice.empty() )
			StoreMode = storemode_create;
		else if ( BurnParameters.IsoPath.empty() )
			StoreMode = storemode_burn;
		return true;
	}

	job_options& job_defaults()
	{
		static job_options instance;
		return instance;
	}

	//!--- job_options_base -------------------------------------------------------

	job_options_base::job_options_base( job_options& options_, bool showAll_ ):
			m_storeModeItem( 0 ),
			m_diskTypeItem( 0 ),
			m_skinItem( 0 ),
			m_skinAspectItem( 0 ),
			m_titleTypeItem( 0 ),
			m_chaptersItem( 0 ),
			m_diskSizeItem( 0 ),
			m_cutItem( 0 ),
#ifdef ENABLE_DMH_ARCHIVE
			m_archiveItem( 0 ),
#endif
			m_skipTitleItem( 0 ),
			m_skipMainItem( 0 ),
			m_useSubtitleTracks( 0 ),
			m_options( options_ ),
			m_showAll( showAll_ )
	{
	}

	void job_options_base::add_job_options()
	{
		if ( m_showAll || global_setup().OfferStoreMode ) {
			Add( m_storeModeItem = new menu::list_edit_item( tr("Target"), m_options.StoreMode, storemode_strings ) );
			m_storeModeItem->SetSelectable( !BurnParameters.fixedStoreMode );
		}

		if ( m_showAll || global_setup().OfferDiskType )
			Add( m_diskTypeItem = new menu::list_edit_item( tr("Disk type"), m_options.DiskType, disktype_strings ) );

#ifdef ENABLE_DMH_ARCHIVE
		if ( m_showAll || global_setup().OfferDmhArchiveMode )
			Add( m_archiveItem = new menu::bool_edit_item( tr("DMH-archive"), m_options.DmhArchiveMode ) );
#endif

		if ( m_showAll || skin_list::get().size() > 0 )
			Add( m_skinItem = new menu::list_edit_item( tr("Skin"), m_options.SkinIndex, skin_list::get_names(), false ) );

		if ( m_showAll || skin_list::get().size() > 0 )
			Add( m_skinAspectItem = new menu::list_edit_item( tr("Skin Aspect Ratio"), m_options.SkinAspectIndex, skinaspect_strings ) );

		if ( m_showAll)
			Add( m_titleTypeItem = new menu::list_edit_item( tr("Title source"), m_options.TitleType, titletype_strings, true) );

		if ( m_showAll || global_setup().OfferChapters )
			Add( m_chaptersItem = new menu::list_edit_item( tr("Chapters"), m_options.ChaptersMode, chaptersmode_strings ) );

		if ( m_showAll || global_setup().OfferDiskSize )
			Add( m_diskSizeItem = new menu::list_edit_item( tr("Disk size"), m_options.DiskSize, disksize_strings ) );

		if ( m_showAll || global_setup().OfferCutOnDemux )
			Add( m_cutItem = new menu::bool_edit_item( tr("Cut"), m_options.CutOnDemux));

		if ( m_showAll || global_setup().OfferSkipTitlemenu )
			Add( m_skipTitleItem = new menu::bool_edit_item( tr("Skip empty titlemenu"), m_options.SkipTitlemenu ) );

		if ( m_showAll || global_setup().OfferSkipMainmenu )
			Add( m_skipMainItem = new menu::bool_edit_item( tr("Skip short mainmenu"), m_options.SkipMainmenu ) );

		if ( m_showAll )
			Add( m_useSubtitleTracks = new menu::bool_edit_item( tr("Use subtitle tracks"), m_options.UseSubtitleTracks ) );
	}

	//!--- plugin_setup_editor ----------------------------------------------------

	plugin_setup_editor::plugin_setup_editor():
			job_options_base( m_defaults, true )
	{
		m_setup = global_setup();
		m_defaults = job_defaults();

		Add( new menu::text_item( tr("--- Common settings --------------------------------------------------") ) );
		Add( new menu::bool_edit_item( tr("Remove path component"), m_setup.RemovePath ) );
		Add( new menu::list_edit_item( tr("Requant using"), m_setup.RequantType, requanttype_strings ) );
		Add( new menu::number_edit_item( tr("Burn speed"), m_setup.BurnSpeed, 0, 32, tr("unlimited") ) );
		Add( new menu::bool_edit_item( tr("Preserve logfiles"), m_setup.PreserveLogFiles ) );
		Add( new menu::list_edit_item( tr("Spare language code"), m_setup.DefaultLanguage, track_info::get_language_codes(), false ) );
		Add( new menu::bool_edit_item( tr("Status in main menu"), m_setup.MainMenuStatus ) );
		Add( new menu::number_edit_item( tr("Custom disk size (MB)"), m_setup.CustomDiskSize, 0, 9999 ) );

		Add( new menu::text_item( tr("--- Job menu settings ------------------------------------------------") ) );
		Add( new menu::bool_edit_item( tr("Offer disk type"), m_setup.OfferDiskType ) );
		Add( new menu::bool_edit_item( tr("Offer chapters"), m_setup.OfferChapters ) );
		Add( new menu::bool_edit_item( tr("Offer target"), m_setup.OfferStoreMode ) );
		Add( new menu::bool_edit_item( tr("Offer disk size"), m_setup.OfferDiskSize ) );
		Add( new menu::bool_edit_item( tr("Offer cutting"), m_setup.OfferCutOnDemux ) );
#ifdef ENABLE_DMH_ARCHIVE
		Add( new menu::bool_edit_item( tr("Offer DMH-archive"), m_setup.OfferDmhArchiveMode ) );
#endif
		Add( new menu::bool_edit_item( tr("Offer skip titlemenu"), m_setup.OfferSkipTitlemenu ) );
		Add( new menu::bool_edit_item( tr("Offer skip mainmenu"), m_setup.OfferSkipMainmenu ) );

		Add( new menu::text_item( tr("--- Job defaults -----------------------------------------------------") ) );
		add_job_options();
	}

	void plugin_setup_editor::store_setup()
	{
		for_each( m_setup.begin(), m_setup.end(), bind( &plugin_setup_editor::store_value, this, _1 ) );
		for_each( m_defaults.begin(), m_defaults.end(), bind( &plugin_setup_editor::store_value, this, _1 ) );

		global_setup() = m_setup;
		job_defaults() = m_defaults;

		job& pending = manager::get_pending();
		if (pending.get_recordings().size() == 0)
			pending.clear();
	}

	void plugin_setup_editor::store_value( const std::pair< std::string, proctools::property_base* >& value )
	{
		SetupStore( value.first.c_str(), value.second->as_string().c_str() );
	}

	//!--- job_options_editor -----------------------------------------------------

	job_options_editor::job_options_editor( job& job_ ):
			job_options_base( m_options, false ),
			m_job( job_ )
	{
		m_options = job_.get_options();

		SetTitle( str( boost::format( "%1$s - %2$s" ) % tr("Write DVDs") % tr("Job options") ).c_str() );

		Add( m_infoTextItem = new menu::size_text_item( m_job ) );
		Add( m_infoBarItem = new menu::size_bar_item( m_job ) );
		Add( new menu::text_item() );
		add_job_options();
		check_item_states();
	}

	void job_options_editor::check_item_states()
	{
#ifdef ENABLE_DMH_ARCHIVE
		if ( m_options.DiskType == disktype_archive ) {
			m_options.DmhArchiveMode = true;
			m_options.CutOnDemux = false;
		} else {
			if ( !global_setup().OfferDmhArchiveMode )
				m_options.DmhArchiveMode = job_defaults().DmhArchiveMode;
#endif
			if ( !global_setup().OfferCutOnDemux )
				m_options.CutOnDemux = job_defaults().CutOnDemux;
#ifdef ENABLE_DMH_ARCHIVE
		}
#endif

//		m_archiveItem->set_value( m_options.DiskType == disktype_archive ? true : bool( m_options.DmhArchiveMode ) );
//		m_cutItem->set_value( m_options.DiskType == disktype_archive ? false : bool( m_options.CutOnDemux ) );

		m_infoTextItem->update( m_options.CutOnDemux );
		m_infoBarItem->update( m_options.CutOnDemux );

#ifdef ENABLE_DMH_ARCHIVE
		if ( m_archiveItem != 0 ) {
			m_archiveItem->set_value( m_options.DmhArchiveMode );
			m_archiveItem->SetSelectable( m_options.DiskType < disktype_archive );
		}
#endif
		if ( m_skinItem != 0 ) {
			m_skinItem->SetSelectable( m_options.DiskType == disktype_dvd_menu );
			m_skinAspectItem->SetSelectable( m_options.DiskType == disktype_dvd_menu );
		}
#ifdef ENABLE_DMH_ARCHIVE
		if ( m_chaptersItem != 0 )
			m_chaptersItem->SetSelectable( m_options.DiskType < disktype_archive );
#endif
		if ( m_cutItem != 0 ) {
			m_cutItem->set_value( m_options.CutOnDemux );
#ifdef ENABLE_DMH_ARCHIVE
			m_cutItem->SetSelectable( m_options.DiskType < disktype_archive );
#endif
		}
		if ( m_skipTitleItem != 0 )
			m_skipTitleItem->SetSelectable( m_options.DiskType == disktype_dvd_menu );
		if ( m_skipMainItem != 0 )
			m_skipMainItem->SetSelectable( m_options.DiskType == disktype_dvd_menu );

		Display();
	}

	eOSState job_options_editor::store_options()
	{
		std::string error;
		if ( m_job.set_options( m_options, error ) )
			return osBack;

		Skins.Message( mtError, error.c_str() );
		return osContinue;
	}

	eOSState job_options_editor::ProcessKey( eKeys key_ )
	{
		eOSState state = cOsdMenu::ProcessKey( key_ );
		if ( state != osUnknown ) {
			check_item_states();
			return state;
		}

		if ( key_ == kOk )
			state = store_options();
		return state;
	}

}
