#include <stdafx.h>

#include "SectionPhysics.h"
#include "gravity/narrowphase/PrimitiveTest.h"
//#include "gravity/narrowphase/PrimitiveTest.inl"

#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>
#include "TriangleSetting.h"
#include "OBBSetting.h"
#include "gravity/narrowphase/RayTriangle.h"
#include "gravity/narrowphase/RayTriangle.inl"
#include "gravity/narrowphase/TriangleTriangle.inl"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/XenoCollide.h"
using namespace Baking;
using namespace Collision;

namespace Samples
{

	void C_SectionPhysics::TestXenoCollideBoxBox()
	{
		using namespace Maths;

		C_OBB boxa;
	

		boxa.c=HVec3(HF32(obb_cX), HF32(obb_cY), HF32(obb_cZ));
		boxa.u.SetIdentity();
		boxa.e=HVec3(HF32(obb_eX), HF32(obb_eY), HF32(obb_eZ));

		C_OBB boxb;
		boxb.c=HVec3(4.5f, -10.5f, -3.f);
		boxb.u.SetIdentity();
		boxb.e=HVec3(2.f, 3.f, 4.f);

		const F32 smll(0.02f);
		const HF32 _small (Load<HF32>(&smll));
		static HF32 rot=Maths::Constants<HF32>::Zero();
		static HF32 rotB = Maths::Constants<HF32>::Zero();
		if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
			rot=0.f;

		if(m_input.IsButtonDn(E_Button_A))
		{
			rot-=_small;
		}

		if(m_input.IsButtonDn(E_Button_B))
		{
			rot+=_small;
		}

		if(m_input.IsButtonDn(E_Button_Y))
		{
			rotB+=_small;
		}

		const Maths::RotX rotxa(rot);
		HMat33 matra(rotxa);
		boxa.u=matra;

		const Maths::RotX rotxb(rotB);
		HMat33 matrb(rotxb);
		boxb.u = matrb;
	
	
		Maths::HMat43 mata(boxa.u, boxa.c);
		Maths::HMat43 matb(boxb.u, boxb.c);


		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-boxa.e, boxa.e, mata, Maths::RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-boxb.e, boxb.e, matb, Maths::RGBA(0, 255, 0));

		Maths::HVec3 contact(Maths::UNDEFINED);
		Maths::HVec3 normal(Maths::UNDEFINED);
		Xeno xeno;
		bool collided=xeno.XenoCollide<C_OBB, C_OBB>(boxa, boxb, contact, normal);

		if(collided)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-boxa.e, boxa.e, mata, Maths::RGBA(0, 0, 255, 255));
			
		}
	
	}

};
