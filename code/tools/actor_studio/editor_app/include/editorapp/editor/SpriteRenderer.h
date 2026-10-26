#ifndef EDITOR_SPRITE_RENDERER_H
#define EDITOR_SPRITE_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "common/ResourcePtr.h"

namespace Editor
{

enum E_SpriteEffect
{
	E_SpriteEffect_None,
	E_SpriteEffect_Greyscale,
};

struct C_SpriteInstance
{
	E_SpriteEffect m_Effect;
	AftkRender::C_ResourcePtr<Display::C_RefTexture2D> m_SpriteTexture;
	Maths::HVec3 m_pivot;
	float m_SpriteSize;
	EditorLib::C_NodeId m_NodeId;
	U32 m_ComponentId;

	static bool TextureSort(const C_SpriteInstance& l, const C_SpriteInstance& r);
	static bool DepthSortBackToFront(const C_SpriteInstance& l, const C_SpriteInstance& r);
	static bool DepthSortFrontToBack(const C_SpriteInstance& l, const C_SpriteInstance& r);
};


class C_SpriteRenderer : public I_EditorRenderer
{
public:
	C_SpriteRenderer();
	~C_SpriteRenderer();
	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroy();
	void OnDestroyVolatile();
	void Clear();

	AftkRender::C_ResourcePtr<Display::C_RefTexture2D> GetSprite(const Btl::String& filename);

	virtual void Update(AftkRender::E_RendererPass pass = AftkRender::E_RendererPass_Normal);

	virtual void GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection);

	virtual void Render();

	void Draw(
		const Btl::String& spriteFile,
		E_SpriteEffect effect,
		float spriteSize, 
		const Maths::HMat44* m_pXform, 
		const EditorLib::C_NodeId& id,
		U32 componentId);

private:
	bool CreateVertexBuffer();
	void PopulateVertexBuffer();

	Display::C_VertexBuffer									*m_pVertexBuffer;
	Ut::UtVector< Ut::DynamicVector<C_SpriteInstance> >		m_SpriteInstances;
	U32														m_MaxInstanceCount;
};

}

#endif