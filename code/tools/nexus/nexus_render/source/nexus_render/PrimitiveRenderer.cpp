#include <nexus_render/stdafx.h>
#include <nexus_render/Camera.h>
#include <nexus_render/TargetCamera.h>
#include <nexus_render/VBWrapper.h>
#include <nexus_render/Primitives.h>
#include <nexus_render/PrimitiveRenderer.h>

using namespace NexusRender;

PrimitiveRenderer::PrimitiveRenderer()
{
	m_dirty = true;
}

PrimitiveRenderer::~PrimitiveRenderer()
{
	Clear();
}

void PrimitiveRenderer::Clear()
{
	m_vbCopy.Clear();
	m_vbWrapper.Destroy();
}

Tu::Status PrimitiveRenderer::Rebuild()
{
	m_vbWrapper.Destroy();
	m_dirty = true;
	return Tu::Status::SUCCESS;
}

Tu::Status PrimitiveRenderer::Draw()
{
	if (m_dirty)
	{
		if (m_vbCopy.Size() == 0)
		{
			return Tu::Status::SUCCESS;
			m_dirty = false;
		}
		else
		{
			Tu::Status status;
			m_vbWrapper.Destroy();
			status = m_vbWrapper.Create(
				sizeof(D3DVertexPosCol), 
				(U32)m_vbCopy.Size(),
				NEXUS_RENDER_PRIMITIVES_LINE_VERTS);
			if (!status) return status;

			//Populate our VB wrapper.
			m_vbWrapper.Lock();
			for (U32 i = 0; i < (U32)m_vbCopy.Size(); i++)
			{
				m_vbWrapper.SetVertex(i, &(m_vbCopy[i]));
			}
			m_vbWrapper.Unlock();

			m_dirty = false;
		}
	}
	
	m_vbWrapper.Draw(D3DPT_LINELIST);
	return Tu::Status::SUCCESS;
}

void PrimitiveRenderer::DrawLine(
	const Maths::HMat44& m,
	const Maths::HVec3& a, 
	const Maths::HVec3& b,
	const Maths::HVec4& colour)
{
	m_vbCopy.Resize(m_vbCopy.Size() + NEXUS_RENDER_PRIMITIVES_LINE_VERTS);
	Primitives::DrawLine(
		&m_vbCopy[0],
		m,
		a,
		b,
		colour);
	m_dirty = true;
}

void PrimitiveRenderer::DrawStar(
	const Maths::HMat44& m,
	const F32 radius,
	const Maths::HVec4& colour)
{
	m_vbCopy.Resize(m_vbCopy.Size() + NEXUS_RENDER_PRIMITIVES_LINE_VERTS);
	Primitives::DrawStar(
		&m_vbCopy[0],
		m,
		radius,
		colour);
	m_dirty = true;
}

void PrimitiveRenderer::DrawCircle(
	const Maths::HMat44& m,
	const F32 radius,
	const Maths::HVec4& colour)
{
	m_vbCopy.Resize(m_vbCopy.Size() + NEXUS_RENDER_PRIMITIVES_LINE_VERTS);
	Primitives::DrawCircle(
		&m_vbCopy[0],
		m,
		radius,
		colour);
	m_dirty = true;
}

void PrimitiveRenderer::DrawCone(
	const Maths::HMat44& m,
	const F32 radius,
	const F32 height,
	const Maths::HVec4& colour)
{
	m_vbCopy.Resize(m_vbCopy.Size() + NEXUS_RENDER_PRIMITIVES_LINE_VERTS);
	Primitives::DrawCone(
		&m_vbCopy[0],
		m,
		radius,
		height,
		colour);
	m_dirty = true;
}

void PrimitiveRenderer::DrawBox(	
	const Maths::HMat44& m,
	const Maths::HVec3& a, 
	const Maths::HVec3& b,
	const Maths::HVec4& colour)
{
	m_vbCopy.Resize(m_vbCopy.Size() + NEXUS_RENDER_PRIMITIVES_BOX_VERTS);
	Primitives::DrawBox(
		&m_vbCopy[0],
		m,
		a,
		b,
		colour);
	m_dirty = true;
}
