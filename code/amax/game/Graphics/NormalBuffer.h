#ifndef AMAX_GRAPHICS_NORMAL_BUFFER_H
#define AMAX_GRAPHICS_NORMAL_BUFFER_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/Texture/Depth.h>

class C_NormalBuffer : public Display::I_Renderer
{
public:

	C_NormalBuffer();
	~C_NormalBuffer();

	virtual bool OnCreate();
	virtual bool OnCreateVolatile();
	virtual void OnDestroy();
	virtual void OnDestroyVolatile();

	void Render();

	const Display::C_Texture2D* GetTexture() const { return &m_RenderTexture.GetTexture(); }

private:

	Display::C_RenderTexture2D	m_RenderTexture;

	Render::Texture::C_Depth	m_DepthTextureUtil;

};

#endif // AMAX_GRAPHICS_NORMAL_BUFFER_H
