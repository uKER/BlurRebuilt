#ifndef ATTK_MIPMAP_PIXEL_DATA_CHAIN_H
#define ATTK_MIPMAP_PIXEL_DATA_CHAIN_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	///\brief Mipmap chain.
	///
	class C_MipmapPixelDataChain
	{
	public:
		C_MipmapPixelDataChain();
		
		~C_MipmapPixelDataChain();

		void SetData(
			U32 width,
			U32 height,
			U32 depth,
			U32 numLevels,
			E_PixelFormat format,
			U8* pPixelData);

		void SetPixelDataPtr(
			U8* pPixelData);

		U32 GetNumLevels() const;

		U8* GetPixelDataPtr();

		const U8* GetPixelDataPtr() const;

		U32 GetPixelDataSize() const;

		U32 GetWidth(U32 mipLevel) const;

		U32 GetHeight(U32 mipLevel) const;

		U32 GetDepth(U32 mipLevel) const;

		E_PixelFormat GetPixelDataFormat() const;

		void Clear();

	private:
		//Deny use of copy constructor.
		C_MipmapPixelDataChain(const C_MipmapPixelDataChain& rhs);

		//Deny use of asignment operator.
		const C_MipmapPixelDataChain operator=(const C_MipmapPixelDataChain& rhs);

		U32 m_width;
		U32 m_height;
		U32 m_depth;
		E_PixelFormat m_format;
		U32 m_numLevels;
		U8* m_pPixelDataChain;
	};

	typedef C_MipmapPixelDataChain C_MipmapCubeMapPixelDataChain[6];
}

#endif
