#ifndef DISPLAY_OCCLUSION_OCCLUSION_QUERY_H
#define DISPLAY_OCCLUSION_OCCLUSION_QUERY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PCD3D11) // [Blur1PCDX11] Native query.
#include "Display/Occlusion/D3D11/D3D11OcclusionQuery.h"
#elif defined(DEVICE_PS3GCM)
#include "Display/Occlusion/GCM/GCMOcclusionQuery.h"
#elif defined(DEVICE_PS3GL)
#include "Display/Occlusion/GL/GLOcclusionQuery.h"
#elif defined(DEVICE_PCGL)
#include "Display/Occlusion/GL/GLOcclusionQuery.h"
#endif

#include "Display/Device/Device.h"

namespace Display
{

//! C_OcclusionQueryRender

class C_OcclusionQueryRender : public I_Renderer
{
public:

	C_OcclusionQueryRender();
	~C_OcclusionQueryRender();

	bool OnCreate();
	void OnDestroy();

	void Bind(const bool show = false);
	void Render(const Maths::HMat43& matrix, const Maths::HVec3& scale);
	void Unbind();

private:

	bool Build();
	
public:

	C_VertexDeclaration*	m_pVertexDeclaration;
    C_VertexBuffer			m_VertexBuffer;            
	C_IndexBuffer			m_IndexBuffer;    
	C_VertexShader			m_VertexShader;
	C_PixelShader			m_PixelShader;

private:
	
	struct C_Vertex
	{
		F32 x, y, z;
	};
};

extern C_OcclusionQueryRender* g_pOcclusionQueryRender;

void OcclusionQueryRender(C_OcclusionQueryRender* pOcclusionQueryRender);

INLINE C_OcclusionQueryRender* OcclusionQueryRender()
{
	assert(g_pOcclusionQueryRender);
	return g_pOcclusionQueryRender;
}

INLINE const bool OcclusionQueryRenderExists()
{
	return g_pOcclusionQueryRender != NULL;
}

}; // namespace Display

#include "Display/Occlusion/OcclusionQuery.inl"

#endif // DISPLAY_OCCLUSION_OCCLUSION_QUERY_H
