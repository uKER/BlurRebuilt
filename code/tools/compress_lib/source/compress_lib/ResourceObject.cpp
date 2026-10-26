#include "stdafx.h"

using namespace CompressLib;

#if !defined(ENABLE_XDK) && !defined(ENABLE_ATI) && !defined(ENABLE_NVIDIA)
#error At least one conversion library must be enabled
#endif


#define FORMAT_LOOKUP(lib, fmt) { lib, #fmt, ##fmt }

static TextureFormat gTextureFormats[] = 
{
#if defined(ENABLE_XDK)
	// --- ATI Formats --------------------------------------------------------

	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_UNKNOWN				),

    // DXT:
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXT1					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXT2					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXT3					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXT4					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXT5					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXN					),

    // 8bpp:
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A8						),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_L8						),

    // 16bpp:
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_R5G6B5					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_R6G5B5					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_L6V5U5					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_X1R5G5B5				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A1R5G5B5				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A4R4G4B4				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_X4R4G4B4				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_Q4W4V4U4				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A8L8					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_G8R8					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_V8U8					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_D16					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_L16					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_R16F					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_R16F_EXPAND			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_UYVY					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_LE_UYVY				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_G8R8_G8B8				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_R8G8_B8G8				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_YUY2					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_LE_YUY2				),

    // 32bpp:
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A8R8G8B8				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_X8R8G8B8				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A8B8G8R8				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_X8B8G8R8				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_X8L8V8U8				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_Q8W8V8U8				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A2R10G10B10			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_X2R10G10B10			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A2B10G10R10			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A2W10V10U10			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A16L16					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_G16R16					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_V16U16					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_R10G11B11				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_R11G11B10				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_W10V11U11				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_W11V11U10				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_G16R16F				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_G16R16F_EXPAND			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_L32					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_R32F					),

    // 64bpp:
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A16B16G16R16			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_Q16W16V16U16			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A16B16G16R16F			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A16B16G16R16F_EXPAND	),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A32L32					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_G32R32					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_V32U32					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_G32R32F				),

    // 128bpp:
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A32B32G32R32			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_Q32W32V32U32			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_A32B32G32R32F			),

    // Front buffer formats
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_LE_X8R8G8B8			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_LE_A8R8G8B8			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_LE_X2R10G10B10			),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_LE_A2R10G10B10			),

    // Other
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXT3A					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXT3A_1111				),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_DXT5A					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_CTX1					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_D24S8					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_D24X8					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_D24FS8					),
	FORMAT_LOOKUP(	E_CompressionLibrary_X360	, D3DFMT_D32					),
#endif


#if defined(ENABLE_ATI)
	// --- ATI Formats --------------------------------------------------------

	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_ARGB_8888		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_ARGB_2101010	),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_ARGB_16			),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_ARGB_32F		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT1			),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT3			),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT5			),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT5_xGBR		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT5_RxBG		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT5_RBxG		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT5_xRBG		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT5_RGxB		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_DXT5_xGxR		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_ATI1N			),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_ATI2N			),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_ATI2N_XY		),
	FORMAT_LOOKUP(	ECL_ATI		, ATI_TC_FORMAT_ATI2N_DXT5		),
#endif


#if defined(ENABLE_NVIDIA)
	// --- NVidia Formats -----------------------------------------------------

	FORMAT_LOOKUP(	ECL_NVidia	, kDXT1				),
	FORMAT_LOOKUP(	ECL_NVidia	, kDXT1a			),		// DXT1 with one bit alpha
	FORMAT_LOOKUP(	ECL_NVidia	, kDXT3				),		// explicit alpha
	FORMAT_LOOKUP(	ECL_NVidia	, kDXT5				),		// interpolated alpha
	FORMAT_LOOKUP(	ECL_NVidia	, k4444				),		// a4 r4 g4 b4
	FORMAT_LOOKUP(	ECL_NVidia	, k1555				),		// a1 r5 g5 b5
	FORMAT_LOOKUP(	ECL_NVidia	, k565				),		// a0 r5 g6 b5
	FORMAT_LOOKUP(	ECL_NVidia	, k8888				),		// a8 r8 g8 b8
	FORMAT_LOOKUP(	ECL_NVidia	, k888				),		// a0 r8 g8 b8
	FORMAT_LOOKUP(	ECL_NVidia	, k555				),		// a0 r5 g5 b5
	FORMAT_LOOKUP(	ECL_NVidia	, kP8c				),		// paletted color only
	FORMAT_LOOKUP(	ECL_NVidia	, kV8U8				),		// DuDv 
	FORMAT_LOOKUP(	ECL_NVidia	, kCxV8U8			),		// normal map
	FORMAT_LOOKUP(	ECL_NVidia	, kA8				),		// alpha only
	FORMAT_LOOKUP(	ECL_NVidia	, kP4c				),		// 16 bit color palette
	FORMAT_LOOKUP(	ECL_NVidia	, kQ8W8V8U8			),
	FORMAT_LOOKUP(	ECL_NVidia	, kA8L8				),
	FORMAT_LOOKUP(	ECL_NVidia	, kR32F				),
	FORMAT_LOOKUP(	ECL_NVidia	, kA32B32G32R32F	),
	FORMAT_LOOKUP(	ECL_NVidia	, kA16B16G16R16F	),
	FORMAT_LOOKUP(	ECL_NVidia	, kL8				),		// luminance
	FORMAT_LOOKUP(	ECL_NVidia	, kP8a				),		// paletted with alpha
	FORMAT_LOOKUP(	ECL_NVidia	, kP4a				),		// 16 bit color palette with alpha
	FORMAT_LOOKUP(	ECL_NVidia	, kR16F				),		// single component fp16
	FORMAT_LOOKUP(	ECL_NVidia	, kDXT5_NM			),		// normal map compression.  G = Y, alpha = X
	FORMAT_LOOKUP(	ECL_NVidia	, kX888				),		// aX r8 g8 b8
	FORMAT_LOOKUP(	ECL_NVidia	, kV16U16			),
	FORMAT_LOOKUP(	ECL_NVidia	, kG16R16			),
	FORMAT_LOOKUP(	ECL_NVidia	, kG16R16F			),
	FORMAT_LOOKUP(	ECL_NVidia	, k3Dc				),
