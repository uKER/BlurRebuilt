#include "stdafx.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/CollisionManager.h"


namespace Collision
{

	C_CompositeShape::~C_CompositeShape()
	{
#ifndef PLATFORM_SPU
#if !CFG_CONVERTER
		C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

		//for(S32 a = 0; a < numShapes; ++a)
		while(iter.HasNextShape())
		{
			iter.NextShape();
			C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
			//pShape->~C_Shape();
			Collision::C_CollisionManager::DestructShape(pShape);
		}
		BizAlignedFree( m_pShapes );
		m_pShapes = NULL;
		m_NumShapes = 0;
#endif
#endif
	}

#ifndef PLATFORM_SPU
	void C_CompositeShape::InvalidateShape()
	{
		m_Index = -1;
		C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);
		while(iter.HasNextShape())
		{
			iter.NextShape();
			C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
			pShape->InvalidateShape();
		}
	}



	void C_CompositeShape::SetGroupFilter(C_CollisionFilter* pFilter)
	{
		/*m_Filter.m_GroupFlags = pFilter->m_GroupFlags;
		m_Filter.m_GroupMask = pFilter->m_GroupMask;
		m_Filter.m_GroupId = pFilter->m_GroupId;
		for(U32 a = 0; a < m_NumShapes; ++a)
		{
			m_pShapes[a].m_pShape->SetGroupFilter(pFilter);
		}*/
		Collision::C_CompositeShapeIterator iter(this);
		m_Filter.SetGroupFlags(pFilter->GetGroupFlags());
		m_Filter.SetGroupMask(pFilter->GetGroupMask());
		m_Filter.SetGroupId(pFilter->GetGroupId());
		while(iter.HasNextShape())
		{
			iter.NextShape();
			C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
			pShape->SetGroupFilter(pFilter);
		}
	}
#endif

	//..............................Sphere......................................
#ifndef PLATFORM_SPU
	void C_SphereShape::CalculateVolume()
	{
		using namespace Maths;
		const HF32 pi=Constants<HF32>::Pi();
		const HF32 constants(Maths::Constants<Maths::HF32>::Four()*SharedPhysics::Constants::Third());
		const HF32 r=m_Sphere.Radius();
		const HF32 r3=r*r*r;
		m_Volume = constants*pi*r3;
	}


	void C_SphereShape::DebugRender(DbUtil::I_DebugRender* pRender)const
	{
		pRender->Draw3dStar(m_Sphere.Centre(), 0.25f, Maths::RGBA(0, 255, 0));
		pRender->Draw3dSphere(m_Sphere.Centre(), m_Sphere.Radius(), Maths::RGBA(255, 0, 0));
	}
#endif

	Maths::HF32 C_SphereShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	{
		using namespace Maths;

		const HVec3 c=m_Sphere.Centre(); 
		const HF32 dist = normal|(c-p);
		const HF32 r=m_Sphere.Radius();

		const HF32 zero =Constants<HF32>::Zero();
		//The plane and sphere is not intersect yet
		if(dist.GT(r))
		{
			ratio = zero;
			return zero;
		}

		const HF32 one=Constants<HF32>::One();
		const HF32 height=r*Constants<HF32>::Two();
		const HF32 length = r-dist;

		Maths::Scalar_Bool con((length - height).GE(zero));
		ratio=Select(con, one, length/height);

		const HF32 halfLength = length*Constants<HF32>::Half();
		const HVec3 v=c - normal*r;
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(centroid, 0.25f, RGBA(0, 255, 0, 0));
		centroid = Select(con, c, v + normal*halfLength);
		return m_Volume*ratio;
	}
	//............................Box........................

#ifndef PLATFORM_SPU
	void C_BoxShape::CalculateVolume()
	{
		using namespace Maths;
		const HVec3 _min = m_Box.Centre() - m_Box.Extent();
		const HVec3 _max = m_Box.Centre() + m_Box.Extent();

		const HVec3 dimension = _max - _min;
		m_Volume = dimension.X() * dimension.Y() * dimension.Z();
	}

	void C_BoxShape::DebugRender(DbUtil::I_DebugRender* pRender) const
	{
		const Maths::HVec3 _max(m_Box.Extent());
		const Maths::HVec3 _min(-_max);
		pRender->Draw3dBoxLit(_min, _max, m_PosRot, Maths::RGBA(255, 0, 0, 255));
	}
#endif

	//.........................ConeShape...................
#ifndef PLATFORM_SPU
	void C_ConeShape::CalculateVolume()
	{
		using namespace Maths;
		const HF32 sRadius = m_Cone.BaseRadius() * m_Cone.BaseRadius();
		//const HF32 axisHeight = m_Cone.m_Height * Constants<HF32>::Two();
		const HF32 pi(Constants<HF32>::Pi());
		const HF32 oneThird(Constants<HF32>::One()/Constants<HF32>::Three());
		m_Volume = oneThird * pi * sRadius * m_Cone.Height();
	}

	void C_ConeShape::DebugRender(DbUtil::I_DebugRender* pRender)const
	{
		using namespace Maths;
		const HVec3 s = m_Cone.MidPointAxis() * m_Cone.Height() * Constants<HF32>::Half();
		const HVec3 start = m_Cone.Centre() + s;
		const HVec3 end = m_Cone.Centre() - s;

		const HF32 sCosTheta = Constants<HF32>::One() - m_Cone.SinTheta() * m_Cone.SinTheta();
		//cons::HF32 length = cone.m_MidPointLength;
		//cons::C_Normal n(-cone.MidPointAxis());
		const C_Normal n(m_Cone.MidPointAxis());
		const HF32 gap = m_Cone.Height()/20.f;
		for(U32 i=0; i<20; ++i)
		{
			const HF32 dist = i*gap;
			const HF32 sDist = dist*dist;
			const HF32 ss = sDist/sCosTheta;
			const HF32 r =Maths::Sqrt(ss - sDist);
			pRender->Draw3dCircle(start - n*dist, n, r, RGBA(255, 0, 0, 255));
		}
		pRender->Draw3dLine(start, end, RGBA(255, 0, 0, 0));
		pRender->Draw3dCircle(end, n, m_Cone.BaseRadius(), RGBA(255, 0, 0, 255));
		
	}
#endif


	Maths::HF32 C_ConeShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	{
		using namespace Maths;
		const HVec3 support2=m_Cone.Support(-normal);
		const HF32 d= (normal|(support2-p));
		const HF32 zero = Constants<HF32>::Zero();

		if(d.GE(zero))
		{
			ratio = zero;
			return zero;
		}

		const HVec3 support1=m_Cone.Support(normal);
		const HVec3 v =support2 - support1;
		const HF32 length=Abs((normal|v));
		const HF32 dist = Abs(d);

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(support1, 0.25f, RGBA(255, 0, 0, 255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(support2, 0.25f, RGBA(0, 0, 255, 255));
		
		const HF32 one = Constants<HF32>::One();

		const Scalar_Bool con((dist - length).GT(zero));
		ratio=Select(con, one, dist/length);

		Collision::C_CRAxisBox box;
		TransformBox(box);

		const HVec3 _min = box.centre - box.extents;
		const HVec3 _max = box.centre + box.extents;
		const HF32 maxDist = normal|(_max - p);
		const HVec3 mid = _max - normal*maxDist;

		//centroid = Select(con, m_Box.Centre(), support2 + normal*dist*Constants<HF32>::Half());
		centroid = Select(con, m_Cone.Centre(), (mid + _min)*Constants<HF32>::Half());

		return m_Volume*ratio;

	}


	//......................Capsule..................
#ifndef PLATFORM_SPU
	void C_CapsuleShape::CalculateVolume()
	{
		//Calculate the volume of two end caps(sphere) and the body(cylinder) pi*r2*(4/3*r + height);
		using namespace Maths;
		const HF32 pi = Constants<HF32>::Pi();
		const HF32 c(Maths::Constants<HF32>::Four()*SharedPhysics::Constants::Third());
		const HF32 r2 = m_Capsule.Radius() * m_Capsule.Radius();
		const HF32 height = (m_Capsule.End()- m_Capsule.Start()).Length();
		m_Volume =  pi*r2*(c*m_Capsule.Radius() + height);
	}

	void C_CapsuleShape::DebugRender(DbUtil::I_DebugRender* pRender)const
	{
		using namespace Maths;
		const U32 numSegments = 2;
		/*const HF32 one(Constants<HF32>::One());
		const HF32 two(Constants<HF32>::Two());*/
		const HVec3 d(m_Capsule.End() - m_Capsule.Start());
		const HF32 length(d.Length());
		const HVec3 unitD(d/length);
		const HF32 stepSize(Constants<Maths::HF32>::Half());
		HF32 ratio(Maths::Constants<Maths::HF32>::Zero());
		for(int i=0; i<numSegments; i++)
		{
			HVec3 e(m_Capsule.Start()+d*ratio);
			ratio += stepSize;
		}
		pRender->Draw3dStar(m_Capsule.Start(), 0.025f, RGBA(255, 255, 0));
		pRender->Draw3dStar(m_Capsule.End(), 0.025f, RGBA(255, 0, 255));
		pRender->Draw3dLine(m_Capsule.Start(), m_Capsule.End(), RGBA(0, 255, 0));
		pRender->Draw3dSphere(m_Capsule.Start(), m_Capsule.Radius(), RGBA(255, 0, 0));
		pRender->Draw3dSphere(m_Capsule.End(), m_Capsule.Radius(), RGBA(255, 0, 0));
		pRender->Draw3dArrow(m_Capsule.Start(), m_PosRot.Rotation().AX(), 0.1f, 0.01f, RGBA(255, 0, 0));
	}

#endif

	Maths::HF32 C_CapsuleShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	{
		using namespace Maths;
		
		const HVec3 support2=m_Capsule.Support(-normal);
	
		const HF32 d = normal|(support2-p);
		const HF32 zero = Constants<HF32>::Zero();

		if(d.GE(zero))
		{
			ratio = zero;
			return zero;
		}
		
		const HVec3 support1=m_Capsule.Support(normal);
		const HVec3 v =support2 - support1;
		const HF32 length=Abs(normal|v);
		const HF32 dist = Abs(d);

		const Scalar_Bool con((dist - length).GE(zero));
		ratio=Select(con, Constants<HF32>::One(), dist/length);

		const HF32 d1 = normal|(support1 - p);
		const HVec3 pro1 = support1 - normal *d1;
		const HVec3 pro2 = support2 + normal * dist;

		const HF32 half = Constants<HF32>::Half();

		const HVec3 averPro=(pro1 + pro2)*half;
	
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(averPro, 0.25f, RGBA(255, 0, 255, 255));
		//centroid = Select(con, m_Capsule.Centre(), averPro - normal * dist *half);
		centroid = Select(con, m_Capsule.Centre(), averPro);


		

		/*Collision::C_CRAxisBox box;
		TransformBox(box);

		const HVec3 _min = box.centre - box.extents;
		const HVec3 _max = box.centre + box.extents;
		const HF32 maxDist = normal|(_max - p);
		const HVec3 mid = _max - normal*maxDist;

		centroid = Select(con, m_Capsule.Centre(), (mid + _min)*Constants<HF32>::Half());*/
		//centroid = support2 + normal* dist*half;


		return m_Volume*ratio;
	}

	//..........................Cylinder...................
#ifndef PLATFORM_SPU
	void C_CylinderShape::CalculateVolume()
	{
		using namespace Maths;
		const HF32 pi=Constants<HF32>::Pi();
		const HF32 halfHeight=m_Cylinder.HalfHeight();
		const HF32 height=halfHeight * Constants<HF32>::Two();
		const HF32 r2 = m_Cylinder.Radius() * m_Cylinder.Radius();
		m_Volume = pi*r2*height;
	}

	void C_CylinderShape::DebugRender(DbUtil::I_DebugRender* pRender)const
	{
		using namespace Maths;
		const U32 numSegments = 10;
		const HF32 stepSize(0.1f);
		HF32 ratio(Maths::Constants<Maths::HF32>::Zero());
		for(int i=0; i<numSegments; i++)
		{
			HVec3 e1(m_Cylinder.Centre()+m_Cylinder.CentreAxis()*ratio*m_Cylinder.HalfHeight());
			HVec3 e2(m_Cylinder.Centre()-m_Cylinder.CentreAxis()*ratio*m_Cylinder.HalfHeight());
			pRender->Draw3dCircle(e1, m_Cylinder.CentreAxis(), m_Cylinder.Radius(), RGBA(255, 0, 0));
			pRender->Draw3dCircle(e2, m_Cylinder.CentreAxis(), m_Cylinder.Radius(), RGBA(255, 0, 0));
			ratio += stepSize;
		}
	}
#endif

	Maths::HF32 C_CylinderShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid,Maths::HF32& ratio)
	{
		using namespace Maths;
		
		const HVec3 support2=m_Cylinder.Support(-normal);
		
		const HF32 d = normal|(support2-p);
		const HF32 zero =Constants<HF32>::Zero();

		if(d.GE(zero))
		{
			ratio =zero;
			return zero;
		}
		const HVec3 support1=m_Cylinder.Support(normal);
		const HVec3 v =support2 - support1;
		const HF32 length=Abs(normal|v);
		const HF32 dist = Abs(d);
		
		const Scalar_Bool con((dist - length ).GE(zero));
		ratio=Select(con, Constants<HF32>::One(), dist/length);

		const HF32 d1 = normal|(support1 - p);
		const HVec3 pro1 = support1 - normal *d1;
		const HVec3 pro2 = support2 + normal * dist;

		const HF32 half = Constants<HF32>::Half();

		const HVec3 averPro=(pro1 + pro2)*half;

		//centroid = Select(con, m_Cylinder.Center(), support2 + normal* dist*Constants<HF32>::Half());

		centroid = Select(con, m_Cylinder.Centre(), averPro);


		return m_Volume*ratio;
	}

	//............................C_ConvexHull....................

	/*void C_ConvexHullShape::CalculateVolume()
	{
		using namespace Maths;
		HF32 volume=Constants<HF32>::Zero();
		const HF32 cons(Maths::Constants<HF32>::Sixth());
		for(U32 i=0; i<m_Hull.m_NumTriangles; ++i)
		{
			Collision::C_ConvexTriangle& tri=m_Hull.m_pTriangles[i];
			const HVec3 a=Load<HVec3,MathsStorage::Float3>(&m_Hull.m_pVertices[tri.m_VertexA]);
			const HVec3 b=Load<HVec3,MathsStorage::Float3>(&m_Hull.m_pVertices[tri.m_VertexB]);
			const HVec3 c=Load<HVec3,MathsStorage::Float3>(&m_Hull.m_pVertices[tri.m_VertexC]);
		
			const HF32 vol(Abs(a|(b^c)));
			volume += vol;
		}

		m_Volume = volume*cons;
	}*/
#ifndef PLATFORM_SPU
	void C_ConvexHullShape::CalculateVolume()
	{
		using namespace Maths;
		HF32 volume=Constants<HF32>::Zero();
		const HF32 cons(Maths::Constants<HF32>::Sixth());
		const HVec3 range(m_Hull.Range());
		const U32 numTriangles(m_Hull.GetNumTriangles());

		for(U32 i=0; i<numTriangles; ++i)
		{
			
			Collision::C_ConvexTriangle& tri = m_Hull.GetTriangle(i);
			const HVec3 a(m_Hull.GetVertex(tri.m_VertexA));
			const HVec3 b(m_Hull.GetVertex(tri.m_VertexB));
			const HVec3 c(m_Hull.GetVertex(tri.m_VertexC));

		
			const HF32 vol(Abs(a|(b^c)));
			volume += vol;
		}

		m_Volume = volume*cons;
	}

	void C_ConvexHullShape::DebugRender(DbUtil::I_DebugRender* pRender)const
	{
		
		using namespace Maths;
		INTPTR addy = (INTPTR)alloca(sizeof(Maths::HVec3)*m_Hull.GetNumVertices() + 16);	// [Blur2PC][x64] full address
		addy = (addy + 15)& ~(INTPTR)0xf;
		Maths::HVec3* verts((Maths::HVec3*)addy);
		const HMat33 orientation(m_Hull.Orientation());
		const HVec3 position(m_Hull.Centre());
		const RGBA colour(0, 255, 0);
		const HVec3 range(m_Hull.Range());//(pShape->m_Hull.m_High - pShape->m_Hull.m_Low);
		U32 numVertices =m_Hull.GetNumVertices();
		for(U32 b = 0; b < numVertices; ++b)
		{
			const Maths::HVec3 point(m_Hull.GetVertex(b));
			verts[b] = position + point*orientation;
		}

		U32 numTriangles = m_Hull.GetNumTriangles();
		for(U32 b = 0; b < numTriangles; ++b)
		{
			Collision::C_ConvexTriangle& tri(m_Hull.GetTriangle(b));
			pRender->Draw3dTriangleLit(verts[tri.m_VertexA],verts[tri.m_VertexB], verts[tri.m_VertexC], colour);
		}
	}
#endif


	//Maths::HF32 C_ConvexHullShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	//{
	//	using namespace Maths;
	//	
	//	const HVec3 support2=m_Hull.Support(-normal);

	//	const HF32 d = normal|(support2-p);
	//	const HF32 zero = Constants<HF32>::Zero();
	//	if(d.GE(zero))
	//	{
	//		ratio =zero;
	//		return zero;
	//	}
	//	const HVec3 support1=m_Hull.Support(normal);
	//	const HVec3 v =support2 - support1;
	//	const HF32 length=Abs((normal|v));
	//	const HF32 dist = Abs(d);
	//	
	//	const Scalar_Bool con((dist - length).GT(zero));
	//	ratio=Select(con, Constants<HF32>::One(), dist/length);

	//	Collision::C_CRAxisBox aabb;
	//	this->TransformBox(aabb);
	//	const HVec3 _min = aabb.centre - aabb.extents;
	//	HVec3 _max = aabb.centre + aabb.extents;
	//	_max.Y(_min.Y() + dist);

	//	//centroid = support2 + normal*dist*Constants<HF32>::Half();
	//	centroid = Select(con, m_Hull.Centre(), (_max + _min)*Constants<HF32>::Half());

	//	return m_Volume*ratio;
	//}

	static inline Maths::HF32 TetrahedronVolume(Maths::HVec3& centroid, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF v1, const Maths::HVec3 PCREF v2, const Maths::HVec3 PCREF v3)
	{
		//const Maths::HF32 sixth(Maths::Constants<Maths::HF32>::Sixth());
		const Maths::HF32 quarter(Maths::Constants<Maths::HF32>::Quarter());

		/*const HF32 volume(Abs(v1|(v2^v3)));
		centroid += quarter * volume * (v1 + v2 + v3 + p);
		return volume;*/

		const Maths::HVec3 a(v2 - v1);
		const Maths::HVec3 b(v3 - v1);
		const Maths::HVec3 r(p - v1);

		const Maths::HF32  volume(Maths::Abs(r | (a ^ b)));
		centroid += quarter*volume*(v1 + v2 + v3 + p);
		return volume;
	}


	static Maths::HF32 ClipTriangle( Maths::HVec3& centroid, const Maths::HVec3 PCREF centre, const Maths::HVec3 PCREF v1, const Maths::HVec3 PCREF v2, const Maths::HVec3 PCREF v3,
							const Maths::HF32 PCREF d1, const Maths::HF32 PCREF d2, const Maths::HF32 PCREF d3)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HVec3 vc1(v1 + (d1/(d1 - d2))*(v2 - v1));
		Maths::HF32 volume(zero);

		if (d1 < zero)
		{
			if (d3 < zero)
			{
				// Case B - a quadrilateral or two triangles.
				const Maths::HVec3 vc2 = v2 + (d2/(d2 - d3))*(v3 - v2);
				volume += TetrahedronVolume(centroid, centre, vc1, vc2, v1);
				volume += TetrahedronVolume(centroid, centre, vc2, v3, v1);
			}
			else
			{
				// Case A - a single triangle.
				const Maths::HVec3 vc2 = v1 + (d1/(d1 - d3))*(v3 - v1);
				volume += TetrahedronVolume(centroid, centre, vc1, vc2, v1);
			}
		}
		else
		{
			if (d3 < zero)
			{
				// Case B
				const Maths::HVec3 vc2 = v1 + (d1/(d1 - d3))*(v3 - v1);
				volume += TetrahedronVolume(centroid, centre, vc1, v2, v3);
				volume += TetrahedronVolume(centroid, centre, vc1, v3, vc2);
			}
			else
			{
				// Case A
				const Maths::HVec3 vc2 = v2 + (d2/(d2 - d3))*(v3 - v2);
				volume += TetrahedronVolume(centroid, centre, vc1, v2, vc2);
			}
		}

		return volume;
	}

	Maths::HF32 C_BoxShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	{
		using namespace Maths;
		const HVec3 support2=m_Box.Support(-normal);
		const HF32 d= (normal|(support2-p));
		const HF32 zero = Constants<HF32>::Zero();


		if(d.GE(zero))
		{
			ratio = zero;
			return zero;
		}

		const HVec3 support1=m_Box.Support(normal);
		const HVec3 v =support2 - support1;
		const HF32 length=Abs((normal|v));
		const HF32 dist = Abs(d);

		const HF32 one(Constants<HF32>::One());

		const Scalar_Bool con((dist - length).GT(zero));
		ratio=Select(con, one, dist/length);

		Collision::C_CRAxisBox box;
		TransformBox(box);

		const HVec3 _min = box.centre - box.extents;
		const HVec3 _max = box.centre + box.extents;
		const HF32 maxDist = normal|(_max - p);
		const HVec3 mid = _max - normal*maxDist;

		//centroid = Select(con, m_Box.Centre(), support2 + normal*dist*Constants<HF32>::Half());
		centroid = Select(con, m_Box.Centre(), (mid + _min)*Constants<HF32>::Half());

		return m_Volume*ratio;

	}

	const Maths::HF32 GetSubmergedVolume(const Maths::HVec3 PCREF v1, const Maths::HVec3 PCREF v2, const Maths::HVec3 PCREF v3,
		const Maths::HVec3 PCREF centre, const Maths::C_Normal PCREF localNormal, const Maths::HF32 PCREF localD, Maths::HVec3& centroid)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 d1((v1 | localNormal) - localD);
		const Maths::HF32 d2((v2 | localNormal) - localD);
		const Maths::HF32 d3((v3 | localNormal) - localD);

		if ((d1 * d2) < zero)
		{
			// v1-v2 crosses the plane
			return ClipTriangle(centroid, centre, v1, v2, v3, d1, d2, d3);
		}
		else if ((d1 * d3) < zero)
		{
			// v1-v3 crosses the plane
			return ClipTriangle(centroid, centre, v3, v1, v2, d3, d1, d2);
		}
		else if ((d2 * d3) < zero)
		{
			// v2-v3 crosses the plane
			return ClipTriangle(centroid, centre, v2, v3, v1, d2, d3, d1);
		}
		else if ((d1 < zero) || (d2 < zero) || (d3 < zero))
		{
			// fully submerged
			return TetrahedronVolume(centroid, centre, v1, v2, v3);
		}
		return zero;
	}

	//Maths::HF32 C_BoxShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	//{
	//	using namespace Maths;
	//	Spu::MemBaseMarker marker;
	//	const HVec3 support2=m_Box.Support(-normal);
	//	const Maths::HF32 dd(normal | p);
	//	const HF32 d((normal|support2) - dd);
	//	const HF32 zero(Constants<HF32>::Zero());
	//	const HF32 one(Constants<HF32>::One());
	//	if(d.GE(zero))
	//	{
	//		ratio =zero;
	//		return zero;
	//	}

	//	const Maths::HVec3 support1(m_Box.Support(normal));
	//	const Maths::HF32 d1((normal | support1) - dd);

	//	if(d1.LE(zero))
	//	{
	//		ratio = one;
	//		centroid = m_PosRot.Translation();
	//		return m_Volume;
	//	}

	//	//Get submerged centroid!!!
	//	const Maths::HF32 centroidD((m_Box.c | normal) - dd);
	//	const Maths::HVec3 difVec(-centroidD * normal);
	//	const Maths::HVec3 centre(Maths::Select(centroidD.GT(zero), m_PosRot.Rotation() * difVec, Maths::HVec3(zero)));
	//	

	//	const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());
	//	const Maths::HF32 sixth(Maths::Constants<Maths::HF32>::Sixth());


	//	//OK. Its definitely crossed the plane. Let's estimate the submerged volume...

	//	const Maths::HMat33 invRot(m_PosRot.Rotation().InverseRotation());
	//	const Maths::C_Normal localNormal(m_PosRot.Rotation() * normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	//	const Maths::C_Normal localP(p * m_PosRot.InverseRotTrans());
	//	const Maths::HF32 localD(localP | localNormal);
	//	Maths::HF32 volume(zero);

	//	const Maths::HVec3 vertices [8] =
	//	{
	//		-m_Box.e, 
	//		Maths::HVec3(m_Box.e.X(), -m_Box.e.Y(), -m_Box.e.Z()),
	//		Maths::HVec3(-m_Box.e.X(), m_Box.e.Y(), -m_Box.e.Z()),
	//		Maths::HVec3(-m_Box.e.X(), -m_Box.e.Y(), m_Box.e.Z()),
	//		Maths::HVec3(m_Box.e.X(), m_Box.e.Y(), -m_Box.e.Z()),
	//		Maths::HVec3(m_Box.e.X(), -m_Box.e.Y(), m_Box.e.Z()),
	//		Maths::HVec3(-m_Box.e.X(), m_Box.e.Y(), m_Box.e.Z()),
	//		m_Box.e
	//	};

	//	volume += GetSubmergedVolume(vertices[0], vertices[1], vertices[2], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[1], vertices[2], vertices[4], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[0], vertices[6], vertices[2], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[0], vertices[3], vertices[6], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[3], vertices[7], vertices[6], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[3], vertices[5], vertices[7], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[5], vertices[7], vertices[1], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[1], vertices[7], vertices[4], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[2], vertices[6], vertices[7], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[2], vertices[7], vertices[4], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[0], vertices[3], vertices[5], centre, localNormal, localD, centroid);
	//	volume += GetSubmergedVolume(vertices[0], vertices[5], vertices[1], centre, localNormal, localD, centroid);

	//	if(volume <= epsilon)
	//	{
	//		return zero;
	//	}
	//	//It did actually hit the plane somewhere, so need to calculate ratio + centroid...

	//	
	//	centroid = -centroid/volume;
	//	centroid = centroid * m_PosRot;
	//	volume = volume * sixth;
	//	ratio = volume/m_Volume;

	//	return volume;
	//}


	//More expensive version that actually uses polygons!!!
	Maths::HF32 C_ConvexHullShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	{
		using namespace Maths;
		Spu::MemBaseMarker marker;
		Collision::C_SpuConvexHull hull(m_Hull, true);
		const HVec3 support2=hull.Support(-normal);
		const Maths::HF32 dd(normal | p);
		const HF32 d = (normal|support2) - dd;
		const HF32 zero = Constants<HF32>::Zero();
		const HF32 one = Constants<HF32>::One();
		if(d.GE(zero))
		{
			ratio =zero;
			return zero;
		}

		const Maths::HVec3 support1(hull.Support(normal));
		const Maths::HF32 d1((normal | support1) - dd);

		if(d1.LE(zero))
		{
			ratio = one;
			centroid = m_PosRot.Translation();
			return m_Volume;
		}

		//Get submerged centroid!!!
		const Maths::HF32 centroidD((hull.Centre() | normal) - dd);
		const Maths::HVec3 difVec(-centroidD * normal);
		const Maths::HVec3 centre(Maths::Select(centroidD.GT(zero), m_PosRot.Rotation() * difVec, Maths::HVec3(zero)));
		const Maths::HVec3 worldSpaceCentre(centre * m_PosRot);
		

		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());
		const Maths::HF32 sixth(Maths::Constants<Maths::HF32>::Sixth());


		//OK. Its definitely crossed the plane. Let's estimate the submerged volume...

		const Maths::HMat33 invRot(m_PosRot.Rotation().InverseRotation());
		const Maths::HVec3 translation(m_PosRot.Translation());
		const Maths::C_Normal localNormal(normal * invRot, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		const Maths::HVec3 localP((p - translation) * invRot);
		const Maths::HF32 localD(localP | localNormal);
		Maths::HF32 volume(zero);

		//Loop through all the polygons and clip them to the plane!!!

		U32 numTriangles = hull.GetNumTriangles();
		for(U32 a = 0; a < numTriangles; ++a)
		{
			//Collision::C_ConvexTriangle* tri = &hull.m_pTriangles[a];
			Collision::C_ConvexTriangle* tri = &hull.GetTriangle(a);
			const HVec3 v1(hull.GetLocalVertex(tri->m_VertexA));
			const HVec3 v2(hull.GetLocalVertex(tri->m_VertexB));
			const HVec3 v3(hull.GetLocalVertex(tri->m_VertexC));


			const Maths::HF32 d1((v1 | localNormal) - localD);
			const Maths::HF32 d2((v2 | localNormal) - localD);
			const Maths::HF32 d3((v3 | localNormal) - localD);

			if ((d1 * d2) < zero)
			{
				// v1-v2 crosses the plane
				volume += ClipTriangle(centroid, centre, v1, v2, v3, d1, d2, d3);
			}
			else if ((d1 * d3) < zero)
			{
				// v1-v3 crosses the plane
				volume += ClipTriangle(centroid, centre, v3, v1, v2, d3, d1, d2);
			}
			else if ((d2 * d3) < zero)
			{
				// v2-v3 crosses the plane
				volume += ClipTriangle(centroid, centre, v2, v3, v1, d2, d3, d1);
			}
			else if (d1.LT(zero) | d2.LT(zero) | d3.LT(zero))
			{
				// fully submerged
				volume += TetrahedronVolume(centroid, centre, v1, v2, v3);
			}

		}

		if(volume <= epsilon)
		{
			return zero;
		}
		//It did actually hit the plane somewhere, so need to calculate ratio + centroid...

		centroid = centroid/volume;
		centroid = centroid * m_PosRot;
		volume = volume * sixth;
		ratio = volume/m_Volume;
		return volume;
	}

	//..................Composite............................
