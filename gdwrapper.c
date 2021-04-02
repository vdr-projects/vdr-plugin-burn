/*
 * See the files COPYING and README for copyright information and how to reach
 * the author.
 *
 *  $Id: gdwrapper.c,v 1.10 2006/09/16 18:33:36 lordjaxom Exp $
 */

#include "common.h"
#include "gdwrapper.h"
#include "iconvwrapper.h"
#include "proctools/format.h"
#include "proctools/logger.h"
#include <vdr/i18n.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace gdwrapper
{

	using namespace std;
	using vdr_burn::user_exception;
	using proctools::format;
	using proctools::logger;

	inline char mytolower( char c ) { return tolower( c ); }

	// --- image --------------------------------------------------------------

	image::image( const string& filename, bool alphaBlending ):
			m_image( 0 )
	{
		if ( filename.length() < 4 ) {
			logger::error( format( "couldn't open image {0} for reading: filename too short to determine format" )
						   % filename, false );
			throw user_exception( tr("Couldn't open image file!") );
		}

		FILE* fp = fopen( filename.c_str(), "rb" );
		if (fp == 0) {
			logger::error( format( "couldn't open image {0} for reading" ) % filename );
			throw user_exception( tr("Couldn't open image file!") );
		}

		string extension;
		extension.resize( 4 );
		transform( filename.begin() + filename.length() - 4, filename.end(), extension.begin(), mytolower );
		if ( extension == ".png" )
			m_image = gdImageCreateFromPng( fp );
		else if ( extension == ".jpg" || extension == ".jpeg" )
			m_image = gdImageCreateFromJpeg( fp );
		else {
			logger::error( format( "couldn't read image {0}: unsupported file format" ) % filename, false );
			throw user_exception( tr("Couldn't open image file!") );
		}
		fclose( fp );

		if (m_image == 0) {
			logger::error( format( "couldn't read image {0}: possibly corrupt" ) % filename, false );
			throw user_exception( tr("Couldn't open image file!") );
		}
		gdImageSaveAlpha( m_image, 1 );
		gdImageAlphaBlending( m_image, alphaBlending ? 1 : 0 );
	}

	image::image( int width, int height, bool alphaBlending ):
			m_image( gdImageCreateTrueColor( width, height ) )
	{
		if (m_image == 0) {
			logger::error( "couldn't create new image: possibly out of memory" );
			throw user_exception( tr("Couldn't create new image!") );
		}
		gdImageSaveAlpha( m_image, 1 );
		gdImageAlphaBlending( m_image, alphaBlending ? 1 : 0 );
	}

	image::~image()
	{
		if (m_image != 0)
			gdImageDestroy( m_image );
	}

	void image::draw_image( const point& position, const image& image_ )
	{
		gdImageCopy( m_image, image_.m_image, position.x, position.y, 0, 0, image_.m_image->sx, image_.m_image->sy );
	}


	// wrap a given text string so that it doesn't break out of rectangle
	// returns wrapped string and lineHeight
	string image::wrap_text( int width, int& lineHeight, const string& text, const font_spec& font )
	{
		static const string delimiters( " -\n" );  // "_" deleted

		const string fontPath( setup::get_font_path() + "/" + font.face + ".ttf" );
		string result;
		string oldresult;
		string::const_iterator lineStart = text.begin();
		string::const_iterator lastpos = lineStart;
		lineHeight = 0;

		// loop over each char of text
		string::const_iterator pos = text.begin();
		while ( pos < text.end()) {

			if ( (pos+1 == text.end() ) || (delimiters.find( *pos ) != string::npos) ) {
				// pos zeigt auf delimiter oder ist am Ende => Prüfen ob länger als erlaubt oder CR
				string sample( lineStart, pos+1 );  // sample = text von lineStart bis pos)

				// render sample
				int bounds[8];
				const char* error = gdImageStringFT( 0, bounds, 0, const_cast<char*>( fontPath.c_str() ), font.size, 0, 0, 0,
								     const_cast<char*>( sample.c_str() ) );
				if (error != 0) {
					string message( error );
					if ( message == "Could not find/open font" )
						message += " " + fontPath;
					logger::error( format( "Couldn't render text: {0}" ) % message, false );
					throw user_exception( tr("Couldn't render menu images!") );
				}
				///isyslog("wt: %3d/%3d %2d=%s", lineHeight, bounds[3]-bounds[5], sample.length(), sample.c_str());

				// bestimme neue Zeilenhöhe
				int currentLineHeight = bounds[3] - bounds[5];
				if (currentLineHeight > lineHeight)
					lineHeight = currentLineHeight;

				// länger als erlaubt? dann neue Zeile
				int lineWidth = bounds[2] - bounds[0];
				if (lineWidth>width) {
					result += string(lineStart, lastpos);
					result += '\n';
					///isyslog("wt1: %s", string(lineStart, lastpos).c_str());
					pos = lineStart = lastpos+1;
				}
				else {
					if (*pos == '\n' || pos+1 == text.end()) {
						result += string(lineStart, pos+1);
						///isyslog("wt2: %s", string(lineStart, pos+1).c_str());
						lineStart = pos+1;
						lastpos = pos+1;
					}
					else
						lastpos = pos;
				}
			}

			pos++;
		}
		return result;
	}

	int image::draw_text( const text_spec& region, const std::string& text, int firstLine )
	{
		if ( text.empty() )
			return -1;

		const string fontPath( setup::get_font_path() + "/" + region.font.face + ".ttf" );
		int lineh;
		string wrappedText = wrap_text( region.boundaries.width, lineh, text, region.font );

		int y_ = region.boundaries.y;
		int lines = 0, width_; //, height_;
		int white = gdImageColorResolveAlpha(m_image, 255, 255, 255, 0);
		int bounds[8];

		if (lineh == 0)
			abort();

		string::size_type pos = 0;
		string::size_type next;
		while ( y_ + lineh < region.boundaries.y + region.boundaries.height - 1 &&
				( next = wrappedText.find( '\n', pos ) ) != string::npos ) {
			string line( wrappedText.substr( pos, next - pos ) );

			if (lines++ >= firstLine) {
				gdImageStringFT( 0, bounds, 0, const_cast<char*>( fontPath.c_str() ),
								 region.font.size, 0, 0, 0, const_cast<char*>( line.c_str() ) );
				width_ = bounds[2] - bounds[0] + 1;
				//height_ = bounds[3] - bounds[1] + 1;
				if ( region.align == 0 )
					gdImageStringFT( m_image, bounds, white, const_cast<char*>( fontPath.c_str() ),
									 region.font.size, 0, region.boundaries.x, y_ + lineh,
									 const_cast<char*>( line.c_str() ) );
				else if (region.align == 2)
					gdImageStringFT( m_image, bounds, white, const_cast<char*>( fontPath.c_str() ),
									 region.font.size, 0, region.boundaries.x + region.boundaries.width - width_ - 1,
									 y_ + lineh, const_cast<char*>( line.c_str() ) );
				y_ += lineh;
			}

			pos = next + 1;
			if ( pos == wrappedText.length() ) {
				lines = -1;
				break;
			}
		}

		if ( pos < wrappedText.length() && y_ + lineh < region.boundaries.y + region.boundaries.height) {
			string rest( wrappedText.substr( pos ) );
			gdImageStringFT( 0, bounds, 0, const_cast<char*>( fontPath.c_str() ), region.font.size, 0, 0, 0,
							 const_cast<char*>( rest.c_str() ) );
			width_ = bounds[2] - bounds[0] + 1;
			//height_ = bounds[3] - bounds[1] + 1;
			if (region.align == 0)
				gdImageStringFT( m_image, bounds, white, const_cast<char*>( fontPath.c_str() ),
								 region.font.size, 0, region.boundaries.x, y_ + lineh, const_cast<char*>( rest.c_str() ) );
			else if (region.align == 2)
				gdImageStringFT( m_image, bounds, white, const_cast<char*>( fontPath.c_str() ),
								 region.font.size, 0, region.boundaries.x + region.boundaries.width - width_ - 1, y_ + lineh,
								 const_cast<char*>( rest.c_str() ) );
			lines = -1;
		}

		return lines;
	}

	void image::fill_rectangle( const rectangle& rectangle, const color& color )
	{
		int bincolor = gdImageColorResolveAlpha( m_image, color.red, color.green, color.blue, color.alpha );
		gdImageFilledRectangle( m_image, rectangle.x, rectangle.y,
								rectangle.x + rectangle.width - 1, rectangle.y + rectangle.height - 1, bincolor );
	}

	void image::save( const std::string& filename )
	{
		FILE* fp = fopen( filename.c_str(), "wb" );
		if (fp == 0) {
			logger::error( format( "Couldn't open image {0} for writing" ) % filename );
			throw user_exception( tr("Couldn't save image file!") );
		}

		gdImagePng( m_image, fp );
		fclose( fp );
	}

	// --- setup --------------------------------------------------------------

	setup& setup::instance()
	{
		static setup instance_;
		return instance_;
	}

	void setup::set_font_path( const std::string& path )
	{
		instance().m_fontPath = path;
	}

}
