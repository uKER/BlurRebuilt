#pragma once

#if defined(ENABLE_XDK)
	struct D3DBaseTexture;
#endif

namespace CompressLib
{
	#if defined(ENABLE_XDK)

	class C_Compressor360 : public CResourceObject
	{
	public:
		C_Compressor360();

		DWORD GetXboxFormat(void);

		virtual bool SaveTexture(C_OutputTexture *pTex, bool bigEndian, bool tileTexture, bool swizzleTexture);

		virtual bool GetD3DHeader(D3DBaseTexture *pHeader, U32 *pBaseSize, U32 *pMipSize)=0;

	protected:
		bool	m_NonPackedMipTail;
		bool	m_Border;

		U32		m_BaseOffset;
		U32		m_MipOffset;
	};

	#endif



	#if defined(ENABLE_NVIDIA)

	class CResourceObjectNVidia : public CResourceObject
	{
	public:
		CResourceObjectNVidia();

		bool SaveTexture(C_OutputTexture *pTex, bool bigEndian, bool tileTexture, bool swizzleTexture);

	protected:
		static NV_ERROR_CODE CResourceObjectNVidia::WriteCallback(const void *buffer, size_t count,  const MIPMapData *mipMapData, void *userData);

	};

	#endif



	#if defined(ENABLE_ATI)

	class CResourceObjectATI : public CResourceObject
	{
	public:
		CResourceObjectATI();

		bool SaveTexture(C_OutputTexture *pTex, bool bigEndian, bool tileTexture, bool swizzleTexture);

	protected:
	};

	#endif
}