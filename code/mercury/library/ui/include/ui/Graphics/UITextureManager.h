#ifndef UITEXTUREMANAGER_H
#define UITEXTUREMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/UILua.h"
#include "ui/Graphics/UISpriteTextureParam.h"

namespace Display
{
	namespace ResourceBundler
	{
		class I_ResourceBundle;
	}
}

namespace UI
{

// Types
typedef const Display::C_Texture*					UITextureHandle;

class C_TextureManager
{
public:
	typedef S8											C_ResourceIndex;
	typedef Display::ResourceBundler::I_ResourceBundle	C_Resource;

	static const C_ResourceIndex						m_InvalidResourceIndex = -1;

public:
	C_TextureManager();
	~C_TextureManager();

	void					LuaState( lua_State* pState );
	void					Update();
	void					DestroyAll();

	const C_ResourceIndex	AddResourceBundle( const Sys::C_FilePath& fileName );
	const bool				RemoveResourceBundle( const C_ResourceIndex index );

	// Access
	const char*				GetTextureName( const C_TextureHandleIndex index );

	C_TextureHandleIndex	GetTextureByName( const char *name );

	UITextureHandle			GetTexturePtr( const C_TextureHandleIndex index );

	UITextureHandle			GetTexturePtrByName( const char *name);

	// Total textures combined
	const U32				TextureCount() const;

	// By index count 
	const char*				GetTextureNameByIndex( const U32 index );

	C_TextureHandleIndex	GetTextureByIndex( const U32 index );

	UITextureHandle			GetTexturePtrByIndex( const U32 index );
	
private:

private:
	C_Resource*											GetResource( const C_TextureHandleIndex index );
	C_Resource*											GetResourceByIndex( const U32 index, S32& resource, S32& texture );
	Display::ResourceBundler::I_ResourceBundle*			LoadBundle( const Sys::C_FilePath& fileName );
	const U32											CountTextures();
	const U32											ResourceIndex( const C_TextureHandleIndex index );
	const U32											TextureIndex( const C_TextureHandleIndex index );
	C_TextureHandleIndex								TextureHandleIndex( S32 resource, S32 texture );

private:

	U32								m_TextureCount;
	C_TextureHandleIndex			m_NullTextureIndex;
	UITextureHandle					m_NullTextureHandle;
	lua_State*						m_pState;
	bool							m_UpdateLuaEnums;

	Btl::Vector< C_Resource* >		m_Resources;
	MT::Lock						m_CriticalSecion;
};

// Singleton
extern C_TextureManager gUITextureManager;

}; // namespace UI
#endif // UITEXTUREMANAGER_H