/*
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_R8G8B8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A8R8G8B8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_X8R8G8B8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_R5G6B5				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_X1R5G5B5				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A1R5G5B5				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A4R4G4B4				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_R3G3B2				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A8					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A8R3G3B2				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_X4R4G4B4				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A2B10G10R10			),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A8B8G8R8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_X8B8G8R8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_G16R16				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A2R10G10B10			),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A16B16G16R16			),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A8P8					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_P8					),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_L8					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A8L8					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A4L4					),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_V8U8					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_L6V5U5				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_X8L8V8U8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_Q8W8V8U8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_V16U16				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A2W10V10U10			),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_UYVY					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_R8G8_B8G8			),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_YUY2					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_G8R8_G8B8			),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_DXT1					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_DXT2					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_DXT3					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_DXT4					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_DXT5					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_3Dc					),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D16_LOCKABLE			),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D32					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D15S1				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D24S8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D24X8				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D24X4S4				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D16					),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D32F_LOCKABLE		),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_D24FS8				),


	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_L16					),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_VERTEXDATA			),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_INDEX16				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_INDEX32				),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_Q16W16V16U16			),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_MULTI2_ARGB8			),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_R16F					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_G16R16F				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A16B16G16R16F		),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_R32F					),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_G32R32F				),
	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_A32B32G32R32F		),

	FORMAT_LOOKUP(	ECL_NVidia	, nvD3DFMT_CxV8U8				),
*/
#endif

//	E_CompressionLibrary_Unknown, NULL, 0xffffffff
};



TextureFormat *FormatFromString(Util::C_String format, bool *pLinear /*=0*/)
{
	if(format == "")
		return NULL;

	if (!strncmp(format.Chars(), "D3DFMT_", 7))		// D3D Format?
	{
		if (!strncmp(format.Chars(), "D3DFMT_LIN_", 11))	// Skip the "D3DFMT_LIN_" prefix
		{
			if (pLinear)
				(*pLinear) = true;

			Util::C_String newFormat = "D3DFMT_";
			newFormat += format.Chars() + strlen("D3DFMT_LIN_");

			format = newFormat;
		}
		else if (!strncmp(format.Chars(), "D3DFMT_", 7))	// Skip the "D3DFMT_" prefix
		{
			if (pLinear)
				(*pLinear) = false;
		}
	}

	for (int i=0; gTextureFormats[i].string != NULL; i++)
	{
		if (format == gTextureFormats[i].string)
			return &gTextureFormats[i];
	}

	return NULL;
}


TextureFormat* CompressLib::FormatFromLibFormat(E_CompressionLibrary lib, U32 format)
{
	for (int i=0; gTextureFormats[i].string != NULL; i++)
	{
		if (gTextureFormats[i].lib == lib && gTextureFormats[i].format == format)
			return &gTextureFormats[i];
	}

	return NULL;
}



CResourceObject::CResourceObject()
{
	m_Name			= "";
	m_Format		= NULL;
	m_MipMapLevels	= 0;
	m_Linear		= false;
}

CResourceObject::~CResourceObject()
{
	Ut::UtList<Attk::C_Image*>::Iterator iter = m_Images.Begin();

	while (iter != m_Images.End())
	{
		delete *iter;

		++iter;
	}
}

