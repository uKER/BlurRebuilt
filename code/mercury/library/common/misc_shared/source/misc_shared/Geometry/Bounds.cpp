#include "MiscSharedPch.h"

#include <core/Core.h>
#include <core/Maths.h>
#include <system/DebugLog.h>
#include <core/containers/string.h>

#include <misc_shared/geometry/Bounds.h>

namespace Bounds
{

Debug::Logger &g_Log = Debug::GetLogger("mercury.misc_shared.bounds");

C_Bounds::C_Bounds()
{
    m_SphereValid = false;
    m_BoxValid = false;
}

C_Bounds::~C_Bounds()
{
}

void C_Bounds::AddPoint(const Maths::HVec3& point)
{
    if(!point.Finite())
    {
        LOGERROR( g_Log,  "Value not valid\n");
    }

	m_PointList.PushBack(point);

    m_SphereValid = false;
    m_BoxValid = false;
}

void C_Bounds::AddSphere(const Maths::C_Sphere& sphere)
{
    if(!((Maths::HVec3)sphere.Centre()).Finite())
    {
        LOGERROR( g_Log,  "Value not valid");
    }

	m_SphereList.PushBack(sphere);

    m_SphereValid = false;
    m_BoxValid = false;
}

#ifdef BUILD_CONFIG_RELEASE
#pragma optimize ("", off) // BUILD_CONFIG_RELEASE
#endif

NOALIAS /*__force*/inline const Maths::HVec3 ClosestPtPointTriangleLocal(
    const Maths::HVec3 PCREF p,
    const Maths::HVec3 PCREF a,
    const Maths::HVec3 PCREF b,
    const Maths::HVec3 PCREF c )
{
    using namespace Maths;
    
	//HVec3 ab = b - a;
	//HVec3 ac = c - a;
	//HVec3 ap = p - a;

	//HF32 d1 = ab | ap;
	//HF32 d2 = ac | ap;
	//const HF32 zero(0.f);
	//const HF32 one(1.f);

	//if(d1.LE(zero) && d2.LE(zero))
	//	return a;

	//HVec3 bp = p - b;
	//HF32 d3 = ab | bp;
	//HF32 d4 = ac | bp;
	//if(d3.GE(zero) && d4.LE(d3))
	//	return b;

	//HF32 vc = (d1 * d4) - (d3 * d2);
	//if(vc.LE(zero) && d1.GE(zero) && d3.LE(zero))
	//{
	//	HF32 v = d1/(d1 - d3);
	//	return a + (v * ab);
	//}

	//HVec3 cp = p - c;
	//HF32 d5 = ab | cp;
	//HF32 d6 = ac | cp;

	//if(d6.GE(zero) && d5.LE(d6))
	//	return c;

	//HF32 vb = (d5 * d2) - (d1 * d6);
	//if(vb.LE(zero) && d2.GE(zero) && d6.LE(zero))
	//{
	//	HF32 w = d2/(d2 - d6);
	//	return a + (w * ac);
	//}

	//HF32 va = (d3 * d6) - (d5 * d4);
	//if(va.LE(zero) && (d4 - d3).GE(zero) && (d5 - d6).GE(zero))

	//{
	//	HF32 w = (d4 - d3)/((d4 - d3) + (d5 - d6));
	//	return b + (w * (c - b));
	//}

	//HF32 denom = one/(va + vb + vc);
	//HF32 v = vb * denom;
	//HF32 w = vc * denom;
	//return a + (ab * v) + (ac * w);
    
    const HF32 zero( Constants<HF32>::Zero() );
    
    const HVec3 ab( b - a );
    const HVec3 ac( c - a );
    const HVec3 ap( p - a );
    const HVec3 bp( p - b );
    const HVec3 cp( p - c );

    const HF32 d1( ab | ap );
    const HF32 d2( ac | ap );
    const HF32 d3( ab | bp );
    const HF32 d4( ac | bp );
    const HF32 d5( ab | cp );
    const HF32 d6( ac | cp );
    
    const HF32 d3_times_d6( d3 * d6 );
    const HF32 d5_times_d2( d5 * d2 );
    const HF32 d1_times_d4( d1 * d4 );
    
    const HF32 d1_minus_d3( d1 - d3 );
    const HF32 d2_minus_d6( d2 - d6 );
    const HF32 d4_minus_d3( d4 - d3 );
    const HF32 d5_minus_d6( d5 - d6 );
    
    const HF32 va( NegMulSub( d5, d4, d3_times_d6 ) /* ( d3 * d6 ) - ( d5 * d4 ) */ );
    const HF32 vb( NegMulSub( d1, d6, d5_times_d2 ) /* ( d5 * d2 ) - ( d1 * d6 ) */ );
    const HF32 vc( NegMulSub( d3, d2, d1_times_d4 ) /* ( d1 * d4 ) - ( d3 * d2 ) */ );
    
    const HF32 d4_minus_d3_plus_d5_minus_d6( ( d4_minus_d3 ) + ( d5_minus_d6 ) );
    const HF32 va_plus_vb_plus_vc( va + vb + vc );
    
    const HF32 denom_u1( Reciprocal( d1_minus_d3 ) );
    const HF32 denom_u2( Reciprocal( d2_minus_d6 ) );
    const HF32 denom_u3( Reciprocal( d4_minus_d3_plus_d5_minus_d6 ) );
    const HF32 denom_vw( Reciprocal( va_plus_vb_plus_vc ) );
    
    const HF32 u1( d1 * denom_u1 );
    const HF32 u2( d2 * denom_u2 );
    const HF32 u3( d4_minus_d3 * denom_u3 );
    
    const HF32 v( vb * denom_vw );
    const HF32 w( vc * denom_vw );
    
    #if 0
    
    if ( d1.LE( zero ) && d2.LE( zero ) )
        return a;
    
    if ( d3.GE( zero ) && d4.LE( d3 ) )
        return b;
    
    if ( vc.LE( zero ) && d1.GE( zero ) && d3.LE( zero ) )
        return a + ( u1 * ab );
    
    if ( d6.GE( zero ) && d5.LE( d6 ) )
        return c;
    
    if ( vb.LE( zero ) && d2.GE( zero ) && d6.LE( zero ) )
        return a + ( u2 * ac );
    
    if ( va.LE( zero ) && ( d4 - d3 ).GE( zero ) && ( d5 - d6 ).GE( zero ) )
        return b + ( u3 * ( c - b ) );
    
    return a + ( ab * v ) + ( ac * w );
    
    #else
    
    const HVec3 result0( a );
    const HVec3 result1( b );
    const HVec3 result2( a + ( u1 * ab ) );
    const HVec3 result3( c );
    const HVec3 result4( a + ( u2 * ac ) );
    const HVec3 result5( b + ( u3 * ( c - b ) ) );
    const HVec3 result6( a + ( ab * v ) + ( ac * w ) );
    
    const Vector3_Bool use_result0( d1.LE( zero ) && d2.LE( zero ) );
    const Vector3_Bool use_result1( d3.GE( zero ) && d4.LE( d3 ) );
    const Vector3_Bool use_result2( vc.LE( zero ) && d1.GE( zero ) && d3.LE( zero ) );
    const Vector3_Bool use_result3( d6.GE( zero ) && d5.LE( d6 ) );
    const Vector3_Bool use_result4( vb.LE( zero ) && d2.GE( zero ) && d6.LE( zero ) );
    const Vector3_Bool use_result5( va.LE( zero ) && ( d4_minus_d3 ).GE( zero ) && ( d5_minus_d6 ).GE( zero ) );
    
    HVec3 result = result6;
    result = Select( use_result5, result5, result );
    result = Select( use_result4, result4, result );
    result = Select( use_result3, result3, result );
    result = Select( use_result2, result2, result );
    result = Select( use_result1, result1, result );
    result = Select( use_result0, result0, result );
    return result;
    
    #endif
}

const Maths::C_Sphere	C_Bounds::CalculateSphere5()
{
	//Basic algorithm (this should be the optimal sphere always)
	//Find the largest medial axis of the system of spheres/points (the pair of points within the system with 
	//greatest distance. This is the diameter of the sphere. Done :-)
	CalculateBox();
	Maths::HF32 maxDist(-1e38f);
	Maths::HVec3 point1, point2;
	const Maths::HF32 half(0.5f);
	const Maths::HF32 zero(Maths::ZERO_INIT);

	Maths::HVec3 bestCentre;
	Maths::HF32 bestRadius;

	for(int a = 0; a < this->m_PointList.Size(); ++a)
	{
		Maths::HVec3& p1 = m_PointList[a];
		for(int b = a + 1; b < m_PointList.Size(); ++b)
		{
			Maths::HVec3& p2 = m_PointList[b];
			Maths::HVec3 dir(p2 - p1);
			Maths::HF32 dist(dir.Length());
			if(dist.GT(maxDist))
			{
				point1 = p1;
				point2 = p2;
				maxDist = dist;
			}
		}
		for(int b = 0; b < m_SphereList.Size(); ++b)
		{
			Maths::HVec3 p2(m_SphereList[b].Centre());
			Maths::HVec3 dir(p2 - p1); 
			Maths::HF32 dist(dir.Length());
			Maths::HF32 rad(m_SphereList[a].Radius());
			if((dist + rad).GT(maxDist))
			{
				maxDist = dist + rad;
				Maths::HF32 scale = (dist + rad)/(dist + 1e-12f);
				point1 = p1;
				point2 = p1 + dir * scale;
			}
		}
	}

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HVec3 p1(m_SphereList[a].Centre());
		Maths::HF32 rad1(m_SphereList[a].Radius());
		for(int b = a + 1; b < m_SphereList.Size(); ++b)
		{
			Maths::HVec3 p2(m_SphereList[b].Centre());
			Maths::HVec3 dir(p2 - p1); 
			Maths::HF32 dist(dir.Length());
			Maths::HF32 rad2(m_SphereList[a].Radius());
			if((dist + rad1 + rad2).GT(maxDist))
			{
				maxDist = dist + rad1 + rad2;
				Maths::HF32 scale1 = (dist + rad1)/(dist + 1e-12f);
				Maths::HF32 scale2 = (dist + rad2)/(dist + 1e-12f);
				point1 = p2 - dir * scale1;
				point2 = p1 + dir * scale2;
			}
		}
	}

