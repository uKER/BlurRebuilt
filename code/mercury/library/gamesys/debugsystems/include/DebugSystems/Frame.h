#ifndef FRAME_H
#define FRAME_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TextureResource/TextureResource.h>

namespace DebugSystems
{

enum E_BackgroundJustification
{
	E_BJ_TopLeft,
	E_BJ_TopRight,
	E_BJ_BottomLeft,
	E_BJ_BottomRight,
	E_BJ_Center,
	E_BJ_None
};

class C_FrameSkin
{
public:
	C_FrameSkin( ) { Reset( );}

	C_FrameSkin(const C_FrameSkin& rhs)
	{
		*this = rhs;
	}

	const C_FrameSkin& operator=(const C_FrameSkin& rhs)
	{
		m_BorderSize = rhs.m_BorderSize;
		m_Colour = rhs.m_Colour;
		m_FrameTexture = rhs.m_FrameTexture;
		m_BackgroundJustification = rhs.m_BackgroundJustification;
		m_BackgroundTexture = rhs.m_BackgroundTexture;
		m_BackgroundTextureSize = rhs.m_BackgroundTextureSize;
		m_BackgroundColour = rhs.m_BackgroundColour;
		m_ShadowOffset = rhs.m_ShadowOffset;
		m_ShadowTranslucency = rhs.m_ShadowTranslucency;
		return *this;
	}

private:
	void Reset( );

public:
	F32 m_BorderSize;
	Maths::RGBA m_Colour;
	Sys::C_FilePath	m_FrameTexture;
	E_BackgroundJustification m_BackgroundJustification;
	Sys::C_FilePath m_BackgroundTexture;
	Maths::HVec2 m_BackgroundTextureSize;
	Maths::RGBA m_BackgroundColour;
	F32 m_ShadowOffset;
	F32 m_ShadowTranslucency;
};

class C_Frame : public Resource::C_Listener
{
	TextureResource::C_TextureHandle	m_FrameTextureHandle;
	TextureResource::C_TextureHandle	m_BackgroundTextureHandle;
	volatile U32						m_resourcesPending;
	C_FrameSkin							m_frameSkin;

	static C_Frame*						m_pCommonFrame;

public:
	C_Frame(){};
	C_Frame( const C_FrameSkin& frameSkin);
	~C_Frame();

	void Draw(Maths::HVec2 PCREF rect, Maths::HVec2 PCREF size);

	static void SetCommonFrame(C_Frame* pFrame) { m_pCommonFrame = pFrame;}
	static C_Frame*	GetCommonFrame(){ return m_pCommonFrame;}

// inherited from Resource::C_Listener
public:
	virtual void ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData);
};



} // namespace DebugSystems

#endif
