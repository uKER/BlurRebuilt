#include "StdAfx.h"
#include "gravity/narrowphase/LineCollisionCache.h"

namespace Collision
{
	//Debug::Bool g_UseCollisionCache("Test.CollisionCache", true);
#ifndef PLATFORM_SPU
#ifndef CFG_CONVERTER
	void C_LineCollisionCache::DebugRender(DbUtil::I_DebugRender* pRender) const
	{
		//Draw with lines only
		Maths::RGBA colour(Full()? 255 : 0, Full() ? 0 : 255, 0 );
		for(U32 a = m_Low; a < m_High; a ++)
		{
			C_CachedTriangle& tri = m_pTriangles[a];
			pRender->Draw3dLine(tri.m_VertexA, tri.m_VertexB, colour);
			pRender->Draw3dLine(tri.m_VertexB, tri.m_VertexC, colour);
			pRender->Draw3dLine(tri.m_VertexA, tri.m_VertexC, colour);
		}
	}
#endif
#endif
}

#include "gravity/narrowphase/LineCollisionCache_Trace.inl"

