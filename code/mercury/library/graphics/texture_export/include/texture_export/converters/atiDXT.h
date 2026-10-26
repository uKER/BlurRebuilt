#ifndef ATIDXT_H__INCLUDED
#define ATIDXT_H__INCLUDED

namespace ActTools
{
	class TexelFormat;
}

#define ATIDXT_ARGB_8888    "ARGB_8888"
#define ATIDXT_ARGB_2101010 "ARGB_2101010"
#define ATIDXT_ARGB_16      "ARGB_16"
#define ATIDXT_ARGB_32F     "ARGB_32F"
#define ATIDXT_DXT1         "DXT1"
#define ATIDXT_DXT3         "DXT3"
#define ATIDXT_DXT5         "DXT5"
#define ATIDXT_DXT5_xGBR    "DXT5_xGBR"
#define ATIDXT_DXT5_RxBG    "DXT5_RxBG"
#define ATIDXT_DXT5_RBxG    "DXT5_RBxG"
#define ATIDXT_DXT5_xRBG    "DXT5_xRBG"
#define ATIDXT_DXT5_RGxB    "DXT5_RGxB"
#define ATIDXT_DXT5_xGxR    "DXT5_xGxR"
#define ATIDXT_ATI1N        "ATI1N"
#define ATIDXT_ATI2N        "ATI2N"
#define ATIDXT_ATI2N_XY     "ATI2N_XY"
#define ATIDXT_ATI2N_DXT5   "ATI2N_DXT5"

namespace AtiDXT
{

ActTools::TexelFormat* GetTexelFormat( const char* aFormatId );

}

#endif // ATIDXT_H__INCLUDED

