#include "Stdafx.h"

#include "CustomTextureManager.h"
#include <ui/Graphics/UITextureManager.h>

namespace UI
{

class C_CustomAlternateTexture : public C_Simple2D::I_AlternateTexture
{
public:
	C_CustomAlternateTexture( const Util::C_String32& name ) : m_Name( name ), m_Texture( NULL ) { }

	UITextureHandle					Handle()	{ return m_Texture; }
	const Util::C_String32&			Name()		{ return m_Name; }

private:
	UITextureHandle					m_Texture;
	Util::C_String32				m_Name;
};

void C_CustomTextureManager::Init()
{
	Util::C_String32 name;
	for( U32 i = 0; i < m_MaxLiveries; i++ )
	{
		name.Printf( "CT_LIVERY%02d", i );
		m_Liverys[ i ].m_pAlternateTexture = new C_CustomAlternateTexture( name );
		BIZ_ASSERT( m_Liverys[ i ].m_pAlternateTexture );

		gSimple2D.SetAlternateTexture( m_Liverys[ i ].m_pAlternateTexture );
	}

	for( U32 i = 0; i < m_MaxGroups; i++ )
	{
		name.Printf( "CT_GROUP%02d", i );
		m_Groups[ i ].m_pAlternateTexture = new C_CustomAlternateTexture( name );
		BIZ_ASSERT( m_Groups[ i ].m_pAlternateTexture );

		gSimple2D.SetAlternateTexture( m_Groups[ i ].m_pAlternateTexture );
	}

	for( U32 i = 0; i < m_MaxAvatars; i++ )
	{
		name.Printf( "CT_AVATAR%02d", i );
		m_Avatars[ i ].m_pAlternateTexture = new C_CustomAlternateTexture( name );
		BIZ_ASSERT( m_Avatars[ i ].m_pAlternateTexture );

		gSimple2D.SetAlternateTexture( m_Avatars[ i ].m_pAlternateTexture );
	}

	m_DefaultLivery	= gUITextureManager.GetTexturePtrByName( "null" );
	m_DefaultGroup	= gUITextureManager.GetTexturePtrByName( "null" );
	m_DefaultAvatar	= gUITextureManager.GetTexturePtrByName( "null" );
}

void C_CustomTextureManager::Destroy()
{
	// Clean up
	for( U32 i = 0; i < m_MaxLiveries; i++ )
		SAFE_DELETE_PTR( m_Liverys[ i ].m_pAlternateTexture );

	for( U32 i = 0; i < m_MaxGroups; i++ )
		SAFE_DELETE_PTR( m_Groups[ i ].m_pAlternateTexture );

	for( U32 i = 0; i < m_MaxAvatars; i++ )
		SAFE_DELETE_PTR( m_Avatars[ i ].m_pAlternateTexture );
}

UITextureHandle C_CustomTextureManager::LiveryTexture( const U32 index )
{
	if( m_Liverys[ index ].m_pAlternateTexture->Handle() )
		return m_Liverys[ index ].m_pAlternateTexture->Handle();
		
	return m_DefaultLivery;

}

UITextureHandle	C_CustomTextureManager::GroupTexture( const U32 index )
{
	if( m_Groups[ index ].m_pAlternateTexture->Handle() )
		return m_Groups[ index ].m_pAlternateTexture->Handle();

	return m_DefaultGroup;
}

UITextureHandle	C_CustomTextureManager::AvatarTexture( const U32 index )
{
	if( m_Avatars[ index ].m_pAlternateTexture->Handle() )
		return m_Avatars[ index ].m_pAlternateTexture->Handle();

	return m_DefaultAvatar;
}

};