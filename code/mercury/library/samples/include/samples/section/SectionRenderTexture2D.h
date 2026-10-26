#ifndef SAMPLES_SECTIONRENDERTEXTURE2D_H
#define SAMPLES_SECTIONRENDERTEXTURE2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Render/TextureRender.h"

namespace Samples
{

class C_SectionRenderTexture2D : public C_Section
{
public:

	C_SectionRenderTexture2D(const Util::C_String& name);
	~C_SectionRenderTexture2D();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome			m_SkyDome;
	Render::Mesh::C_Mesh			m_Mesh;
	Render::C_TextureRender			m_TextureRender;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONRENDERTEXTURE2D_H
