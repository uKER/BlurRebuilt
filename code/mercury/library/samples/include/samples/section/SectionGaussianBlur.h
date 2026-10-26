#ifndef SAMPLES_SECTIONGAUSSIANBLUR_H
#define SAMPLES_SECTIONGAUSSIANBLUR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Render/TextureRender.h"
#include "Render/Texture/TextureUtils.h"

namespace Samples
{

class C_SectionGaussianBlur : public C_Section
{
public:

	C_SectionGaussianBlur(const Util::C_String& name);
	~C_SectionGaussianBlur();

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

private:

	Render::Sky::C_SkyDome				m_SkyDome;
	Render::Mesh::C_Mesh				m_Mesh;
	Render::C_TextureRender				m_TextureRender;
	Render::Texture::C_TextureUtils		m_TextureUtils;
	Display::C_RenderTexture2D		m_RenderTexture;
};

}; // namespace Samples

#endif // SAMPLES_SECTIONGAUSSIANBLUR_H
