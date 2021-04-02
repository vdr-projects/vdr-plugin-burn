#include "chain-vdr.h"
#include "chain-dvd.h"
#include "logger-vdr.h"
#include "setup.h"
#include "menuburn.h"
#include "proctools/format.h"
#include "proctools/logger.h"
#include "proctools/shellprocess.h"
#include "proctools/shellescape.h"
#include <memory>
#include <vdr/config.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef ENABLE_DMH_ARCHIVE
#include "chain-archive.h"
#endif

namespace vdr_burn
{

	using namespace std;
	using proctools::chain;
	using proctools::logger;
	using proctools::format;
	using proctools::process;
	using proctools::shellescape;

	chain_vdr::chain_vdr(const string& name, job& job_):
			chain(name),
			cThread("burn: subprocess watcher"),
			m_job( job_ ),
			m_paths( create_temp_path( BurnParameters.TempPath ), create_temp_path( BurnParameters.DataPath ) ),
			m_progress(0),
			m_burning(false),
			m_burnProgress(0),
			m_canceled(false)
	{
		job_.set_paths(m_paths);
		logger_vdr::set_logfile( get_log_path() );
		logger::debug(format ("writing detail log to {0}") % get_log_path());
	}

	chain_vdr::~chain_vdr()
	{
		stop();
		if (!BurnParameters.KeepTempFiles) {
			if (return_status() != process::ok)
				execute( shellescape( "rm" ) + "-rf" + m_job.get_iso_path() );
			execute( shellescape( "rm" ) + "-rf" + m_paths.temp );
			execute( shellescape( "rm" ) + "-rf" + m_paths.data );
		}
	}

	chain_vdr* chain_vdr::create_chain(job& job)
	{
		switch (job.get_disk_type()) {
		case disktype_dvd_menu:
		case disktype_dvd_nomenu: return new chain_dvd(job);

#ifdef ENABLE_DMH_ARCHIVE
		case disktype_archive:    return new chain_archive(job);
#endif
		}

		logger::error( "chain_vdr::create_chain requested for unknown disktype" );
		BURN_THROW_DEBUG( "chain_vdr::create_chain requested for unknown disktype" );
		return 0;
	}

	void chain_vdr::stop()
	{
		logger::debug("stopping chain");
		m_canceled = true;
		chain::stop();
		Cancel(20);
	}

	void chain_vdr::add_process(proctools::process* proc)
	{
		proc->set_workdir(m_paths.data);
		chain::add_process(proc);
	}

	string chain_vdr::create_temp_path( const string& pathPrefix )
	{
		static const string pathTemplate( "{0}/vdr-burn.{1}.XXXXXX" );

		string tempPath( format( pathTemplate ) % pathPrefix % string_replace( m_job.get_title(), '/', '~' ) );
		string cleanPath( clean_path_name( tempPath ) );
		protected_array<char> path( new char[ cleanPath.length() + 1 ] );
		strcpy( path.get(), cleanPath.c_str() );
		mkdtemp( path.get() );
		return path.get();
	}

	void chain_vdr::all_done()
	{
		logger_vdr::close_logfile();

		if (global_setup().PreserveLogFiles) {
			string target = BurnParameters.IsoPath.empty() ? BurnParameters.TempPath : BurnParameters.IsoPath;
			logger::debug(format ("moving detail log to {0}/vdrburn-{1}.log") % target % m_job.get_title() );
			execute( shellescape( "cp" ) + get_log_path() +
					 str( format( "{0}/vdrburn-{1}.log" ) % target % m_job.get_title() ) );
		}
	}

	void chain_vdr::Action()
	{
		SetPriority(19);
		run();
	}
};
