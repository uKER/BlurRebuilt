#ifndef RENDER_RENDER_CUBE_TEXTURE_SAVER_H
#define RENDER_RENDER_CUBE_TEXTURE_SAVER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Render/CubeTextureInterface.h"

namespace Render
{

class C_CubeTextureSaver : public I_CubeTexture, public Display::I_Renderer
{
public:

	C_CubeTextureSaver(const U32 width);
	~C_CubeTextureSaver();
	
	bool OnCreateVolatile();
	void OnDestroyVolatile();

	void Bind(const Display::E_CubeTextureFace face);
	void Unbind(const Display::E_CubeTextureFace face);

	void DebugRender();

	bool Save(const Util::C_String& fileName);
	bool SaveFace(const Display::E_CubeTextureFace face, const Util::C_String& fileName);


private:

	Display::C_RenderTexture2D		m_RenderTexture[Display::E_CubeTextureFace_Count];
	Display::C_Surface				m_DepthStencilSurface;
	U32								m_Width;
};

} // namespace Render

#endif // RENDER_RENDER_CUBE_TEXTURE_SAVER_H
