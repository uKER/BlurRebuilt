#ifndef RENDER_MESH2_SHADERS_H
#define RENDER_MESH2_SHADERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Display2/Device/Device.h>

namespace Render
{

//! C_Mesh2Shaders

class C_Mesh2Shaders
{
public:

	C_Mesh2Shaders();

	void SetDevice(Display2::C_Device* pDevice);

	Display2::C_VertexShader* CreateVertexShader(const Util::C_String& filePath);
	Display2::C_PixelShader* CreatePixelShader(const Util::C_String& filePath);
	void Destroy();

private:

	Btl::Vector<Display2::C_VertexShader*>		m_VertexShaderList;
	Btl::Vector<Display2::C_PixelShader*>		m_PixelShaderList;
	Display2::C_Device*							m_pDevice;
};

} // namespace Render

#endif // RENDER_MESH2_SHADERS_H
