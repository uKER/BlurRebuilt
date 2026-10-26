#include "stdafx.h"

#include "../../BundlerOutputOptions.h"
#include "../../Bundler.h"
#include "../../Utility.h"

#include <process_texture/ProcessedMipmapChain.h>

#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>

#include "../IExportTexture.h"
#include "ExportTextureWii.h"
#include "ExportArrayOfTextures2DWii.h"
#include "../OutputTextureData.h"

using namespace ResourceBundler;

C_ExportArrayOfTextures2DWii::C_ExportArrayOfTextures2DWii()
{
}

bool C_ExportArrayOfTextures2DWii::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;
	return true;
}

/*>*******************************(*)*******************************<*/
// From TCFixEndian in RVL samples
// Reverse the byte order within a block of bytes.
// 
/*>*******************************(*)*******************************<*/ 

static void s_tc_swap_endian( U8* src, U32 num_bytes )
{
	U8  tmp[8];  // large enough to hold a double
	U32 max, i;

	assert( num_bytes <= 8 ); // "s_tc_swap_endian: num_bytes > 8\n" );

	if( (num_bytes == 0) || (num_bytes == 1) )
	{
		return;
	}

	max = num_bytes - 1;
	for(i=0; i< num_bytes; i++)
	{
		tmp[(max - i)] = src[i];	
	}

	for(i=0; i< num_bytes; i++)
	{
		src[i] = tmp[i];
	}
}

/*>*******************************(*)*******************************<*/
// From TCFixCMPWord in the RVL samples
// switch tuple and byte order within 16-bit words of an s3-packed tile
// to match hw.
// 1) switch 2-bit tuple order within bytes
//    from ( 0,1,2,3 ) to ( 3,2,1,0 ).
// 2) leave byte order within the word as is.
/*>*******************************(*)*******************************<*/

static void s_tc_fix_cmp_word( U16* data )
{
	U16 tmp;

	tmp = *data;

	// reverse tuple order within bytes
	*data = ( (tmp & 0x3 )   << 6 ) |
		( (tmp & 0xC )   << 2 ) |
		( (tmp & 0x30)   >> 2 ) |
		( (tmp & 0xC0)   >> 6 ) |

		( (tmp & 0x300 ) << 6 ) |
		( (tmp & 0xC00 ) << 2 ) |
		( (tmp & 0x3000) >> 2 ) |
		( (tmp & 0xC000) >> 6 ) ;
}

/*>*******************************(*)*******************************<*/ 
// From TCPackTile_CMP in RVL samples
//
// pack a 2x2 tile block, each tile of 4x4 texels, into a single 
// 32B dst tile note: this assumes s3 algorithm pads out to a minimum 
// block size of 4x4 texels
/*>*******************************(*)*******************************<*/ 

void	pack_compressed_tile( U32 tile_x, U32 tile_y, U16* dst, U16* src_buffer, U32 width, U32 height )
{
	U32  x, y;
	U16* srcPtr;
	U16  tmp;
	U32  srcTileOffset;
	U32  subTileRows, subRowShorts;    // number of s3 4x4 tiles
	U32  srcPadWidth, srcPadHeight;
	U16* buffPtr;

	// set the padded size of the s3 source image out to a 4-texel boundary
	srcPadWidth  = ( (width  + 3) >> 2 );
	srcPadHeight = ( (height + 3) >> 2 );

	// number of bytes in a single row of 4x4 texel source tiles
	srcTileOffset = srcPadWidth * 8;

	// number of 4x4 (source) tile rows to copy ( will be 1 or 2 )
	subTileRows = 2;
	if( (srcPadHeight - tile_y) < 2 )
		subTileRows = 1;

	// number of 4x4 tile cols to copy translated into number of short values
	// ( will be 4 or 8 )
	subRowShorts = 8;
	if( (srcPadWidth - tile_x) < 2 )
		subRowShorts = 4;

	for( y=0; y < subTileRows; y++ )
	{
		srcPtr  = (U16*)( (U8*)(src_buffer) + ((tile_y + y) * srcTileOffset) + (tile_x*8) ); 
		buffPtr = ( dst + (y * 8) );        // 16 bytes per subRow = 8 shorts

		// process one or both 4x4 row tiles at once- 4 short each
		for( x=0; x < subRowShorts; x++ )
		{			
			switch( x )
			{

				// color table entries - switch bytes within a 16-bit world only
			case 0:	
			case 1:
			case 4:
			case 5:
				tmp = *srcPtr++;
				s_tc_swap_endian( (U8*)(&tmp), 2 );
				*buffPtr++ = tmp;
				break;

				// 2-bit color tuples;
				// reverse tuple order within bytes of a word
			case 2:
			case 3:
			case 6:
			case 7:
				tmp = *srcPtr++;
				s_tc_fix_cmp_word( &tmp );
				*buffPtr++ = tmp;
				break;

			} // end switch
		} // end for( subRowShorts )			
	} // end for( subTileRows )
}

