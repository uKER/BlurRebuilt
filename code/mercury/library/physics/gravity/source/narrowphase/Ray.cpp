#include "StdAfx.h"
#include <btl/utility/string.h>	// [Blur2PC] Btl::String (core/containers/string.h no longer pulls it in)
#include "gravity/narrowphase/Ray.h"
#include "gravity/narrowphase/Shape.h"
#include "gravity/sharedphysics/Material.h"
#include "gravity/sdk/scene.h"

namespace Collision
{
	void C_Ray::DebugRender(DbUtil::I_DebugRender* pRender, const Maths::RGBA PCREF color)
	{
		if(m_result.Hit())
		{
			const Gravity::C_Body* pBody = m_result.GetBody();
			const Gravity::C_Scene* pScene = pBody->GetScene();
			const SharedPhysics::C_MaterialDebug* material = &pScene->m_MaterialDebug;
			pRender->Draw3dLine( m_start, m_end, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );
			pRender->Draw3dStar( m_result.HitPoint(),  0.05f, color);

			Btl::String str = Btl::String::Format("Material:%s|%s\n", material->GetEffectMaterialName( m_result.GetEffectMaterial() ), material->GetPhysicsMaterialName( m_result.GetPhysicsMaterial() ));
			pRender->Draw3dText(m_end, str.Chars());
		}
		else
		{
			pRender->Draw3dLine( m_start, m_end, Maths::RGBA(0xff, 0x00, 0xff, 0xff) );
		}

	}
} 

