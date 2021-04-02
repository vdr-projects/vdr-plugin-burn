/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: common.h,v 1.29 2006/10/01 21:22:27 lordjaxom Exp $
 */

#ifndef VDR_BURN_COMMON_H
#define VDR_BURN_COMMON_H

#include "etsi-const.h"
#include "proctools/functions.h"
#include <stdexcept>
#include <string>
#include <vector>
#include <stdint.h>

#define JOBNAMELEN 255
#define MAXATRACKS (MAXAPIDS + MAXDPIDS)

class cRecording;

namespace vdr_burn
{

#ifndef NDEBUG
#	define BURN_THROW_DEBUG(msg) throw std::runtime_error( msg );
#else
#	define BURN_THROW_DEBUG(msg)
#endif

#if 0 // maybe later
	// helper class to encapsulate all enumerations into integers, in order to
	// allow editing in VDR's setup items (since enum is not guaranteed to be
	// of type integer)
	template<typename Enum>
	class int_enum
	{
	public:
		int_enum() { }
		int_enum(const Enum& value): m_value( value ) { }

		operator Enum() { return Enum( m_value ); }
		operator int&() { return m_value; }
		int_enum<Enum>& operator=(const Enum& value) { m_value = value; return *this; }

	private:
		int m_value;
	};
#endif

	//!--- protected_array ----------------------------------------------------

	template<typename Type>
	class protected_array
	{
	public:
		protected_array( Type* data = 0 ): m_data( data ) {}
		~protected_array() { delete[] m_data; }

		void reset( Type* data = 0 ) { delete[] m_data; m_data = data; }

		Type& operator[]( size_t index ) const { return m_data[ index ]; }
		Type* get() const { return m_data; }

	private:
		protected_array( const protected_array<Type>& ) {}
		protected_array<Type>& operator=( const protected_array<Type>& ) {}

		Type* m_data;
	};

	//!--- user_exception -----------------------------------------------------

	class user_exception: public std::runtime_error
	{
	public:
		user_exception(const std::string& message): std::runtime_error( message ) {}
	};

	//!--- path_pair ----------------------------------------------------------

	struct path_pair
	{
		std::string temp;
		std::string data;
		path_pair() {}
		path_pair(const std::string& temp_, const std::string& data_):
			temp(temp_), data(data_) {}
	};

	//!--- size_pair ----------------------------------------------------------

	template<typename Ty>
	struct size_pair_t
	{
		typedef Ty size_type;

		size_pair_t(size_type uncut, size_type cut): uncut( uncut ), cut( cut ) { }

		template<typename From>
		size_pair_t(const size_pair_t<From>& from): uncut( from.uncut ), cut( from.cut ) { }

		size_type uncut;
		size_type cut;
	};

	typedef size_pair_t<uint64_t> size_pair;
	typedef size_pair_t<int> length_pair;

	//!--- string_list --------------------------------------------------------

	typedef std::vector< std::string > string_list;

	//!--- disktype -----------------------------------------------------------

	enum disktype
	{
		disktype_dvd_menu,
		disktype_dvd_nomenu,
#ifdef ENABLE_DMH_ARCHIVE
		disktype_archive
#endif
	};

#ifdef ENABLE_DMH_ARCHIVE
	const int disktype_count( disktype_archive + 1 );
#else
	const int disktype_count( disktype_dvd_nomenu + 1 );
#endif
	const int disktype_countrequant( disktype_dvd_nomenu + 1 );
	extern const char* disktype_strings[disktype_count];

	//!--- storemode ----------------------------------------------------------

	enum storemode
	{
		storemode_create,
		storemode_burn,
		storemode_createburn
	};

	const int storemode_count( storemode_createburn + 1 );
	extern const char* storemode_strings[storemode_count];

	//!--- skinaspect ----------------------------------------------------------

	enum skinaspect
	{
		skinaspect_16_9,
		skinaspect_4_3
	};

	const int skinaspect_count( skinaspect_4_3 + 1 );
	extern const char* skinaspect_strings[skinaspect_count];

	//!--- chaptersmode -------------------------------------------------------

	enum chaptersmode
	{
		chaptersmode_none,
		chaptersmode_marks,
		chaptersmode_5,
		chaptersmode_10,
		chaptersmode_15,
		chaptersmode_30,
		chaptersmode_60
	};

	const int chaptersmode_count( chaptersmode_60 + 1 );
	extern const char* chaptersmode_strings[chaptersmode_count];
	extern const int chaptersmode_intervals[chaptersmode_count];

	//!--- disksize -----------------------------------------------------------

	enum disksize
	{
		disksize_singlelayer,
		disksize_doublelayer,
		disksize_custom
	};

	const int disksize_count( disksize_custom + 1 );
	extern const char* disksize_strings[disksize_count];
	extern const int disksize_values[disksize_count];

	//!--- demuxtype ----------------------------------------------------------

	enum demuxtype
	{
		demuxtype_projectx
	};

	const int demuxtype_count( demuxtype_projectx + 1 );
	extern const char* demuxtype_strings[demuxtype_count];

	//!--- requanttype --------------------------------------------------------

	enum requanttype
	{
		requanttype_m2vrequantiser,
		requanttype_transcode,
		requanttype_lxdvdrip,
	};

	const int requanttype_count = requanttype_lxdvdrip + 1;
	extern const char* requanttype_strings[requanttype_count];

	//!--- titletype --------------------------------------------------------

	enum titletype
	{
		titletype_epgtitle,
		titletype_epgtitleshorttext,
		titletype_epgshorttext,
		titletype_rectitle,
	};

	const int titletype_count = titletype_rectitle + 1;
	extern const char* titletype_strings[titletype_count];


	int ScanPageCount(const std::string& Path);
	std::string progress_bar(double current, double total, int length = 20);
	void trim_left( std::string& text_, const char* characters_, std::string::size_type offset_ = 0 );

	//!--- recording helper functions -----------------------------------------

	std::string get_recording_datetime(const cRecording* recording_, char delimiter = '\t');
	std::string get_recording_osd_line(const cRecording* recording_, int level);
	std::string get_recording_event_title(const cRecording* recording_);
	std::string get_recording_event_shorttext(const cRecording* recording_);
	std::string get_recording_event_description(const cRecording* recording_);
	std::string get_recording_name(const cRecording* recording);

	std::string string_replace( const std::string& text, char from, char to );
	bool elapsed_since(time_t& timestamp, time_t difference);
	std::string int_to_string(int value, int base, bool prefix = false);
	std::string clean_path_name(const std::string& text);
	std::string convert_to_utf8( const std::string& text );

	template<typename FwdIt, typename Fn>
	std::string join_strings(FwdIt first, FwdIt last, Fn func, std::string delimiter)
	{
		return proctools::sum(first, last, std::string( "" ), func, delimiter);
	}

	template<typename FwdIt, typename Fn>
	size_pair::size_type accumulate_size(FwdIt first, FwdIt last, Fn func)
	{
		return proctools::sum( first, last, size_pair::size_type( 0 ), func );
	}

}

#endif // VDR_BURN_COMMON_H
