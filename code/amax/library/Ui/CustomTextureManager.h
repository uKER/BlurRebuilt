#ifndef CUSTOMTEXTUREMANAGER_H
#define CUSTOMTEXTUREMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/graphics/simple2d.h>

namespace UI
{

class C_CustomAlternateTexture;

// Storage for custom textures generated via the Vinyl editor / downloaded from LSP
class C_CustomTextureManager
{
public:
	void Init();
	void Destroy();

	UITextureHandle					LiveryTexture( const U32 index );
	UITextureHandle					GroupTexture( const U32 index );
	UITextureHandle					AvatarTexture( const U32 index );

private:
	class C_Resource
	{
	public:
		C_CustomAlternateTexture*	m_pAlternateTexture;
	};

	static const U32	m_MaxLiveries = 21;
	static const U32	m_MaxGroups = 10;
	static const U32	m_MaxAvatars = 21;

	Btl::Vector< C_Resource, Btl::VectorParameters_Static< m_MaxLiveries > >	m_Liverys;
	Btl::Vector< C_Resource, Btl::VectorParameters_Static< m_MaxGroups > >		m_Groups;
	Btl::Vector< C_Resource, Btl::VectorParameters_Static< m_MaxAvatars > >		m_Avatars;

	UITextureHandle		m_DefaultLivery;
	UITextureHandle		m_DefaultGroup;
	UITextureHandle		m_DefaultAvatar;
};

} // namespace UI

#endif // CUSTOMTEXTUREMANAGER_H
