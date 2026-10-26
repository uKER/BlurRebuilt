#include "stdafx.h"
#include "racingphysics/collisions/infiniteplane.h"

#include "display/render/basicrender.h"
#include "racingphysics/collisions/collisioncache.h"
#include "racingphysics/physics/physicstimer.h"

namespace RacingPhysics
{

InfinitePlaneSystem InfinitePlaneSystem::s_instance;

//#pragma optimize("", off)

//////////////////////////////////////////

void InfinitePlane::DisplayDebug()
{
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetMaxBeginVerticesUsage(100000);
	Display::BasicRender()->BindColoured3D();

	Point3R p = MakePoint(ConvertHVec(Display::Device()->GetView(0)->GetViewSettings3D().GetCameraPosition()));
	p.X(floor(p.X() / 10.f) * 10.f);
	p.Z(floor(p.Z() / 10.f) * 10.f);
	p.Y(GetHeight(p));

	for (int i = -10; i < 10; i++)
	{
		Point3R p1 = p;		p1.X(p1.X() + -100.f);			p1.Z(p1.Z() + (10.f * i));		p1.Y(GetHeight(p1));
		Point3R p2 = p;		p2.X(p2.X() + 100.f);			p2.Z(p2.Z() + (10.f * i));		p2.Y(GetHeight(p2));
		Point3R p3 = p;		p3.X(p3.X() + (10.f * i));		p3.Z(p3.Z() + -100.f);			p3.Y(GetHeight(p3));
		Point3R p4 = p;		p4.X(p4.X() + (10.f * i));		p4.Z(p4.Z() + 100.f);			p4.Y(GetHeight(p4));

		Display::BasicRender()->DrawColoured3DLine(p1.X(), p1.Y(), p1.Z(), p2.X(), p2.Y(), p2.Z(), Maths::RGBA(0, 0, 255));
		Display::BasicRender()->DrawColoured3DLine(p3.X(), p3.Y(), p3.Z(), p4.X(), p4.Y(), p4.Z(), Maths::RGBA(0, 0, 255));
	}
}

void InfinitePlane::BuildCache(CollisionCache* cache, const Point3R& min, const Point3R& max)
{
	RP_ASSERT(cache);
	cache->Clear();

	cache->SetBoundingBoxMinMax(min, max);

	Point3R v0(min.X(), 0.f, min.Z()); v0.Y(GetHeight(v0));
	Point3R v1(max.X(), 0.f, min.Z()); v1.Y(GetHeight(v1));
	Point3R v2(max.X(), 0.f, max.Z()); v2.Y(GetHeight(v2));
	Point3R v3(min.X(), 0.f, max.Z()); v3.Y(GetHeight(v3));
	
	cache->AddTriangle(v0, v1, v2, 
						0, 1, 2,
						1, 1, 1, 1, 1, 1, 0);

	cache->AddTriangle(v0, v2, v3, 
						0, 2, 3,
						1, 1, 1, 1, 1, 1, 0);

	cache->Finish();
}

///////////////////////////////////////////////////////////////////////	

float InfinitePlane::GetHeight(const Point3R& p) const
{
	Point3R p2 = m_position;// + Constants<Normal3R>::Y() * Maths::Sin(PhysicsTimer::Instance().PhysicsGetTimer()) * 4.f;

	return ((MakeVector(p) | ComponentMultiply(m_normal, Maths::Constants<Vector3R>::XZ()))
				+ (MakeVector(p2) | m_normal)) / m_normal.Y();
}

}