	Maths::HVec3 centre((point1 + point2)*half);
	Maths::HF32 radius(maxDist * half);
	radius = radius * radius;

	bool finished = true;
	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		Maths::HVec3 diff(m_PointList[a] - centre);
		Maths::HF32 disSq(diff | diff);
		if(disSq.GT(radius))
		{
			//OK. We've just found a point outside of the sphere...
			radius = disSq; 
			break;
		}
	}
	radius = radius.Sqrt();
	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HVec3 diff(m_SphereList[a].Centre().m_V - centre);
		Maths::HF32 dis(diff.Length());
		if((dis + m_SphereList[a].Radius()).GT(radius))
		{
			//OK. We've just found a point outside of the sphere...
			radius = dis + m_SphereList[a].Radius();
		}
	}

	bestRadius = radius;
	bestCentre = centre;

	//Loop through and find out if this line 
	//Now we have the 2 most distant points, we need to find the point which is most distant
	//from this one - to create a triangle with maximal area within the sphere
	Maths::C_Vertex vpoint1(point1);
	Maths::C_Vertex vpoint2(point2);
	Maths::C_Line line1(vpoint1, vpoint2);

	maxDist = -1e38f;
	Maths::HVec3 point3;
	Maths::HVec3 bestClosestPoint;
	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		Maths::C_Vertex p1(m_PointList[a]);
		Maths::HF32 dist;
		Maths::C_Vertex(closestPoint);
		Maths::C_Normal normal;
		line1.ClosestPoint( p1, closestPoint, normal, dist );
		//line1.ClosestPoint(p1, closestPoint, normal, dist );
		if(dist.GT(maxDist))
		{
			maxDist = dist;
			point3 = p1;
			bestClosestPoint = closestPoint.m_V;
		}
	}

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::C_Vertex p1(m_SphereList[a].Centre());
		Maths::HF32 rad(m_SphereList[a].Radius());
		Maths::HF32 dist;
		Maths::C_Vertex closestPoint;
		Maths::C_Normal normal;
		line1.ClosestPoint(p1, closestPoint, normal, dist );
		if((dist+rad).GT(maxDist))
		{
			maxDist = (dist+rad);
			Maths::HF32 scale = (dist + rad)/(dist + 1e-12f);
			Maths::HVec3 dir((p1 - closestPoint).m_V);
			point3 = closestPoint.m_V + (dir * scale);
			bestClosestPoint = closestPoint.m_V;
		}
	}

	//An approximation to the centroid of the sphere can be found from the mid-point of the triangle...
	centre = (point1 + point2 + point3)/Maths::HF32(3.f);
	radius = zero; 

	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		Maths::HVec3 dif(m_PointList[a] - centre);
		Maths::HF32 dis(dif | dif);
		if(dis.GT(radius))
		{
			finished = false;
			radius = dis;
		}
	}
	radius = radius.Sqrt();

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HVec3 dif(m_SphereList[a].Centre().m_V - centre);
		Maths::HF32 dis(dif.Length() + m_SphereList[a].Radius());
		if(dis.GT(radius))
		{
			finished = false;
			radius = dis;
		}
	}
	finished = true;

	if(bestRadius.GT(radius))
	{
		bestRadius = radius;
		bestCentre = centre;
	}

	//Loop through all the points and spheres, finding the most distant point from the triangle constructed from
	//point1, point2, point3

	maxDist = -FLT_MAX;
	Maths::HVec3 point4;
	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		Maths::HVec3& p(m_PointList[a]);
		Maths::HVec3 closestPoint = ClosestPtPointTriangleLocal(p, point1, point2, point3);
		Maths::HVec3 dif(closestPoint - p);
		Maths::HF32 dis(dif | dif);
		if(dis.GT(maxDist))
		{
			maxDist = dis;
			bestClosestPoint = closestPoint;
			point4 = p;
		}
	}
	maxDist = maxDist.Sqrt();
	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HVec3 p(m_SphereList[a].Centre());
		Maths::HVec3 closestPoint = ClosestPtPointTriangleLocal(p, point1, point2, point3);
		Maths::HVec3 dif(p - closestPoint);
		Maths::HF32 dis(dif.Length());
		if(dis.GT(maxDist + m_SphereList[a].Radius()))
		{
			maxDist = dis + m_SphereList[a].Radius();
			bestClosestPoint = closestPoint;
			Maths::HF32 scal = (dis + m_SphereList[a].Radius())/dis;
			point4 = p + dif * scal;
		}
	}

	Maths::HF32 four(4.f);
	centre = point1 + point2 + point3 + point4;
	centre/= four;

	radius = zero; 

	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		Maths::HVec3 dif(m_PointList[a] - centre);
		Maths::HF32 dis(dif | dif);
		if(dis.GT(radius))
		{
			finished = false;
			radius = dis;
		}
	}
	radius = radius.Sqrt();

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HVec3 dif(m_SphereList[a].Centre().m_V - centre);
		Maths::HF32 dis(dif.Length() + m_SphereList[a].Radius());
		if(dis.GT(radius))
		{
			finished = false;
			radius = dis;
		}
	}

	if(bestRadius.GT(radius))
	{
		bestRadius = radius;
		bestCentre = centre;
	}

	m_Sphere = Maths::C_Sphere(bestCentre, bestRadius);
	return m_Sphere;
}
#ifdef BUILD_CONFIG_RELEASE
#pragma optimize ("", on) // BUILD_CONFIG_RELEASE
#endif

