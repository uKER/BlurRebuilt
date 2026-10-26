#ifndef SAMPLE_DEFAULT_H
#define SAMPLE_DEFAULT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Sample.h"

#include <Mesh2/Mesh/Mesh2.h>
#include <Mesh2/Mesh/Mesh2Shaders.h>

class C_SampleDefault : public C_Sample
{
public:

	C_SampleDefault(C_Application* pApplication);
	~C_SampleDefault();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate();
	void OnRender();

private:

	Render::C_Mesh2Shaders				m_MeshShaders;
	Util::C_String						m_MeshFileRoot;
	Util::C_String						m_MeshFilePath;
	Render::C_Mesh2						m_Mesh;

	Display2::C_VertexShader*			m_pVertexShader;
	Display2::C_VertexShaderParam 		m_hWorldMatrix;
	Display2::C_VertexShaderParam 		m_hViewProjMatrix;
	Display2::C_PixelShader*			m_pPixelShader;
	
	Display::C_Texture					m_Texture;
};

#endif // SAMPLE_DEFAULT_H