/*>*******************************(*)*******************************<*/ 
// From TCWriteTplImage_CMP in RVL samples
//
/*>*******************************(*)*******************************<*/ 

void	tile_compressed_image( U8* dest_buffer, U16* src_buffer, U32 width, U32 height )
{
	U32 tileRow, tileCol;
	U32 srcTileRows, srcTileCols;
	U16* dst;

	// each source tile is 4x4 texels, 8B
	srcTileRows   = (( height + 3) >> 2);
	srcTileCols   = (( width  + 3) >> 2);

	dst = (U16*)(dest_buffer);

	// each dst tile is 2x2 source tiles, so move by 2 each iteration
	for(tileRow = 0; tileRow < srcTileRows; tileRow += 2 )
	{
		for(tileCol = 0; tileCol < srcTileCols; tileCol += 2 )
		{
			pack_compressed_tile( tileCol, tileRow, dst, src_buffer, width, height );
			dst += 16; // 32B per dst tile, short ptr
		}
	}
}

// Tiles texel data (this is how the Wii expects the data)
U8* TileTexelData( void *src_buffer, U32 width, U32 height, U32 bpp )
{
	size_t size = (size_t)( (float)width * (float)height * ( bpp / 8.0f ) );
	U8 *dst_buffer = new U8[ size ];
	memset( dst_buffer, 0, size );

	switch( bpp )
	{
		case 4:
		{
			tile_compressed_image( dst_buffer, (U16 *)src_buffer, width, height );
		} break;

		case 8:
		{
			// Tiles for 8-bit textures are 4(h)x8(w) texels
			U8 *src_texel_data = (U8 *)src_buffer;
			U8 *dst_cur_spot = dst_buffer;
			for( U32 y = 0; y < height; y += 4 )
			{
				for( U32 x = 0; x < width; x += 8 )
				{
					U8 *src_cur_spot = &src_texel_data[ y * width + x ];

					for( U32 count = 0; count < 4; count++ )
					{
						memcpy( dst_cur_spot, src_cur_spot, 8 );
						dst_cur_spot += 8;
						src_cur_spot += width;
					}
				}
			}
		} break;

		case 16:
		{
			// Tiles for 16-bit textures are 4(h)x4(w) texels
			U16 *src_texel_data = (U16 *)src_buffer;
			U16 *dst_cur_spot = (U16 *)dst_buffer;
			for( U32 y = 0; y < height; y += 4 )
			{
				for( U32 x = 0; x < width; x += 4 )
				{
					U16 *src_cur_spot = &src_texel_data[ y * width + x ];

					for( U32 count = 0; count < 4; count++ )
					{
						memcpy( dst_cur_spot, src_cur_spot, 8 );
						dst_cur_spot += 4;
						src_cur_spot += width;
					}
				}
			}
		} break;

		case 32:
		{
			// Tiles for 32-bit textures are 4(h)x4(w) texels (2 cache lines, AR and GB)
			U32 *src_texel_data = (U32 *)src_buffer;
			U32 *dst_cur_spot = (U32 *)dst_buffer;
			for( U32 y = 0; y < height; y += 4 )
			{
				for( U32 x = 0; x < width; x += 4 )
				{
					U32 *src_cur_spot = &src_texel_data[ y * width + x ];

					// Write out 4x4 block of AR values
					U16 *src_16 = (U16 *)src_cur_spot;
					for( U32 count = 0; count < 4; count++ )
					{
#ifdef FLIP_COLORS
						//U16 rg[ 4 ] = { src_16[ 0 ], src_16[ 2 ], src_16[ 4 ], src_16[ 6 ] };
						//U16 ba[ 4 ] = { src_16[ 1 ], src_16[ 3 ], src_16[ 5 ], src_16[ 7 ] };
						//U16 ar[ 4 ];
#else
						U16 ar[ 4 ] = { src_16[ 1 ], src_16[ 3 ], src_16[ 5 ], src_16[ 7 ] };
#endif

						// Texels come in as BGRA so we have to swap the upper and lower bytes (RA becomes AR)
						for( U32 ar_cur = 0; ar_cur < 4; ar_cur++ )
						{
#ifdef FLIP_COLORS
							U8 r = ( rg[ ar_cur ] >> 8 ) & 0x00FF;
							U8 a = ( ba[ ar_cur ] & 0x00FF );
							ar[ ar_cur ] = r | ( a << 8 );
#else
							U8 upper_byte = ( ar[ ar_cur ] >> 8 ) & 0x00FF;
							U8 lower_byte = ar[ ar_cur ] & 0x00FF;
							ar[ ar_cur ] = upper_byte | ( lower_byte << 8 );
#endif
						}

						memcpy( dst_cur_spot, ar, 8 );
						dst_cur_spot += 2;
						src_16 += width * 2;
					}

					// Write out 4x4 block of GB values
					src_16 = (U16 *)src_cur_spot;
					for( U32 count = 0; count < 4; count++ )
					{
						U16 gb[ 4 ] = { src_16[ 0 ], src_16[ 2 ], src_16[ 4 ], src_16[ 6 ] };
						// Texels come in as BGRA so we have to swap the upper and lowet bytes (BG becomes GB)
						for( U32 gb_cur = 0; gb_cur < 4; gb_cur++ )
						{
							U8 upper_byte = ( gb[ gb_cur ] >> 8 ) & 0x00FF;
							U8 lower_byte = gb[ gb_cur ] & 0x00FF;
							gb[ gb_cur ] = upper_byte | ( lower_byte << 8 );
						}

						memcpy( dst_cur_spot, gb, 8 );
						dst_cur_spot += 2;
						src_16 += width * 2;
					}
				}
			}
		} break;
	}

	memcpy( src_buffer, dst_buffer, size );
	return dst_buffer;
}