const Maths::C_Sphere	C_Bounds::CalculateSphere4()
{
	const Maths::HF32 epsilon(0.01f);

	Maths::HF32 sumRadii(Maths::ZERO_INIT);
	

	Maths::HVec3 centre(Maths::ZERO_INIT);

	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		centre += m_PointList[a] * epsilon;
		sumRadii += epsilon;
	}

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HF32 rad = m_SphereList[a].Radius();
		centre += m_SphereList[a].Centre() * rad;
		sumRadii += rad;
	}

	centre /= sumRadii;

	//Now loop through all the spheres/points and find the sphere most distant from this one...
	int numSpheres = m_SphereList.Size() + m_PointList.Size();
	Maths::C_Sphere* spheres = new Maths::C_Sphere[numSpheres];
	Maths::C_Sphere** usedSpheres = new Maths::C_Sphere*[numSpheres];
	U32* bitField = new U32[(numSpheres/32)+1];
	int numUsedSpheres = 0;

	Maths::HF32 zero(Maths::ZERO_INIT);
	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		bitField[numUsedSpheres/32] &= ~(1 << (numUsedSpheres % 32));
		spheres[numUsedSpheres++] = Maths::C_Sphere(m_PointList[a], epsilon);
	}

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		bitField[numUsedSpheres/32] &= ~(1 << (numUsedSpheres % 32));
		spheres[numUsedSpheres++] = m_SphereList[a];
	}

	numUsedSpheres = 0;

	bool contained = false;

	Maths::HF32 radius(zero);
	Maths::HF32 sumRadius(zero);
	Maths::HF32 one(1.f);
	while(!contained)
	{
		contained = true;
		Maths::HF32 maxProj(1e38f);
		int index = -1;
		for(int a = 0; a < numSpheres; ++a)
		{
			if(bitField[a/32] & (1 << (a % 32)))
				continue;
			Maths::HVec3 vdis(spheres[a].Centre().m_V - centre);
			Maths::HF32 distance(vdis.Length() + spheres[a].Radius());
			if(distance.GT(radius) && distance.LT(maxProj))
			{
				maxProj = distance;
				index = a;
				contained = false;
			}
		}
		if(index >= 0)
		{
			//We have a maximally-projected sphere which is outside of this bounding sphere
			//We need to work out a sphere which will contain this sphere, and the other spheres already contained
			//in the bounding sphere
			Maths::HF32 lRadius(spheres[index].Radius());
			sumRadius += lRadius;
			Maths::HF32 draw(lRadius/sumRadius); //A value in the range [0, 1], which dictates how much
															//this new sphere drags the centre of the sphere

			centre = draw * spheres[index].Centre() + (one - draw) * centre;
			bitField[index/32] |= 1<<(index % 32);
			usedSpheres[numUsedSpheres++] = &spheres[index];

			radius = zero;
			for(int a = 0; a < numUsedSpheres; ++a)
			{
				//Find the appropriate radius to enclose these used spheres...
				Maths::HVec3 vdis(usedSpheres[a]->Centre().m_V - centre);
				Maths::HF32 distance(vdis.Length() + usedSpheres[a]->Radius());
				radius = Maths::Max(radius, distance);
			}
		}
	}

	delete [] spheres;
	delete [] usedSpheres;
	delete [] bitField;

	CalculateBox();
	m_Sphere = Maths::C_Sphere(centre, radius);
	return m_Sphere;
}