bool CResourceObject::SetInfo(Util::C_String name, TextureFormat *format, U32 mipMapLevels)
{
	m_Name			= name;
	m_Format		= format;
	m_MipMapLevels	= mipMapLevels;

	return true;
}

bool CResourceObject::AddTexture(Attk::C_Image *img)
{
	m_Images.PushBack(img);
	
	return true;
}

U32 CResourceObject::Images(void)
{
	return m_Images.Size();
}


#if defined(ENABLE_XDK)

C_Compressor360::C_Compressor360()
{
	m_NonPackedMipTail	= false;
	m_Border			= false;
}

DWORD C_Compressor360::GetXboxFormat(void)
{
	DWORD format = m_Format->format;
	if (m_Linear)
		format = MAKELINFMT(format);

	return format;
}

bool C_Compressor360::SaveTexture(
	C_OutputTexture *pTex, 
	bool bigEndian, 
	bool tileTexture,
	bool swizzleTexture)
{
	
}

#endif



#if defined(ENABLE_NVIDIA)

CResourceObjectNVidia::CResourceObjectNVidia()
{
}

typedef struct
{
	U32		Size;
	U8		*buf;
} CONV_BUFFER;

bool CResourceObjectNVidia::SaveTexture(C_OutputTexture *pTex, bool bigEndian)
{
	U8 *pOutput = AddAlloc(pTex, sizeof(ResourceHeader));
	if (!pOutput)
		return false;

	GetOutputHeader((ResourceHeader*) pOutput);

	if (bigEndian)
		XGEndianSwapMemory(pOutput, pOutput, XGENDIAN_8IN32, sizeof(DWORD), sizeof(ResourceHeader) / sizeof(DWORD));

//	pBundler->WriteHeader(&header, sizeof(header));
//	(*pSize) += sizeof(header);

	C_Status status;
	int w, h;

	UtList<C_Image*>::Iterator iter = m_Images.Begin();

	while (iter != m_Images.End())
	{
		w = (*iter)->GetWidth();
		h = (*iter)->GetHeight();

		for (U32 level=0; level<m_MipMapLevels; level++)
		{
			(*iter)->Resize(w, h, (*iter)->GetDepth(), E_ResizeFilter_ScaleBox);

			nvCompressionOptions compOpts;
			CONV_BUFFER output;

			compOpts.textureFormat = (nvTextureFormats) m_Format->Format;

			compOpts.mipMapGeneration = kNoMipMaps;

			output.Size = 0;
			output.buf = NULL;

			compOpts.user_data = &output;

			nvDDS::nvDXTcompress((*iter)->GetPixelDataPtr(),
				(*iter)->GetWidth(), (*iter)->GetHeight(),
				(*iter)->GetWidth(),
				nvRGBA,
				&compOpts,
				WriteCallback);

//			if (!pBundler->WriteData(output.buf, output.Size))
//				return false;

//			(*pSize) += output.Size;

//			free(output.buf);

			pOutput = AddAlloc(pTex, output.Size);
			memcpy(pOutput, output.buf, output.Size);

			w >>= 1;
			h >>= 1;
		}

		++iter;
	}

	return true;
}

NV_ERROR_CODE CResourceObjectNVidia::WriteCallback(const void *buffer, size_t count, const MIPMapData *mipMapData, void *userData)
{
	if (mipMapData == NULL)	// We only want to add actual image data, not header info etc
		return NV_OK;

	CONV_BUFFER *pBuf = (CONV_BUFFER*) userData;

	if (pBuf->Size)
		pBuf->buf = (U8*) realloc(pBuf->buf, pBuf->Size + count);
	else
		pBuf->buf = (U8*) malloc(count);

	if (pBuf->buf == NULL)
		return NV_OUT_OF_MEMORY;

	memcpy(pBuf->buf+pBuf->Size, buffer, count);
	pBuf->Size += (U32) count;

	return NV_OK;
}

#endif



#if defined(ENABLE_ATI)

CResourceObjectATI::CResourceObjectATI()
{
}

