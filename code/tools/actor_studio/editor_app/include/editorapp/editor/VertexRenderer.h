#ifndef EDITOR_VERTEX_RENDERER_H
#define EDITOR_VERTEX_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{

class C_VertexRenderer : public I_EditorRenderer
{
public:
	C_VertexRenderer(int size);
	~C_VertexRenderer();
	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroy();
	void OnDestroyVolatile();
	void Clear();

	virtual void Update(AftkRender::E_RendererPass pass = AftkRender::E_RendererPass_Normal);

	virtual void GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection);

	virtual void Render();

	void* BeginVertices(const Display::E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, Display::C_VertexDeclaration* pVertexDeclaration, const bool checkUsage);
	void EndVertices(void* pVertex);

private:
	Display::C_DynamicVertexBuffer	m_DynamicVertexBuffer;

	U32 m_VertexBufferSize;
	U32 m_BeginVerticesSize;
	U8* m_BeginVerticesStart;
	U32 m_BeginVerticesUsage;
	U32 m_MaxBeginVerticesUsage;
	bool m_BeginVerticesLocked;
};
}

#endif