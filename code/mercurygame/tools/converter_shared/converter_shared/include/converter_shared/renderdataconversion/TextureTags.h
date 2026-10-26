#ifndef __TEXTURE_TAGS_H__
#define __TEXTURE_TAGS_H__

namespace RenderDataConversion
{

enum E_TextureTagAddressMode
{
	E_TextureTagAddressMode_Wrap = 0,
	E_TextureTagAddressMode_Mirror,
	E_TextureTagAddressMode_Clamp,
};

enum E_TextureTagFilter
{	
	E_TextureTagFilter_Linear = 0,
	E_TextureTagFilter_Anisotropic,
	E_TextureTagFilter_Point,
};

enum E_TextureTagAlphaType
{
	E_TextureTagAlphaType_AlphaDither = 0,
	E_TextureTagAlphaType_Punchthrough,
	E_TextureTagAlphaType_AlphaBlend,
	E_TextureTagAlphaType_Decal,
};

// C_TextureTags
class C_TextureTags
{
public:
	C_TextureTags();
	~C_TextureTags();

public:
	bool m_Exclude;
	bool m_Shared;
	bool m_OverrideIprReduce;
	
	E_TextureTagAddressMode m_AddressModeU;
	E_TextureTagAddressMode m_AddressModeV;

	E_TextureTagFilter m_FilteringType;

	S8 m_MipmapBias;
	U8 m_MaxAnisotropy;

	E_TextureTagAlphaType m_AlphaType;
	U8 m_AlphaRef;
	U8 m_DecalOrder;

	Util::C_String m_Override_IPR0;
	Util::C_String m_Override_NEON;
	Util::C_String m_Override_BUMP_NORM;

private:
	//Disallow shallow copies.
	C_TextureTags(const C_TextureTags& rhs);
	const C_TextureTags& operator=(const C_TextureTags& rhs);
};

// I_TextureTagsReader
class I_TextureTagsReader
{
public:
	I_TextureTagsReader();
	virtual ~I_TextureTagsReader();

	virtual void ReadTextureTags( const Util::C_String& textureFilename, C_TextureTags& result ) = 0;

};

// C_DefaultTextureTagsReader, default implementation of I_TextureTagsReader
class C_DefaultTextureTagsReader : public I_TextureTagsReader
{
public:
	C_DefaultTextureTagsReader();
	~C_DefaultTextureTagsReader();

	void ReadTextureTags( const Util::C_String& textureFilename, C_TextureTags& result );
};

}

#endif
