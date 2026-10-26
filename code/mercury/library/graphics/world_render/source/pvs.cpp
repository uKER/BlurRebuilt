#include "stdafx.h"
#include "world_render/pvs.h"
#include "System/Debug/I_DebugRender.h"
#include <core/maths\hvec\New\IncludeMinimal.h>
#include <core/maths\hvec\New\IncludeMinimal.inl>

Debug::Bool g_DebugPVSRender("Graphics.WorldRender.PVS.Render", false);
Debug::Int g_DebugPVSForceCell("Graphics.WorldRender.PVS.ForceCell", -1);
Debug::Bool g_DebugPVSLoadSamplePoints("Graphics.WorldRender.PVS.LoadSamplePoints", false);
Debug::Bool g_DebugPVSNSamplePoints("Graphics.WorldRender.PVS.ShowNumberOfSamplePoints", false);
Debug::Bool g_DebugPVSShowCellIndex("Graphics.WorldRender.PVS.ShowCellIndex", false);

//#pragma optimize("", off)

namespace Pvs {
namespace Target
{

Btl::BitVector PvsData::s_debugBits;

void PvsData::DebugRender(const Maths::HVec3& camera) const
{
	if (!g_DebugPVSRender)
		return;

	for (int i = 0; i < m_cells.Size(); i++)
	{
		if (s_debugBits.Size() < m_cells.Size() || s_debugBits[i])
		{
			const MathsStorage::Matrix44& m = m_cells[i].matrix;

			Maths::HMat43 mat43(Maths::HVec3(m.rx.x, m.rx.y, m.rx.z),
								Maths::HVec3(m.ry.x, m.ry.y, m.ry.z),
								Maths::HVec3(m.rz.x, m.rz.y, m.rz.z),
								Maths::HVec3(m.rw.x, m.rw.y, m.rw.z));

			Maths::RGBA colour = Maths::RGBA(1.f, 0.f, 0.f);

			if (i == g_DebugPVSForceCell)
				colour = Maths::RGBA(0.f, 0.f, 1.f);
			else if (g_DebugPVSForceCell != -1)
				continue;

			DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dBox(Maths::HVec3(-0.99f, -0.99f, -0.99f), Maths::HVec3(0.99f, 0.99f, 0.99f), mat43, colour);
			DbUtil::DefaultDebugRenderSys()->Immediate()->SetStates(false);

			Maths::Mat44 imat44 = Maths::Load<Maths::HMat44, MathsStorage::Matrix44>(&m_inverseMatrices[i]);
			Maths::Vector4 v = Maths::MakeVector4(camera.X(), camera.Y(), camera.Z(), 1.f);
			Maths::Vector4 v2 = v * imat44;
			Maths::VF32 one = Maths::MakeVF32(1.f);

			if (((Abs(X(v2)) < one) & (Abs(Y(v2)) < one) & (Abs(Z(v2)) < one)) || i == g_DebugPVSForceCell)
			{
				for (int j = 0; j < m_cells[i].samplePoints.Size(); j++)
				{
					Point3 p = m_cells[i].samplePoints[j].position;
					Maths::HVec3 h(p.x, p.y, p.z);

					Maths::RGBA colour;
			
					//if (m_cells[i].samplePoints[j].optional) 
						colour = Maths::RGBA(1.f, 1.f, 0.f);
					//else
					//	colour = Maths::RGBA(1.f, 0.f, 0.f);

					DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dLine(h - Maths::HVec3(0.f, 1.f, 0.f), h, colour);
				}
			}
		}
	}

	if (g_DebugPVSNSamplePoints || g_DebugPVSShowCellIndex)
	{
		for (int i = 0; i < m_cells.Size(); i++)
		{
			if (s_debugBits.Size() < m_cells.Size() || s_debugBits[i])
			{
				const MathsStorage::Matrix44& m = m_cells[i].matrix;

				Maths::HVec3 p(Maths::HVec3(m.rw.x, m.rw.y, m.rw.z));

				if (g_DebugPVSNSamplePoints)
				{
					Util::C_String text;
					text.Printf("%i", m_cells[i].samplePoints.Size());
					DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dText(p, text);
				}

				if (g_DebugPVSShowCellIndex)
				{
					Util::C_String text;
					text.Printf("%i", i);
					DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dText(p, text);
				}
			}
		}
	}

	DbUtil::DefaultDebugRenderSys()->Immediate()->SetStates(true);
}

void PvsData::SetDebugBits(const Btl::BitVector& bits)
{
	s_debugBits = bits;
}

S32 PvsData::GetCell(const Maths::HVec3& camera) const
{
	PERF_EVENT("PvsData::GetCell");

	if (g_DebugPVSForceCell > -1 && g_DebugPVSForceCell < m_cells.Size())
		return g_DebugPVSForceCell;

	Maths::Vector4 v = Maths::MakeVector4(camera.X(), camera.Y(), camera.Z(), 1.f);
	Maths::VF32 one = Maths::MakeVF32(1.f);

	for (int i = 0; i < m_cells.Size(); i++)
	{
		Maths::Mat44 imat44 = Maths::Load<Maths::HMat44, MathsStorage::Matrix44>(&m_inverseMatrices[i]);
		Maths::Vector4 v2 = v * imat44;

		if ((Abs(X(v2)) < one) & (Abs(Y(v2)) < one) & (Abs(Z(v2)) < one))
		{
			return i;
		}
	}

	return -1;
}

}}