const Maths::C_Sphere C_Bounds::CalculateSphere3()
{
	//if(m_SphereValid)
     //   return m_Sphere;

	//First stage - pick an appropriate centre of the sphere. To do this, let's 
	//sum up all the radii, and multiply the centre of each sphere by its radii. For points, lets
	//take a radius of size epsilon

	const Maths::HF32 epsilon(0.01f);

	Maths::HF32 sumRadii(Maths::ZERO_INIT);
	

	Maths::HVec3 centre(Maths::ZERO_INIT);

	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		centre += m_PointList[a] * epsilon;
		sumRadii += epsilon;
	}

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HF32 rad = m_SphereList[a].Radius();
		centre += m_SphereList[a].Centre() * rad;
		sumRadii += rad;
	}

	centre /= sumRadii;

	//We now have an approximate centre
	 F32 furthestDist = 0.0f;    

	S32 i;
	for(i=0; i<m_PointList.Size(); i++)
	{
		Maths::HVec3& currPoint = m_PointList[i];

		const F32 distToCentre = centre.Distance( currPoint );
		if( distToCentre > furthestDist)
		{
			furthestDist = distToCentre;
		}
	}

	for(i=0; i<m_SphereList.Size(); i++)
	{
        Maths::C_Sphere& currSphere = m_SphereList[i];
        const F32 distToCentre = centre.Distance(currSphere.Centre()) + currSphere.Radius();

        if( distToCentre > furthestDist)
        {
            furthestDist = distToCentre;
        }
	}

    m_Sphere = Maths::C_Sphere(centre, furthestDist);

    m_SphereValid = true;

    return m_Sphere;


}