bool CResourceObjectATI::SaveTexture(C_OutputTexture *pTex, bool bigEndian)
{
	// ****

	C_Image img;

	U8 *data = (U8*) malloc(2 * 2 * 4);
	img.AddFrame();
	img.AddLayer(2, 2, 1);
	img.AddMipLevel();
	img.NewPixelData(2, 2, 1, E_PixelFormat_R8G8B8A8U, data);

	// ****




	U8 *pOutput = AddAlloc(pTex, sizeof(ResourceHeader));
	if (!pOutput)
		return false;

	GetOutputHeader((ResourceHeader*) pOutput);

	if (bigEndian)
		XGEndianSwapMemory(pOutput, pOutput, XGENDIAN_8IN32, sizeof(DWORD), sizeof(ResourceHeader) / sizeof(DWORD));

//	pBundler->WriteHeader(&header, sizeof(header));
//	(*pSize) += sizeof(header);

	C_Status status;
	int w, h;

	UtList<C_Image*>::Iterator iter = m_Images.Begin();

	while (iter != m_Images.End())
	{
		w = (*iter)->GetWidth();
		h = (*iter)->GetHeight();

		status = (*iter)->ChangePixelFormat(E_PixelFormat_A32R32G32B32F);
		if (!status)
			return false;

		for (U32 level=0; level<m_MipMapLevels; level++)
		{
			(*iter)->Resize(w, h, (*iter)->GetDepth(), E_ResizeFilter_ScaleBox);

			ATI_TC_Texture srcTex, dstTex;

			srcTex.dwSize		= sizeof(srcTex);
			srcTex.format		= ATI_TC_FORMAT_ARGB_32F;
			srcTex.dwWidth		= w;
			srcTex.dwHeight		= h;
			srcTex.dwPitch		= w * (*iter)->GetBytesPerPixel();
			srcTex.pData		= (*iter)->GetPixelDataPtr();
			srcTex.dwDataSize	= w*h * sizeof(F32)*4;

			U32 dataSize = (*iter)->GetPixelDataSize();
			assert(dataSize == srcTex.dwDataSize);

			dstTex.dwSize		= sizeof(dstTex);
			dstTex.format		= (ATI_TC_FORMAT) m_Format->Format;
			dstTex.dwWidth		= w;
			dstTex.dwHeight		= h;
			dstTex.dwPitch		= 0;
			dstTex.dwDataSize	= ATI_TC_CalculateBufferSize(&dstTex);
			dstTex.pData		= AddAlloc(pTex, dstTex.dwDataSize);

			ATI_TC_ERROR err = ATI_TC_ConvertTexture(
				&srcTex,			// [in]  - Pointer to the source texture */
				&dstTex,			// [out] - Pointer to the destination texture */
				NULL,				// [in]  - Pointer to the compression options - can be NULL */
				NULL,				// [in]  - Pointer to the feedback proc - can be NULL */
				0,					// [in]  - User data to pass to the feedback proc */
				0);					// [in]  - User data to pass to the feedback proc */

//			if (!pBundler->WriteData(dstTex.pData, dstTex.dwDataSize))
//				return false;

//			(*pSize) += dstTex.dwDataSize;

//			delete [] dstTex.pData;
//			m_pOutput = dstTex.pData;

			w>>=1;
			h>>=1;
		}

		++iter;
	}

	return true;
}

#endif



CResourceObject *GetTextureObject(TextureFormat *pFormat)
{
	switch(pFormat->lib)
	{
#if defined(ENABLE_XDK)
	case E_CompressionLibrary_X360:
		return new CTexture360;
#endif

#if defined(ENABLE_NVIDIA)
	case ECL_NVidia:
		return new CTextureNVidia;
#endif

#if defined(ENABLE_ATI)
	case ECL_ATI:
		return new CTextureATI;
#endif

	default:
		return NULL;
	};
}

CResourceObject *GetArrayTextureObject(TextureFormat *pFormat)
{
	switch(pFormat->lib)
	{
#if defined(ENABLE_XDK)
	case E_CompressionLibrary_X360:
		return new CArrayTexture360;
#endif

#if defined(ENABLE_NVIDIA)
	case ECL_NVidia:
		return new CArrayTextureNVidia;
#endif

#if defined(ENABLE_ATI)
	case ECL_ATI:
		return new CArrayTextureATI;
#endif

	default:
		return NULL;
	};
}

CResourceObject *GetVolumeTextureObject(TextureFormat *pFormat)
{
	switch(pFormat->lib)
	{
#if defined(ENABLE_XDK)
	case E_CompressionLibrary_X360:
		return new CVolumeTexture360;
#endif

#if defined(ENABLE_NVIDIA)
	case ECL_NVidia:
		return new CVolumeTextureNVidia;
#endif

#if defined(ENABLE_ATI)
	case ECL_ATI:
		return new CVolumeTextureATI;
#endif

	default:
		return NULL;
	};
}

CResourceObject *GetCubeMapObject(TextureFormat *pFormat)
{
	switch(pFormat->lib)
	{
#if defined(ENABLE_XDK)
	case E_CompressionLibrary_X360:
		return new CCubeMap360;
#endif

#if defined(ENABLE_NVIDIA)
	case ECL_NVidia:
		return new CCubeMapNVidia;
#endif

#if defined(ENABLE_ATI)
	case ECL_ATI:
		return new CCubeMapATI;
#endif

	default:
		return NULL;
	};
}
