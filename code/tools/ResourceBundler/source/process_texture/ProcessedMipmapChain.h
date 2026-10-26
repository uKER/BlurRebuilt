#pragma once

namespace ResourceBundler
{
	///\brief Processed mipmap chain.
	///
	class C_ProcessedMipmapChain
	{
	public:
		C_ProcessedMipmapChain(
			U32 width,
			U32 height,
			U32 depth = 1);
		
		~C_ProcessedMipmapChain();

		U32 AddLevelPtr(
			U8* pData,
			Display::ResourceBundler::E_TextureFormat format,
			bool isBigEndian);

		U32 GetNumLevels() const;

		U8* GetLevelPtr(U32 mipLevel);

		const U8* GetLevelPtr(U32 mipLevel) const;

		U32 GetWidth(U32 level = 0) const;

		U32 GetHeight(U32 level = 0) const;

		U32 GetDepth(U32 level = 0) const;

		U32 GetBaseSize() const;

		U32 GetMipSize() const;

		U32 GetLevelSize(U32 level) const;

		Display::ResourceBundler::E_TextureFormat GetFormat() const;

		bool IsBigEndian() const;

	private:
		//Deny use of copy constructor.
		C_ProcessedMipmapChain(const C_ProcessedMipmapChain& rhs);

		//Deny use of asignment operator.
		const C_ProcessedMipmapChain operator=(const C_ProcessedMipmapChain& rhs);

		bool			m_isBigEndian;
		U32				m_width;
		U32				m_height;
		U32				m_depth;
		Display::ResourceBundler::E_TextureFormat m_format;
		Ut::UtVector<Ut::DynamicVector<U8*> > m_mipLevels;
	};
}
