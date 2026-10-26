#ifndef SAMPLES_SECTIONRENDERTEXTURECUBE_H
#define SAMPLES_SECTIONRENDERTEXTURECUBE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Render/CubeTextureRender.h"

namespace Samples
{

class C_SectionRenderTextureCube : public C_Section
{
public:

	C_SectionRenderTextureCube(const Util::C_String& name);
	~C_SectionRenderTextureCube();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome				m_SkyDome;
	Render::Mesh::C_Mesh				m_Mesh;
	Render::C_CubeTextureRender			m_CubeTextureRender;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONRENDERTEXTURECUBE_H
