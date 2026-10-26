#include "stdafx.h"
#include "gravity/sdk/InertiaTensorGenerator.h"



namespace Gravity
{
	Maths::HMat33  C_InertiaTensorGenerator::ParallelAxis(const Maths::HMat33 PCREF inertia, const Maths::HVec3 PCREF displacement, const Maths::HF32 PCREF mass)
	{

		 const Maths::HF32 Ax( displacement.X() );
		const Maths::HF32 Ay( displacement.Y() );
		const Maths::HF32 Az( displacement.Z() );
	    
		const Maths::HVec3 offset_it_x(  Ay * Ay + Az * Az,	-Ax * Ay,			-Ax * Az           );
		const Maths::HVec3 offset_it_y( -Ax * Ay,			Ax * Ax + Az * Az,	-Ay * Az           );
		const Maths::HVec3 offset_it_z( -Ax * Az,			-Ay * Az,			Ax * Ax + Ay * Ay );
	    
		return inertia + Maths::HMat33( offset_it_x*mass, offset_it_y*mass, offset_it_z*mass );

	}

	Maths::HMat33 C_InertiaTensorGenerator::SphereInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 two_fifths(0.2f);
		const Maths::HF32 inertia(mass * radius * radius * two_fifths);
		Maths::HMat33 matrix(Maths::UNDEFINED);
		matrix.AX(Maths::HVec3(inertia, zero, zero));
		matrix.AY(Maths::HVec3(zero, inertia, zero));
		matrix.AZ(Maths::HVec3(zero, zero, inertia));
		return matrix;
	}


	Maths::HMat33 C_InertiaTensorGenerator::BoxInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF halfWidths)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 twelfth(Maths::Constants<Maths::HF32>::Sixth() * Maths::Constants<Maths::HF32>::Half());
		const Maths::HVec3 widths(halfWidths * Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 x2(widths.X() * widths.X());
		const Maths::HF32 y2(widths.Y() * widths.Y());
		const Maths::HF32 z2(widths.Z() * widths.Z());
		const Maths::HF32 coefficient(twelfth * mass);
		Maths::HMat33 matrix(Maths::UNDEFINED);
		const Maths::HVec3 row1(coefficient * (y2 + z2), zero, zero);
		const Maths::HVec3 row2(zero, coefficient * (x2 + z2), zero);
		const Maths::HVec3 row3(zero, zero, coefficient * (x2 + y2));
		matrix.AX(row1);
		matrix.AY(row2);
		matrix.AZ(row3);
		return matrix;
	}

	Maths::HMat33 C_InertiaTensorGenerator::ConvexHullInertiaTensor(const Maths::HF32 PCREF mass, const SharedPhysics::SortedArray<Maths::HVec3>& points)
	{
		//return Maths::HMat33();
		//Simple version to begin with... calculate the axis box around this and then fall back on the box
		//inertia tensor instead

		Maths::HVec3 extents(Maths::Constants<Maths::HVec3>::Zero());

		for(S32 a = 0; a < points.m_numElements; ++a)
		{
			extents = Maths::Max(extents, Maths::Abs(points[a]));
		}
		return C_InertiaTensorGenerator::BoxInertiaTensor(mass, extents);
	}


	Maths::HMat33 C_InertiaTensorGenerator::CylinderInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height)
	{
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 half(Constants<HF32>::Half());
		const HF32 twelfth(Constants<HF32>::Sixth() * half);//1/12
		const HF32 sqHeight(height*height);
		const HF32 sqRadius(radius*radius);
		const HF32 c1(twelfth*mass);
		const HF32 c2(half*mass);
		const HF32 t1(Maths::Constants<HF32>::Three()*sqRadius + sqHeight);
		const HF32 x2(c1*t1); //x2=z2
		const HF32 y2(c2*sqRadius);

		HMat33 matrix(UNDEFINED);
		const HVec3 row1(x2, zero, zero);
		const HVec3 row2(zero, y2, zero);
		const HVec3 row3(zero, zero, x2);

		matrix.AX(row1);
		matrix.AY(row2);
		matrix.AZ(row3);
		return matrix;	

	}

	Maths::HMat33 C_InertiaTensorGenerator::ThinCircularDiskInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius)
	{
		//Thin Circular Dist
		using namespace Maths;
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const HF32 half(Maths::Constants<Maths::HF32>::Half());
		const HF32 quart(Maths::Constants<Maths::HF32>::Quarter());
		const HF32 sqRadius(radius*radius);
		const HF32 c1(quart*mass);
		const HF32 c2(half*mass);
		const HF32 x2(c1*sqRadius); //x2=z2
		const HF32 y2(c2*sqRadius);

		Maths::HMat33 matrix(Maths::UNDEFINED);
		const Maths::HVec3 row1(x2, zero, zero);
		const Maths::HVec3 row2(zero, y2, zero);
		const Maths::HVec3 row3(zero, zero, x2);

		matrix.AX(row1);
		matrix.AY(row2);
		matrix.AZ(row3);
		return matrix;	

	}

	Maths::HMat33 C_InertiaTensorGenerator::InverseInertiaTensor( const Maths::HMat33 PCREF iBodyMatrix)
	{
		/*const Maths::HVec3 rowX(iBodyMatrix.AX());
		const Maths::HVec3 rowY(iBodyMatrix.AY());
		const Maths::HVec3 rowZ(iBodyMatrix.AZ());
		
		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		const Maths::HF32 len(Maths::Min((rowX | rowX), Maths::Min((rowY | rowY), (rowZ | rowZ))));
		if(len < epsilon)
		{
			return Maths::HMat33(rowX.Reciprocal(), rowY.Reciprocal(), rowZ.Reciprocal());
		}*/
		return iBodyMatrix.Inverse();
	}

	Maths::HMat33 C_InertiaTensorGenerator::SlenderRodInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF height)
	{
		using namespace Maths;
		const HF32 epsilon(Constants<HF32>::Epsilon());
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 half(Constants<HF32>::Half());
		const HF32 twelfth(Constants<HF32>::Sixth() * half);//1/12
		const HF32 sqHeight(height*height);
		const HF32 c(twelfth*mass);
		const HF32 x2(c*sqHeight); //x2=z2

		HMat33 matrix(UNDEFINED);
		const HVec3 row1(x2, zero, zero);
		const HVec3 row2(zero, epsilon, zero);
		const HVec3 row3(zero, zero, x2);

		matrix.AX(row1);
		matrix.AY(row2);
		matrix.AZ(row3);
		return matrix;	
	}

	Maths::HMat33 C_InertiaTensorGenerator::CharacterInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		const Maths::HF32 c2(half*mass);
		const Maths::HF32 y2(c2*radius*radius);

		Maths::HMat33 matrix(Maths::UNDEFINED);
		const Maths::HVec3 row1(zero, zero, zero);
		const Maths::HVec3 row2(zero, y2, zero);
		const Maths::HVec3 row3(zero, zero, zero);

		matrix.AX(row1);
		matrix.AY(row2);
		matrix.AZ(row3);
		return matrix;	
	}


	Maths::HMat33 C_InertiaTensorGenerator::ConeInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 eightieth(SharedPhysics::Constants::Eightieth());//1/80
		const Maths::HF32 tenth(SharedPhysics::Constants::Tenth());//1/10
		//const Maths::HF32 eightieth(1.f/80.f);//1/80
		//const Maths::HF32 tenth(1.f/10.f);//1/10
		const Maths::HF32 three(Maths::Constants<Maths::HF32>::Three());
		const Maths::HF32 four(Maths::Constants<Maths::HF32>::Four());
		const Maths::HF32 c1(three*eightieth*mass);
		const Maths::HF32 c2(three*tenth*mass);
		const Maths::HF32 sqHeight(height*height);
		const Maths::HF32 sqRadius(radius*radius);
		const Maths::HF32 t(four*sqRadius + sqHeight);
		const Maths::HF32 x2(c1*t); // x2=z2
		const Maths::HF32 y2(c2*sqRadius);

		Maths::HMat33 matrix(Maths::UNDEFINED);
		const Maths::HVec3 row1(x2, zero, zero);
		const Maths::HVec3 row2(zero, y2, zero);
		const Maths::HVec3 row3(zero, zero, x2);

		matrix.AX(row1);
		matrix.AY(row2);
		matrix.AZ(row3);
		return matrix;

		//return BoxInertiaTensor(mass, Maths::HVec3(radius, height * 0.5f, radius));

	}

	Maths::HMat33 C_InertiaTensorGenerator::HemisphereInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 c1(83.f / 320.f);
		const Maths::HF32 c2(0.2f);
		const Maths::HF32 sqRadius(radius*radius);
		const Maths::HF32 t(mass*sqRadius);
		const Maths::HF32 x2(c1*t);
		const Maths::HF32 y2(c2*t);

		Maths::HMat33 matrix(Maths::UNDEFINED);
		const Maths::HVec3 row1(x2, zero, zero);
		const Maths::HVec3 row2(zero, y2, zero);
		const Maths::HVec3 row3(zero, zero, x2);

		matrix.AX(row1);
		matrix.AY(row2);
		matrix.AZ(row3);
		return matrix;	
	}


	/*
		body_Height is the total length of vector from the start to the end point in the capsule
	*/
	Maths::HMat33 C_InertiaTensorGenerator::CapsuleInertiaTensor(const Maths::HF32 PCREF body_Mass, const Maths::HF32 PCREF body_Height, const Maths::HF32 PCREF cap_Mass, const Maths::HF32 PCREF radius)
	{
		const Maths::HMat33 cylinderInertia=CylinderInertiaTensor(body_Mass, radius, body_Height);
		const Maths::HMat33 hemisphereInertia=HemisphereInertiaTensor(cap_Mass, radius);

#if 0 //def PARALLEL_AXIS_INERTIA
		const Maths::HF32 halfHeight(body_Height*Maths::Constants<Maths::HF32>::Half());
		const Maths::HF32 threeEighth(3.f/8.f);
		const Maths::HF32 dis(halfHeight + threeEighth);
		const Maths::HMat33 hemisphere1=ParallelAxis(hemisphereInertia, Maths::HVec3(0.f, dis, 0.f), body_Mass);
		const Maths::HMat33 hemisphere2=ParallelAxis(hemisphereInertia, Maths::HVec3(0.f, -dis, 0.f), body_Mass);
		const Maths::HMat33 matrix=cylinderInertia+hemisphere1+hemisphere2;
#else
		const Maths::HMat33 matrix=cylinderInertia+hemisphereInertia*Maths::Constants<Maths::HF32>::Two();
#endif

		//Cylinder and hemisphere declared in terms of y-axis being medial axis. However, we always use
		//x-axis as medial axis

		return matrix;
	}
};
