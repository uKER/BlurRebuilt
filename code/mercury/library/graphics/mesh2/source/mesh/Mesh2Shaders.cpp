#include "stdafx.h"
#include "Mesh2/Mesh/Mesh2Shaders.h"
#include <Display2/Utils/Utils.h>

namespace Render
{

C_Mesh2Shaders::C_Mesh2Shaders()
{
	m_pDevice = NULL;
}

void C_Mesh2Shaders::SetDevice(Display2::C_Device* pDevice)
{
	BIZ_ASSERT(pDevice);
	m_pDevice = pDevice;
}

Display2::C_VertexShader* C_Mesh2Shaders::CreateVertexShader(const Util::C_String& filePath)
{
	assert(m_pDevice);

	Display2::C_VertexShader* pVertexShader = new Display2::C_VertexShader();

	if(!Display2::CreateVertexShaderFromFile(filePath, 0, pVertexShader))
	{
		SAFE_DELETE_PTR(pVertexShader);
		return NULL;
	}
	
	m_VertexShaderList.PushBack(pVertexShader);

	return pVertexShader;
}

Display2::C_PixelShader* C_Mesh2Shaders::CreatePixelShader(const Util::C_String& filePath)
{
	assert(m_pDevice);

	Display2::C_PixelShader* pPixelShader = new Display2::C_PixelShader();

	if(!Display2::CreatePixelShaderFromFile(filePath, 0, pPixelShader))
	{
		SAFE_DELETE_PTR(pPixelShader);
		return NULL;
	}
	
	m_PixelShaderList.PushBack(pPixelShader);

	return pPixelShader;
}

void C_Mesh2Shaders::Destroy()
{
	assert(m_pDevice);

	for(S32 i=0; i<m_VertexShaderList.Size(); i++)
	{
		m_pDevice->Destroy(m_VertexShaderList[i]);
		SAFE_DELETE_PTR(m_VertexShaderList[i]);
	}
	m_VertexShaderList.Clear();

	for(S32 i=0; i<m_PixelShaderList.Size(); i++)
	{
		m_pDevice->Destroy(m_PixelShaderList[i]);
		SAFE_DELETE_PTR(m_PixelShaderList[i]);
	}
	m_PixelShaderList.Clear();
}

} // namespace Render
