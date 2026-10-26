#pragma once

namespace ResourceBundler
{
	class C_OutputTextureData
	{
	public:
		C_OutputTextureData();

		~C_OutputTextureData();

		const U32 GetDataSize() const;

		const U8* GetDataPtr() const;

		const U32 GetMipSize(S32 level=0, S32 image=0) const;

		const U8* GetMipPtr(S32 level=0, S32 image=0) const;

		bool AllocateStorage(
			S32 numImages, 
			S32 numLevels);

		U8* ReallocateStorage(U32 newSize);

		U8* IncreaseStorage(U32 size);

		bool SetMipInfo(
			S32 image, 
			S32 level, 
			U8 *pPtr, 
			U32 size);

	private:
		U32		m_Size;
		U8*		m_pBuf;

		S32		m_NumImages;
		S32		m_numMipLevels;

		U8**	m_pBufPtr;
		U32*	m_BufSize;
	};
}