const Maths::C_Sphere C_Bounds::CalculateSphere2()
{
	//if(m_SphereValid)
     //   return m_Sphere;

	Maths::HVec3 min(FLT_MAX), max(-FLT_MAX);
	Maths::HVec3 xmin, xmax, ymin, ymax, zmin, zmax;

	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		Maths::HVec3& currPoint = m_PointList[a];
		Maths::HF32 val(currPoint.X());
		if(val.LT(min.X()))
		{
			min.X(val);
			xmin = currPoint;
		}
		if(val.GT(max.X()))
		{
			max.X(val);
			xmax = currPoint;
		}
		val = currPoint.Y();
		if(val.LT(min.Y()))
		{
			min.Y(val);
			ymin = currPoint;
		}
		if(val.GT(max.Y()))
		{
			max.Y(val);
			ymax = currPoint;
		}
		val = currPoint.Z();
		if(val.LT(min.Z()))
		{
			min.Z(val);
			zmin = currPoint;
		}
		if(val.GT(max.Z()))
		{
			max.Z(val);
			zmax = currPoint;
		}
	}

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HVec3 currPoint(m_SphereList[a].Centre());
		Maths::HF32 rad(m_SphereList[a].Radius());
		Maths::HF32 val(currPoint.X() - rad);
		if(val.LT(min.X()))
		{
			min.X(val);
			xmin = currPoint;
		}
		val = currPoint.X() + rad;
		if(val.GT(max.X()))
		{
			max.X(val);
			xmax = currPoint;
		}
		val = currPoint.Y() - rad;
		if(val.LT(min.Y()))
		{
			min.Y(val);
			ymin = currPoint;
		}
		val = currPoint.Y() + rad;
		if(val.GT(max.Y()))
		{
			max.Y(val);
			ymax = currPoint;
		}
		val = currPoint.Z() - rad;
		if(val.LT(min.Z()))
		{
			min.Z(val);
			zmin = currPoint;
		}
		val = currPoint.Z() + rad;
		if(val.GT(max.Z()))
		{
			max.Z(val);
			zmax = currPoint;
		}
	}

	Maths::HVec3 vXSpan(xmax - xmin);
	Maths::HVec3 vYSpan(ymax - ymin);
	Maths::HVec3 vZSpan(zmax - zmin);
	Maths::HF32 xSpan = vXSpan | vXSpan;
	Maths::HF32 ySpan = vYSpan | vYSpan;
	Maths::HF32 zSpan = vZSpan | vZSpan;

	Maths::HVec3 vMin(xmin);
	Maths::HVec3 vMax(xmax);
	Maths::HF32 maxSpan(xSpan);
	if(ySpan.LT(xSpan) && ySpan.LT(zSpan))
	{
		vMin = ymin;
		vMax = ymax;
		maxSpan = ySpan;
	}
	else if(zSpan.LT(xSpan))
	{
		vMin = zmin;
		vMax = zmax;
		maxSpan = zSpan;
	}
	const Maths::HF32 half(0.5f);
	Maths::HVec3 centre(vMin + vMax);
	centre *= half;
	Maths::HVec3 vrad(vMax - centre);
	Maths::HF32 radsq(vrad | vrad);
	Maths::HF32 radius(Maths::Sqrt(radsq));

	//Finally, grow this base sphere to enclose all the points/spheres it should

	for(int a = 0; a < m_PointList.Size(); ++a)
	{
		Maths::HVec3& currPoint = m_PointList[a];
		Maths::HVec3 vdif(centre - currPoint);
		Maths::HF32 disSq(vdif | vdif);
		if(disSq > radsq)
		{
			radsq = disSq;
			Maths::HF32 dis(Maths::Sqrt(disSq));
			radius = dis;/*(radius + dis) * half;
			radsq = radius * radius;
			Maths::HF32 old_to_new(radius - dis);
			centre = (radius * centre + old_to_new * currPoint)/dis;*/
		}
	}

	for(int a = 0; a < m_SphereList.Size(); ++a)
	{
		Maths::HVec3 currPoint(m_SphereList[a].Centre());
		Maths::HF32 spRad(m_SphereList[a].Radius());
		Maths::HVec3 vdif(centre - currPoint);
		Maths::HF32 totalDis(vdif.Length());
		Maths::HVec3 supportPoint(vdif * ((totalDis + spRad)/totalDis));
		Maths::HF32 disSq(supportPoint | supportPoint);
		if(disSq > radsq) //Part of this sphere is outside the bounding sphere
		{
			Maths::HF32 dis(Maths::Sqrt(disSq));
			radius = dis;/*(radius + dis) * half;
			radsq = radius * radius;
			Maths::HF32 old_to_new(radius - dis);
			Maths::HVec3 distantPoint(supportPoint + centre);
			centre = (radius * centre + old_to_new * distantPoint)/dis;*/
		}
	}
	m_Sphere = Maths::C_Sphere(centre, radius);
	m_Box.Max(max);
	m_Box.Min(min);

	m_SphereValid = true;
	m_BoxValid = true;
	return m_Sphere;
}

