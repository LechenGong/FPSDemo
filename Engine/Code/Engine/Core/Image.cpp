#include "Engine/Core/Image.hpp"
#include "ThirdParty/stbi/stb_image.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <string>

Image::Image()
{
}

Image::~Image()
{
}

Image::Image( char const* imageFilePath )
	: m_imageFilePath( imageFilePath )
	, m_dimensions( IntVec2::ZERO )
{
	int width, height, bpp;
	stbi_set_flip_vertically_on_load( 1 );
	unsigned char* pixels = stbi_load( imageFilePath, &width, &height, &bpp, 0 );
	if (pixels == nullptr)
	{
		ERROR_RECOVERABLE( Stringf( "Fail Load Image: %s", imageFilePath ) );
		stbi_image_free( pixels );
		return;
	}
	m_dimensions = IntVec2( width, height );
	m_bitPerPixel = bpp;
	m_rgbaTexels.resize( width * height );
	for (int i = 0; i < width * height * m_bitPerPixel; ++i)
	{
		int index = i / m_bitPerPixel;
		int mod = i - index * m_bitPerPixel;
		if (m_bitPerPixel == 3)
		{
			switch (mod)
			{
			case 0:
				m_rgbaTexels[index].r = pixels[i];
				break;
			case 1:
				m_rgbaTexels[index].g = pixels[i];
				break;
			case 2:
				m_rgbaTexels[index].b = pixels[i];
				break;
			}
			m_rgbaTexels[index].a = 255;
		}
		else if (m_bitPerPixel == 4)
		{
			switch (mod)
			{
			case 0:
				m_rgbaTexels[index].r = pixels[i];
				break;
			case 1:
				m_rgbaTexels[index].g = pixels[i];
				break;
			case 2:
				m_rgbaTexels[index].b = pixels[i];
				break;
			case 3:
				m_rgbaTexels[index].a = pixels[i];
				break;
			}
		}
	}
 	stbi_image_free( pixels );
}

Image::Image( IntVec2 size, Rgba8 color )
	: m_dimensions( size )
	, m_imageFilePath( "" )
{
	m_bitPerPixel = 4;
	m_rgbaTexels.reserve( m_dimensions.x * m_dimensions.y );
	for (int i = 0; i < size.x; i++)
	{
		for (int j = 0; j < size.y; j++)
		{
			m_rgbaTexels.push_back( color );
		}
	}
}

std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

void const* Image::GetRawData() const
{
	return m_rgbaTexels.data();
}

Rgba8 Image::GetTexelColor( IntVec2 const& texelCoords ) const
{
	int texelIndex = texelCoords.y * m_dimensions.x + texelCoords.x;
	return m_rgbaTexels[texelIndex];
}

void Image::SetTexelColor( IntVec2 const& texelCoords, Rgba8 const& newColor )
{
	int texelIndex = texelCoords.y * m_dimensions.x + texelCoords.x;
	m_rgbaTexels[texelIndex] = newColor;
}