bool C_ExportArrayOfTextures2DWii::_Export(
	C_OutputTextureData& output, 
	const C_BundlerOutputOptions& bundlerOptions,
	const C_ExportTextureOptions& exportOptions)
{
	U32 mipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 numImages = m_mipmapChains.Size();

	m_textureFormat = exportOptions.outputFormat;

	if (!output.AllocateStorage(numImages, mipLevels))
		return false;

	U8* pOutput;

	//Initialise output storage for our mip levels.
	{
		U32 baseSize = m_mipmapChains.Front()->GetBaseSize() * numImages;
		U32 mipSize = m_mipmapChains.Front()->GetMipSize() * numImages;

		pOutput = output.IncreaseStorage(baseSize + mipSize);
		if (!pOutput)
			return false;
		// clear any unused areas of the texture
		memset(pOutput , 127, baseSize + mipSize);
	}

	U32 depth = 0;
	switch (m_mipmapChains.Front()->GetFormat())
	{
		case Display::ResourceBundler::E_TextureFormat_A8R8G8B8:
			depth = 32;
			break;
		case Display::ResourceBundler::E_TextureFormat_L8:
			//No swap
			depth = 8;
			break;
		
		case Display::ResourceBundler::E_TextureFormat_DXT1:
			depth = 4;
			break;

		default:
			printf("Unsupported format for Wii.\n");
			return false;
	}

	U32 nMipLevels = m_mipmapChains.Front()->GetNumLevels();
	U32 mipOffset = 0;
	for (U32 iLevel = 0; iLevel < nMipLevels; iLevel++)
	{
		U32 imageIndex = 0;
		for (
			Ut::UtList<C_ProcessedMipmapChain*>::Iterator iter = m_mipmapChains.Begin(); 
			iter != m_mipmapChains.End();
			++imageIndex,
			++iter)
		{
			C_ProcessedMipmapChain* pMipChain = *iter;
			//Set appropriate source format. If uncompressed it will be E_TextureFormat_A32R32G32B32.
			//If compressed then it should be equal to the output format.
			if (pMipChain->GetFormat() == Display::ResourceBundler::E_TextureFormat_A32R32G32B32)
			{
				printf("Uncompressed generic textures not supported yet.\n");
				return false;
			}
			else
			{
				if (pMipChain->GetFormat() != m_textureFormat)
				{
					assert(!"Texture format mismatch");
					return false;
				}
			}

			U32 mipSize = pMipChain->GetLevelSize(iLevel);
			U8* pSource = pMipChain->GetLevelPtr(iLevel);

			if(	( pMipChain->GetWidth( iLevel ) >= 4 ) &&
				( pMipChain->GetHeight( iLevel ) >= 4 ))
			{
				pSource = TileTexelData( pSource, pMipChain->GetWidth( iLevel ), 
										pMipChain->GetHeight( iLevel ), depth );
			}

			//Copy out our data.
			memcpy(
				pOutput+mipOffset,
				pSource,
				mipSize);

			//Let's set the mip header.
			output.SetMipInfo(
				imageIndex,
				iLevel, 
				pOutput+mipOffset,
				mipSize);

			//Increase offset.
			mipOffset += mipSize;
		}
	}

	return true;
}
