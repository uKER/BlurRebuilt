#ifndef RENDER_RENDER_CUBE_TEXTURE_INTERFACE_H
#define RENDER_RENDER_CUBE_TEXTURE_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

class I_CubeTexture
{
public:

	virtual ~I_CubeTexture() {}

	virtual void Bind(const Display::E_CubeTextureFace face) = 0;
	virtual void Unbind(const Display::E_CubeTextureFace face) = 0;
		
	virtual void DebugRender() = 0;
};

} // namespace Render

#endif // RENDER_RENDER_CUBE_TEXTURE_INTERFACE_H