const Maths::C_Sphere C_Bounds::CalculateSphere()
{
    //if(m_SphereValid)
    //    return m_Sphere;

    const Maths::C_AxisBox box = CalculateBox();

    Maths::HVec3 centre = box.Centre();
    F32 furthestDist = 0.0f;    

	S32 i;
	for(i=0; i<m_PointList.Size(); i++)
	{
		Maths::HVec3& currPoint = m_PointList[i];

		const F32 distToCentre = centre.Distance( currPoint );
		if( distToCentre > furthestDist)
		{
			furthestDist = distToCentre;
		}
	}

	for(i=0; i<m_SphereList.Size(); i++)
	{
        Maths::C_Sphere& currSphere = m_SphereList[i];
        const F32 distToCentre = centre.Distance(currSphere.Centre()) + currSphere.Radius();

        if( distToCentre > furthestDist)
        {
            furthestDist = distToCentre;
        }
	}

    m_Sphere = Maths::C_Sphere(centre, furthestDist);

    m_SphereValid = true;

    return m_Sphere;
}

const Maths::C_AxisBox C_Bounds::CalculateBox()
{
  //  if(m_BoxValid)
    //    return m_Box;

    if((m_PointList.Size() == 0) && (m_SphereList.Size() == 0))
    {
        m_Box.SetZeroExtents();
        return m_Box;
    }

    m_Box.SetInvalidExtents();

	S32 i;
	for(i=0; i<m_PointList.Size(); i++)
	{
		Maths::HVec3& currPoint = m_PointList[i];

        m_Box.UpdateExtents(currPoint);
    }

	for(i=0; i<m_SphereList.Size(); i++)
	{
        Maths::C_Sphere& currSphere = m_SphereList[i];
        m_Box.UpdateExtents(currSphere);
    }

    m_BoxValid = true;

    return m_Box;
}

} // namespace Bounds
