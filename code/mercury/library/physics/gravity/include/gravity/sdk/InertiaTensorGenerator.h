#ifndef INERTIA_TENSOR_GENERATOR_H
#define INERTIA_TENSOR_GENERATOR_H

#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/constants.h"

namespace Gravity
{

#ifndef PARALLEL_AXIS_INERTIA
	#define PARALLEL_AXIS_INERTIA
#endif


class C_InertiaTensorGenerator
{
public:

	static Maths::HMat33 ParallelAxis(const Maths::HMat33 PCREF inertia, const Maths::HVec3 PCREF displacement, const Maths::HF32 PCREF mass);

	static Maths::HMat33 SphereInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius);

	static Maths::HMat33 BoxInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF halfWidths);

	static Maths::HMat33 ConvexHullInertiaTensor(const Maths::HF32 PCREF mass, const SharedPhysics::SortedArray<Maths::HVec3>& points);

	static Maths::HMat33 CylinderInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height);

	static Maths::HMat33 ConeInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height);

	static Maths::HMat33 HemisphereInertiaTensor(const Maths::HF32 PCREF mass, const Maths::HF32 PCREF radius);

	static Maths::HMat33 CapsuleInertiaTensor(const Maths::HF32 PCREF body_Mass, const Maths::HF32 PCREF body_Height, const Maths::HF32 PCREF cap_Mass, const Maths::HF32 PCREF radius);

	static Maths::HMat33 CharacterInertiaTensor(const Maths::HF32 PCREF body_Mass, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height);

	static Maths::HMat33 ThinCircularDiskInertiaTensor(const Maths::HF32 PCREF body_Mass, const Maths::HF32 PCREF radius);

	static Maths::HMat33 SlenderRodInertiaTensor(const Maths::HF32 PCREF body_Mass, const Maths::HF32 PCREF height);

	static Maths::HMat33 InverseInertiaTensor( const Maths::HMat33 PCREF iBodyMatrix);

};

};

#endif

