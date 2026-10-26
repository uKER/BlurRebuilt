// Postprocess.h
#ifndef UIPOSTPROCESS_H
#define UIPOSTPROCESS_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_PS3

#include "ui/Graphics/UITextureManager.h"

namespace UI
{

class C_PostProcess
{
public:
	C_PostProcess() :
#if defined ( _GENESIS_ )
		m_DisplayDevicePtr(NULL),
		m_RenderTargetPtr(NULL),
		m_ResManPtr(NULL),
#endif
		m_Active(false)
	{
	}

#if defined ( _GENESIS_ )
	void Init( Display::C_Device* displayDevicePtr, Resource::C_Manager* );
#endif
	void Exit();

	void Create(const S32 layer);
	void Destroy();
	const bool Update();
	void Update(const bool update)	{ m_DoUpdate = update; }
	const S32 Layer()				{ return m_Layer; }
	

#if defined ( _GENESIS_ )
	UITextureHandle GetTexture( )
	{
		Resource::C_Texture* texturePtr;
		m_RenderTargetPtr->GetResource(texturePtr);
		return texturePtr;
	}
#endif

private:
	bool							m_Active;
	bool							m_DoUpdate;
	S32								m_Layer;
#if defined ( _GENESIS_ )
	Display::C_Device*		m_DisplayDevicePtr;
	Resource::C_Manager*			m_ResManPtr;
	Resource::C_HandlePtr			m_RenderTargetPtr;
#endif

};

extern C_PostProcess gPostProcess;

}; // namespace UI

#endif // UIPOSTPROCESS_H

#endif