#ifndef PLATFORM_SPU
	void C_CompositeShape::CalculateVolume()
	{
		using namespace Maths;

		C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

		while(iter.HasNextShape())
		{
			iter.NextShape();
			C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
			if(pShape->HasVolume())
			{
				pShape->CalculateVolume();
				m_Volume += pShape->m_Volume;
			}
		}
	}


	Maths::HF32 C_CompositeShape::GetSubVolume(const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	{
		using namespace Maths;

		const HF32 zero(Constants<HF32>::Zero());
		HF32 volume(zero);
		HF32 tempRatio(zero);
		HF32 total(zero);
		const Maths::HF32 one(Maths::Constants<HF32>::One());
		Maths::HF32 numShapes(zero);
		HF32 totalVolume(zero);
		C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

		while(iter.HasNextShape())
		{
			iter.NextShape();
			C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
			if(pShape->HasVolume())
			{
				Maths::HVec3 centre(zero);
				tempRatio = zero;
				const Maths::HF32 submergedVolume = pShape->GetSubVolume(p, normal, centre, tempRatio);
				volume += submergedVolume;
				centroid += centre * submergedVolume;
				totalVolume += pShape->m_Volume;
				total += tempRatio;
				numShapes += one;
			}
		}
		ratio = volume/totalVolume;
		centroid = centroid/volume;
		return volume;
	}

	void C_CompositeShape::DebugRender(DbUtil::I_DebugRender* pRender)const
	{
		C_CompositeShapeIterator iter(m_pShapes, m_CompositeSize);

		while(iter.HasNextShape())
		{
			iter.NextShape();
			C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
			pShape->DebugRender(pRender);
		}
	}
#endif


	//......................................C_CharacterControllerShape............................
#ifndef PLATFORM_SPU
	void C_CharacterControllerShape::DebugRender(DbUtil::I_DebugRender* pRender)const
	{
		using namespace Maths;
		RGBA colour(0, 255, 0);
		RGBA colour0(255, 0, 0);

		{
			Collision::C_Capsule& capsule = m_Controller.m_EnclosingCapsule;
			const U32 numSegments = 2;
			const HVec3 d(capsule.End() - capsule.Start());
			const HF32 length(d.Length());
			const HVec3 unitD(d/length);
			const HF32 stepSize(Maths::Constants<Maths::HF32>::Half());
			HF32 ratio(Maths::Constants<Maths::HF32>::Zero());
			for(int i=0; i<numSegments; i++)
			{
				HVec3 e(capsule.Start()+d*ratio);
				pRender->Draw3dCircle(e, unitD, capsule.Radius(), colour);
				ratio += stepSize;
			}
			pRender->Draw3dSphere(capsule.Start(), capsule.Radius(), colour);
			pRender->Draw3dSphere(capsule.End(), capsule.Radius(), colour);
		}

		{
			Collision::C_Capsule& capsule = m_Controller.m_Capsule;
			const U32 numSegments = 2;
			const HVec3 d(capsule.End() - capsule.Start());
			const HF32 length(d.Length());
			const HVec3 unitD(d/length);
			const HF32 stepSize(Maths::Constants<Maths::HF32>::Half());
			HF32 ratio(Maths::Constants<Maths::HF32>::Zero());
			for(int i=0; i<numSegments; i++)
			{
				HVec3 e(capsule.Start()+d*ratio);
				pRender->Draw3dCircle(e, unitD, capsule.Radius(), colour);
				ratio += stepSize;
			}
			pRender->Draw3dSphere(capsule.Start(), capsule.Radius(), colour0);
			pRender->Draw3dSphere(capsule.End(), capsule.Radius(), colour0);
		}
		pRender->Draw3dLine(m_Controller.m_Ray.m_start,m_Controller.m_Ray.m_end, colour0);
	}
#endif

}
