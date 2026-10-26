#ifndef CLOTHRENDERER_H
#define CLOTHRENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Dynamic2/Cloth/Renderer.h"

namespace DynamicEffect2
{

class C_SampleClothRenderer;

struct ClothInstance : Instance<C_Cloth>
{
	ClothInstance() {}
	~ClothInstance() {}

	MathsStorage::Float4		vColour;
	Display::C_Texture2D*	pDiffuse;
	Display::C_Texture2D*	pNormalMap;
};

class C_SampleClothRenderer : public C_ClothRenderer<C_SampleClothRenderer, ClothInstance>
{
public:
	C_SampleClothRenderer(C_Renderer* pRenderer, const U32 iIndex);
	virtual ~C_SampleClothRenderer() {};

	void						SetMaterial(const ClothInstance* pInstance) const;

	bool						Create();
	void						Destroy();
private:
	Display::C_PixelShaderParameter	m_ShaderSettings;
	Display::C_PixelShaderParameter	m_ShaderColour;
};

}

#endif // RENDER_DYNAMICMESH_MESHINTERFACE_H
