#include "stdafx.h"
#include <animation/pose/Pose.h>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/PoseMapping.h>
#include <animation/pose/PoseLayout.h>
#include "pose/Pose.inl"
#include <animation/pose/PoseFields.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/common/Stats.h>
#include <animation/skeleton/Skeleton.h>
#include "BlendFunctions.h"
#include "BlendFunctions.inl"
#include <core/threads/Atomic.h>
#include <system/Perf.h>

/**

@mainpage Mercury %Animation

@section intro Introduction

The Mercury %Animation library provides a means of evaluating and blending
time-varying data. This data includes joint (bone) transforms, 1-d scalar curves and 3-d curves.
Blending of animated data can be performed using accumulative, linear or
additive modes.

The animation library sources its data from two types of resources:
@link Animation::C_Cycle cycles @endlink and @link Animation::C_Skeleton skeletons @endlink.
Cycles contain animated data over a time period, stored in a compressed format suitable
for efficient evaluation. Skeletons represent a hierarchy of joints or objects that can be
used to interpret animated transforms stored in local (parent-relative) space. Skeletons
also store information about @em constraints that are used to procedurally calculate
or modify the transforms of certain joints.

Mercury %Animation currently consists of two libraries: runtime and tools. The tools library is a superset of the runtime library
and contains code for exporting cycles and skeletons into formats suitable for all supported
target platforms (Xbox 360, PS3 and PC). To build and link with the tools library, ensure
that the host application is built for the @em WIN32_TOOL target.

The Animation and  AnimationTools namespaces are used to separate the runtime code
from the tools code. Note that it is not intended for any code in the
AnimationTools namespace to be used in a game.

For a brief overview of how to use the runtime library, see @ref page1.
*/

/**
@page page1 Mercury Animation Overview

@section poses Poses and Fields

Animation data state is managed by the Animation::C_Pose class, so-called as it typically
represents the current pose of an animated character.
Data is divided into arrays of @em fields of the same type. For a list of field types,
see the internal @link Animation::E_PoseFieldType E_PoseFieldType @endlink enumeration.
Every field in a pose has a unique name so that the indices of individual fields can be
looked up at runtime. 

C_Pose provides a number of methods for accessing fields by type, such as 
@link Animation::C_Pose::GetJointTransform() GetJointTransform() @endlink and 
@link Animation::C_Pose::GetCustomScalar() GetCustomScalar() @endlink. The
field values returned by these methods can be used directly (e.g. for performing blending)
or implicitly converted into appropriate data types, as follows:

	- Animation::C_ScalarField converts to an F32.
	- Animation::C_Vector3Field converts to a Maths::Vector3.
	- Animation::C_QuaternionField converts to a Maths::Quaternion.

The pose for a character with a skeleton will contain an array of
@link Animation::C_JointTransform C_JointTransform @endlink fields which hold the transform of each of the character's joints
at the current point in time. The elements of this array can be accessed using
the @link Animation::C_Pose::GetJointTransform() C_Pose::GetJointTransform() @endlink method.

@note It is always assumed that the joint transform array uses exactly the same order of joints as the single skeleton associated with the pose.

There can a number of different field types in a pose and no restriction that any particular type must
be present. As such, a pose can be used to gather only the types of animation data that
the user is interested in.

Other than joint transforms, one commonly used type of field is the <em>Custom Scalar</em> which represents a value
from a scalar (one dimensional) curve. These "custom" curves have no particular meaning to the animation system and can be used for any purpose. 
The animation exporter currently converts custom attributes associated with joint nodes into
custom scalar curves, but this could be extended in the future to include data such as per object visibility
flags or material properties.

The Animation::C_PoseFormat class is used to describe the fields required when creating a pose.
For example, to create a pose for an animated character with a skeleton:

@code
// Create a pose for mySkeleton.
myPose.Create( Animation::C_PoseFormat( *mySkeleton ) );
@endcode

To add additional custom scalar fields for left and right foot down event curves:

@code
// Create a pose for mySkeleton with two additional scalar fields.
Animation::C_PoseFormat poseFormat( *mySkeleton );

U32 leftFootDownIndex  = poseFormat.AddCustomScalar( "game_root.LEFT_FOOT_DOWN" );
U32 rightFootDownIndex = poseFormat.AddCustomScalar( "game_root.RIGHT_FOOT_DOWN" );

myPose.Create( poseFormat );

// ... evaluate an animation into myPose ...

// Query the current state of the left foot down curve.
F32 leftFootDown = myPose.GetCustomScalar( leftFootDownIndex );
// Now query the state of the right foot down curve.
F32 rightFootDown = myPose.GetCustomScalar( rightFootDownIndex );
@endcode

@section skeletons Skeletons

The Animation::C_Skeleton class describes the joint hierarchy comprising a character's skeleton together with a default state
for each joint, known as the bind, base or reference pose. It may also hold a list of joint constraints 
to be applied to a pose after animation blending.

@subsection jointconstraints Joint Constraints

Joint constraints are rules that modify joint transforms after blending, for example to calculate the orientations
of intermediate helper joints that improve the deformation of a character's mesh during skinning. Joint constraints
are either static (stateless) or dynamic (stateful). Dynamic constraints are typically used to provide simple physical
secondary animation effects such as swinging objects.
For further details on the types of constraints that are available and how they work,
see @ref StaticJointConstraints and @ref DynamicJointConstraints.

To apply joint constraints to a pose, you will need to create an Animation::C_SkeletonInstance object for each
character and call Animation::C_SkeletonInstance::ApplyJointConstraints() after animation blending has been completed.

@subsection skeletonpose Working In Model Space and Local Space

The Animation::C_SkeletonPose class is used to provide a temporary interface for working
with a pose in both model (skeleton) space and local space, assuming that the pose contains local space joint transforms.
The C_Pose class has no knowledge of what space its joint transforms are in, so it is 
entirely possible to use world space transforms if desired.

@section cycles Cycles

Cycles contain a collection of curves representing the state of various fields over time. Since fields are matched
by name, cycles can be applied to arbitrary poses, allowing a single cycle to be used with many different skeletons.

An Animation::C_CycleInstance object needs to be created for each instance of a C_Cycle 
to be evaluated in a C_Pose. On construction, C_CycleInstance automatically maps the fields
stored in a cycle onto the corresponding fields of the given C_Pose. 

To evaluate a cycle into a C_Pose, use the Animation::C_Cycle::Evaluate() function, 
specifying a blend mode (see Animation::E_BlendMode), animation time (in seconds), C_CycleInstance,
skeleton, blend weight and the destination C_Pose.

<hr>

Back to @ref intro "Introduction".

*/

namespace Animation
{

C_Pose::C_ElementInfo C_Pose::s_elementInfo[ E_PoseFieldElement_Count ] =
{
	{ E_PoseFieldType_JointTransform,			0	},
	{ E_PoseFieldType_JointTransform,			16	},
	{ E_PoseFieldType_JointTransform,			32	},
	{ E_PoseFieldType_CustomScalar,				0	},
	{ E_PoseFieldType_CustomVector3,			0	},
	{ E_PoseFieldType_TrajectoryDeltaTransform,	0	},
	{ E_PoseFieldType_TrajectoryDeltaTransform,	16	},
	{ E_PoseFieldType_TrajectoryTransform,		0   },
	{ E_PoseFieldType_TrajectoryTransform,		16  },
	{ E_PoseFieldType_CustomVector4,			0   },
	{ E_PoseFieldType_CustomVector4,			16  }
};

C_Pose::C_Pose():
	m_poseLayout( 0 ),
	m_readerCount( 0 ),
	m_writerCount( 0 )
{
}

#ifndef PLATFORM_SPU

C_Pose::C_Pose( const C_PoseFormat& poseFormat ):
	m_poseLayout( 0 ),
	m_readerCount( 0 ),
	m_writerCount( 0 )
{
	Create( poseFormat );
}

#endif

C_Pose::~C_Pose()
{
	BIZ_STATIC_ASSERT((sizeof(s_elementInfo)/sizeof(s_elementInfo[0])) == E_PoseFieldElement_Count);
	
	BIZ_ASSERT_MSG(m_readerCount == 0 && m_writerCount == 0, "Pose deleted whilst other threads may be accessing it");

	Destroy();
}

#ifndef PLATFORM_SPU

void C_Pose::Create( const C_PoseFormat& poseFormat )
{
	const C_PoseLayout* poseLayout = C_PoseLayout::CreatePoseLayout( poseFormat );
	
	U32 newDataSize = 0;
	U32 numScalarFields = 0;
	
	if ( poseLayout )
	{
		poseLayout->AddRef();
		newDataSize = LayoutMemory(*poseLayout);
		numScalarFields = poseLayout->GetFieldCount(Animation::E_PoseFieldType_CustomScalar);
	}

	// Unreference the existing PoseLayout.
	if (m_poseLayout)
	{
		bool deleted = m_poseLayout->DecRef();
		// sanity check to make sure that the pose layout we're about to use hasn't just been deleted.
		BIZ_ASSERT(!(deleted && m_poseLayout == poseLayout));
	}

#ifndef PLATFORM_SPU
	if (m_data.IsValid())
	{
		ANIMATION_STATS_UPDATE(memoryUsedByPoseData -= m_data.GetSize());
		ANIMATION_STATS_UPDATE(numPoses--);
	}
#endif

	m_data.Create(newDataSize);

	if (numScalarFields > 0)
	{
		// Set scalar field defaults to zero.
		F32* pDefaultFields = m_scalarDefaults(m_data);
		for (U32 i = 0; i < numScalarFields; ++i)
		{
			pDefaultFields[i] = 0.f;
		}
	}

#ifndef PLATFORM_SPU
	{
		ANIMATION_STATS_UPDATE(numPoses++);
		ANIMATION_STATS_UPDATE(memoryUsedByPoseData += m_data.GetSize());
	}
#endif

	m_poseLayout = poseLayout;
	m_readerCount = 0;
	m_writerCount = 0;

}

#endif

/**
The memory pointed to by @a data must be at least @a poseLayout->GetPoseDataSize() bytes in size
and be 16-bytes aligned. It is assumed that the existing pose is either uninitialised or was
previously initialised with CreateInPlace().

Note that @a poseLayout's reference count is left unchanged since this method is intended
for creating temporary poses on the stack.

@see C_PoseStack
*/
void C_Pose::CreateInPlace( const C_PoseLayout& poseLayout, U8* data )
{
	if (m_data.IsValid())
	{
		BIZ_ASSERT(m_data.IsInPlace());
	}
	m_poseLayout = &poseLayout;
	U32 dataSize = LayoutMemory(poseLayout);
	m_data.CreateInPlace(data, dataSize);
	BIZ_ASSERT( ( (U32) &m_data[0] & 0xf ) == 0 && "Pose data must be 16-byte aligned" );
}

void C_Pose::Destroy()
{
#ifndef PLATFORM_SPU
	if (m_data.IsValid() && !m_data.IsInPlace())
	{
		ANIMATION_STATS_UPDATE(numPoses--);
		ANIMATION_STATS_UPDATE(memoryUsedByPoseData -= m_data.GetSize());
	}

	if ( !m_data.IsInPlace() && m_poseLayout )
	{
		m_poseLayout->DecRef();
	}
#endif

	m_data.Destroy();
	m_poseLayout = 0;
}

bool C_Pose::IsValid() const
{
	return m_poseLayout != NULL;
}

void C_Pose::SpuFetchAsync( U32 tag )
{
#ifdef PLATFORM_SPU

	BIZ_ASSERT(m_poseLayout.EA != NULL);

	// Fetch pose layout and field data.
	// NB. The pose layout does not include the field names, so GetFieldName and FindFieldByName functions
	// are not supported on SPU.
	Spu::FetchAllocAsync< const C_PoseLayout >( m_poseLayout, sizeof( C_PoseLayout ), tag );
	m_data.SpuFetchAsync(tag);
#endif
}

void C_Pose::SpuFetch()
{
	SpuFetchAsync( 0 );
	Spu::Wait( 0 );
}

void C_Pose::SpuStore()
{
#ifdef PLATFORM_SPU
	// Write back field data.
	SpuStoreAsync(0);
	Spu::Wait(0);
#endif
}

void C_Pose::SpuStoreAsync(U32 tag)
{
#ifdef PLATFORM_SPU
	// Write back field data.
	m_data.SpuStoreAsync(tag);
#endif
}

void C_Pose::AcquireReader(C_Pose* ppuPose)
{
	MT::AtomicIncrement(&ppuPose->m_readerCount);
	MT::MemoryFence();
}

void C_Pose::ReleaseReader(C_Pose* ppuPose)
{
	MT::MemoryFence();
	MT::AtomicDecrement(&ppuPose->m_readerCount);
}

void C_Pose::AcquireWriter(C_Pose* ppuPose)
{
	MT::AtomicIncrement(&ppuPose->m_writerCount);
	MT::MemoryFence();
}

void C_Pose::ReleaseWriter(C_Pose* ppuPose)
{
	MT::MemoryFence();
	MT::AtomicDecrement(&ppuPose->m_writerCount);
}

bool C_Pose::IsReadable(const C_Pose* ppuPose)
{
	S32 writerCount = MT::AtomicGet((volatile S32*)&ppuPose->m_writerCount);
	if (writerCount == 0)
	{
		MT::MemoryFence();
		return true;
	}
	return false;
}

bool C_Pose::IsWriteable(const C_Pose* ppuPose)
{
	S32 readerCount = MT::AtomicGet((volatile S32*)&ppuPose->m_readerCount);
	if (readerCount != 0)
	{
		return false;
	}
	S32 writerCount = MT::AtomicGet((volatile S32*)&ppuPose->m_writerCount);
	if (writerCount != 0)
	{
		return false;
	}
	MT::MemoryFence();
	return true;
}

namespace
{
	void ZeroFields( void* dest, U32 count, U32 fieldSize )
	{
		{
		#if PLATFORM_PS3 || PLATFORM_SPU
			__builtin_memset(
		#else
			memset(
		#endif
				dest,
				0,
				fieldSize * count
			);
		}
	}
	
	// Normalise an array of Vec4F32 fields by dividing their XYZ values by their weights.
	// TODO: blend in zero according to defaultThreshold.
	template <typename T>
	void NormaliseVec4s( T* fields, U32 count )
	{
//		BIZ_STATIC_ASSERT( 0 == ( sizeof(T) == sizeof(Maths::Vec4F32) );
		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		Maths::Vec4F32* vec4s = reinterpret_cast<Maths::Vec4F32*>(&fields[ 0 ]);
		for ( U32 i = 0; i < count; ++i )
		{
			const Maths::Vec4F32 scalarWithWeight = vec4s[ i ];
			const Maths::VF32 weight = Maths::W( scalarWithWeight );
			const Maths::VF32 invWeight = Maths::Select( weight > zero, one / weight, one );
			vec4s[ i ] = scalarWithWeight * invWeight;
		}
	}

	Maths::Vector4 kNormalisationConstants = Maths::MakeVector4(
		1024.f,				// 1/threshold
		1024.f * 1024.f,	// 1/threshold^2
		0.f,
		0.f);

	void NormaliseScalars(C_ScalarField* pFields, U32 count, const F32* pDefaults)
	{
		const Maths::Vector4 constants = kNormalisationConstants;
		
		const Maths::VF32 oneOverDefaultThreshold = Maths::X(constants);
		const Maths::VF32 oneOverDefaultThresholdSquared = Maths::Y(constants);

		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		Maths::Vec4F32* pVec4s = reinterpret_cast<Maths::Vec4F32*>(&pFields[0]);

		for ( U32 i = 0; i < count; ++i )
		{
			Maths::Vec4F32 scalarWithWeight = pVec4s[i];
			Maths::VF32 weight = Maths::W(scalarWithWeight);
			Maths::VF32 d = Maths::MakeVF32(&pDefaults[i]);
			BIZ_ASSERT(Maths::Finite(d));
			Maths::Vec4F32 scalarDefault = Maths::MakeVec4F32(d, d, d, one);
			Maths::Vec4F32 scalarNormalised = scalarWithWeight * Maths::Rcp(weight);
			Maths::VF32 defaultBlend = Maths::Max(one - (weight * oneOverDefaultThreshold), zero);
			Maths::Vec4F32 result = Maths::Lerp(scalarNormalised, scalarDefault, defaultBlend);
			BIZ_ASSERT(Maths::Finite(result));
			pVec4s[i] = result;
		}
	}

	void SetFieldsToIdentity( C_JointTransform* fields, U32 count )
	{
		for ( U32 i = 0; i < count; ++i )
		{
			fields[i].SetIdentity();
		}
	}
	
	inline void BlendFields( C_JointTransform* dst, const C_JointTransform* src, U32 count, E_BlendMode blendMode, const Maths::VF32& weight )
	{
		BlendJointTransformArray( blendMode, dst, src, count, weight );
	}

	inline void BlendFields( C_Vector3Field* dst, const C_Vector3Field* src, U32 count, E_BlendMode blendMode, const Maths::VF32& weight )
	{
		BlendVector3Array( blendMode, dst, src, count, weight );
	}

	inline void BlendFields( C_Vector4Field* dst, const C_Vector4Field* src, U32 count, E_BlendMode blendMode, const Maths::VF32& weight )
	{
		BlendVector4Array( blendMode, dst, src, count, weight );
	}

	inline void BlendFields( C_ScalarField* dst, const C_ScalarField* src, U32 count, E_BlendMode blendMode, const Maths::VF32& weight )
	{
		BlendScalarArray( blendMode, dst, src, count, weight );
	}
}

void C_Pose::Clear()
{
	if (!m_data.IsValid()) return;
	
	// Zero pose data up to defaults.
	ZeroFields(&m_data[0], 1, m_scalarDefaults.GetOffset());
}

void C_Pose::ClearToBasePose( const C_Skeleton* skeleton )
{
	if (skeleton != NULL)
	{
		const U32 numJoints = Maths::Min(GetNumJointTransforms(), skeleton->GetNumJoints());
		C_JointTransform* localJointTransforms = &GetJointTransform(0);
		for ( U32 i = 0; i < numJoints; ++i )
		{
			localJointTransforms[i] = skeleton->GetLocalBasePoseJointTransform(i);
		}
		
		// TODO: what about all the other fields?
	}
	else
	{
		Clear();
	}
}

void C_Pose::Normalise( const C_Skeleton* basePose )
{
	if (!m_data.IsValid()) return;
	
	if ( basePose != NULL )
	{
		// Normalise joint transforms using base pose.
		const U32 numJointTransforms = GetNumJointTransforms();
		const U32 numSkeletonJointTransforms = Maths::Min(basePose->GetNumJoints(), numJointTransforms);
		
		C_JointTransform::Normalise(&GetJointTransform(0), numSkeletonJointTransforms, basePose->GetLocalBasePoseJointTransformArray());
		
		if (numJointTransforms > numSkeletonJointTransforms)
		{
			C_JointTransform::Normalise(&GetJointTransform(numSkeletonJointTransforms), numJointTransforms - numSkeletonJointTransforms);
		}
	}
	else
	{
		// Normalise all transforms without base pose.
		C_JointTransform::Normalise(&GetJointTransform(0), GetNumJointTransforms());
	}
		
	NormaliseScalars(&GetCustomScalar(0), GetNumCustomScalars(), m_scalarDefaults(m_data));
	NormaliseVec4s(&GetCustomVector3(0), GetNumCustomVector3s());
	NormaliseVec4s(&GetCustomVector4(0), GetNumCustomVector4s() * 2);
	
	C_JointTransform::Normalise(&GetTrajectoryDeltaTransform(), GetFieldCount(E_PoseFieldType_TrajectoryDeltaTransform), false);
	C_JointTransform::Normalise(&GetTrajectoryTransform(), GetFieldCount(E_PoseFieldType_TrajectoryTransform), false);
}

void C_Pose::Normalise( const C_Pose& defaultPose )
{
	if (!m_data.IsValid()) return;
	
	BIZ_ASSERT(defaultPose.GetPoseLayout() == GetPoseLayout());
	
	const F32 kDefaultThreshold = 1.f / 1024.f;
	const F32 kOneOverDefaultThreshold = 1.f / kDefaultThreshold;
	
	const Maths::VF32 oneOverDefaultThreshold = Maths::MakeVF32( kOneOverDefaultThreshold );
	const Maths::VF32 oneOverDefaultThresholdSquared = oneOverDefaultThreshold * oneOverDefaultThreshold;

	C_JointTransform::Normalise( &GetJointTransform(0), GetNumJointTransforms(), &defaultPose.GetJointTransform(0) );
		
	// TODO: Custom curves use values from defaultPose.
	NormaliseScalars(&GetCustomScalar(0), GetNumCustomScalars(), m_scalarDefaults(m_data));
	NormaliseVec4s(&GetCustomVector3(0), GetNumCustomVector3s());
	NormaliseVec4s(&GetCustomVector4(0), GetNumCustomVector4s() * 2);
	
	// Trajectory transforms always normalise to identity.
	C_JointTransform::Normalise(&GetTrajectoryDeltaTransform(), GetFieldCount(E_PoseFieldType_TrajectoryDeltaTransform));
	C_JointTransform::Normalise(&GetTrajectoryTransform(), GetFieldCount(E_PoseFieldType_TrajectoryTransform));
}

void C_Pose::BlendInto( C_Pose& dest, E_BlendMode blendMode, F32 weight ) const
{
	BIZ_ASSERT( &dest != this ); // attempt to blend into itself!
	BIZ_ASSERT( dest.GetPoseLayout() == GetPoseLayout() ); // must have same pose layout
		
	if ( dest.GetPoseLayout() != GetPoseLayout() || &dest == this || !m_data.IsValid() )
	{
		return;
	}
	
	Maths::VF32 vWeight = Maths::MakeVF32( weight );
	
	BlendFields( &dest.GetJointTransform(0), &GetJointTransform(0), GetNumJointTransforms(), blendMode, vWeight );
	BlendFields( &dest.GetCustomScalar(0), &GetCustomScalar(0), GetNumCustomScalars(), blendMode, vWeight );
	BlendFields( &dest.GetCustomVector4(0), &GetCustomVector4(0), GetNumCustomVector4s(), blendMode, vWeight );
	BlendFields( &dest.GetCustomVector3(0), &GetCustomVector3(0), GetNumCustomVector3s(), blendMode, vWeight );
	BlendFields( &dest.GetTrajectoryDeltaTransform(), &GetTrajectoryDeltaTransform(), GetFieldCount( E_PoseFieldType_TrajectoryDeltaTransform ), blendMode, vWeight );
	BlendFields( &dest.GetTrajectoryTransform(), &GetTrajectoryTransform(), GetFieldCount( E_PoseFieldType_TrajectoryTransform ), blendMode, vWeight );
}

void C_Pose::CopyFrom( const C_Skeleton* destSkeleton, const C_Pose& sourcePose, const C_Skeleton* sourceSkeleton, const C_PoseMapping& poseMapping, bool ignoreScale )
{
	for (U32 fieldTypeIndex = 0; fieldTypeIndex < Animation::E_PoseFieldType_Count; ++fieldTypeIndex)
	{
		Animation::E_PoseFieldType fieldType = static_cast<Animation::E_PoseFieldType>(fieldTypeIndex);
		
		if (fieldType == E_PoseFieldType_JointTransform && sourceSkeleton != NULL)
		{
			C_JointTransform* __restrict destTransforms = &GetJointTransform(0);
			
			// Copy in source model transforms and convert them into local space.
			const U32 destTransformCount = GetNumJointTransforms();
			
			for (U32 destTransformIndex = 0; destTransformIndex < destTransformCount; ++destTransformIndex)
			{
				S32 sourceIndex = poseMapping.GetSourceFieldIndex(E_PoseFieldType_JointTransform, destTransformIndex);
				
				if (sourceIndex >= 0)
				{
					// Get source joint transforms in model space.
					// If performance is an issue, evaluating the source model transforms into a temporary buffer would speed this up.
					C_JointTransform transform = sourcePose.GetModelJointTransform(sourceIndex, sourceSkeleton);

					if (ignoreScale)
					{
						transform.GetScaleField().SetOne();
					}
				
					// Convert into local space.
					S32 parentIndex = destSkeleton->GetJointParentIndex(destTransformIndex);
					if (parentIndex >= 0)
					{
						transform *= GetModelJointTransform(parentIndex, destSkeleton).Inverse();
					}

					destTransforms[destTransformIndex] = transform;
				}
			}
		}
		else
		{
			//
			// Copy non-transform fields verbatim.
			//
			const U32 fieldSize = GetFieldSize(fieldType);
			
			for (U32 j = 0, count = GetFieldCount(fieldType); j < count; ++j)
			{
				S32 sourceIndex = poseMapping.GetSourceFieldIndex(fieldType, j);
				
				if (sourceIndex >= 0)
				{
					const void* srcFieldPtr = sourcePose.GetField(fieldType, sourceIndex);
					void* destFieldPtr = GetField(fieldType, j);
					
					memcpy(destFieldPtr, srcFieldPtr, fieldSize);
				}
			}
		}
	}
}

void C_Pose::CopyDefaultsFrom(const C_Pose& sourcePose)
{
	BIZ_ASSERT(GetPoseDataSize() == sourcePose.GetPoseDataSize());

	U32 defaultSize = m_data.GetSize() - m_scalarDefaults.GetOffset();
	if (defaultSize > 0)
	{
		memcpy(m_scalarDefaults(m_data), sourcePose.m_scalarDefaults(sourcePose.m_data), defaultSize);
	}
}


//
// Joint transform fields.
//

U32 C_Pose::GetNumJointTransforms() const
{
	return GetFieldCount( E_PoseFieldType_JointTransform );
}


#ifndef PLATFORM_SPU
const C_StringId& C_Pose::GetJointTransformName( U32 fieldIndex ) const
{
	return GetFieldName( E_PoseFieldType_JointTransform, fieldIndex );
}

S32 C_Pose::FindJointTransformByName( const C_StringId& name ) const
{
	return FindFieldByName( E_PoseFieldType_JointTransform, name );
}

S32 C_Pose::FindJointTransformByName( const CHAR8* name ) const
{
	return FindFieldByName( E_PoseFieldType_JointTransform, C_StringId( name, false ) );
}


#endif

const C_JointTransform& C_Pose::GetJointTransform( U32 index ) const
{
	return *static_cast<const C_JointTransform*>(GetField( E_PoseFieldType_JointTransform, index));
}

C_JointTransform& C_Pose::GetJointTransform( U32 index )
{
	return *static_cast<C_JointTransform*>(GetField( E_PoseFieldType_JointTransform, index));
}

C_JointTransform C_Pose::GetModelJointTransform( U32 index, const C_Skeleton* skeleton ) const
{
	bool useSkeleton = false;
	if (skeleton != NULL)
	{
		if (index < skeleton->GetNumJoints())
		{
			useSkeleton = true;
		}
	}
		
	BIZ_ASSERT(index < GetNumJointTransforms());
	
	if (useSkeleton)
	{
		const C_JointTransform* localJointTransforms = static_cast<const C_JointTransform*>(GetFirstField(E_PoseFieldType_JointTransform));

		S32 parentIndex = skeleton->GetJointParentIndex(index);
		C_JointTransform transform = localJointTransforms[index];
		
		Maths::Quaternion q = transform.GetRotation();
		Maths::Vector3 t = transform.GetTranslation();
		Maths::Vector3 s = transform.GetScale();
		
		while ( parentIndex >= 0 )
		{
			C_JointTransform rhs = localJointTransforms[parentIndex];
			const Maths::Quaternion qRhs = rhs.GetRotation();
			const Maths::Vector3 tRhs = rhs.GetTranslation();
			const Maths::Vector3 sRhs = rhs.GetScale();
			
			q = q * qRhs;
			t = t * qRhs + tRhs;
			s = s * sRhs;

			parentIndex = skeleton->GetJointParentIndex(parentIndex);
		}
		
		return C_JointTransform( q, t, s );		
	}
	else
	{
		return GetJointTransform(index);
	}
}

void C_Pose::SetModelJointTransform( U32 index, const C_JointTransform& transform, const C_Skeleton* skeleton )
{
	bool useSkeleton = false;
	if (skeleton != NULL)
	{
		if (index < skeleton->GetNumJoints())
		{
			useSkeleton = true;
		}
	}
		
	BIZ_ASSERT(index < GetNumJointTransforms());
	
	if (useSkeleton)
	{
		C_JointTransform parentModelTransform(C_JointTransform::IDENTITY);
		
		S32 parentIndex = skeleton->GetJointParentIndex(index);
		if (parentIndex >= 0)
		{
			parentModelTransform = GetModelJointTransform(parentIndex, skeleton);
		}
		
		C_JointTransform& localTransform = GetJointTransform(index);
		localTransform = transform * parentModelTransform.Inverse();
	}
	else
	{
		GetJointTransform(index) = transform;
	}
}

#if 0
void C_Pose::ConvertToModelSpace(const C_Skeleton* skeleton)
{
	if (!skeleton)
	{
		// No skeleton means that all transforms are in world space already.
		return;
	}
	U32 numJointTransforms = GetNumJointTransforms();
	if (numJointTransforms == 0)
	{
		return;
	}
	
	C_JointTransform* pTransforms = &GetJointTransform(0);

	C_JointTransform identity(C_JointTransform::IDENTITY);

	for (U32 i = 0; i < numJointTransforms; ++i)
	{
		C_JointTransform parentTransform = identity;
		S32 parentIndex = skeleton->GetJointParentIndex(i);
		if (parentIndex >= 0)
		{
			parentTransform = pTransforms[parentIndex];
		}
		C_JointTransform modelTransform = pTransforms[i] * parentTransform;
		pTransforms[i] = modelTransform;
	}
	// set a flag indicating that these transforms have been converted into model space?
}

void C_Pose::ConvertToLocalSpace(const C_Skeleton* skeleton)
{
	if (!skeleton)
	{
		// No skeleton means that local space is the same as world space.
		return;
	}

	U32 numJointTransforms = GetNumJointTransforms();
	if (numJointTransforms == 0)
	{
		return;
	}
	
	C_JointTransform* pTransforms = &GetJointTransform(0);

	C_JointTransform identity(C_JointTransform::IDENTITY);

	for (U32 i = numJointTransforms; i > 0; ++i)
	{
		C_JointTransform invParentTransform = identity;
		S32 parentIndex = skeleton->GetJointParentIndex(i-1);
		if (parentIndex >= 0)
		{
			invParentTransform = pTransforms[parentIndex].Inverse();
		}
		C_JointTransform localTransform = pTransforms[i-1] * invParentTransform;
		pTransforms[i-1] = localTransform;
	}
	// set a flag indicating that these transforms have been converted into local space?
}
#endif

//
// Trajectory transform fields.
//

bool C_Pose::HasTrajectoryTransform() const
{
	return GetFieldCount( E_PoseFieldType_TrajectoryTransform ) != 0;
}

const C_JointTransform& C_Pose::GetTrajectoryTransform() const
{
	return *static_cast<const C_JointTransform*>(GetField( E_PoseFieldType_TrajectoryTransform, 0 ));
}

C_JointTransform& C_Pose::GetTrajectoryTransform()
{
	return *static_cast<C_JointTransform*>(GetField( E_PoseFieldType_TrajectoryTransform, 0 ));
}

bool C_Pose::HasTrajectoryDeltaTransform() const
{
	return GetFieldCount( E_PoseFieldType_TrajectoryDeltaTransform ) != 0;
}

const C_JointTransform& C_Pose::GetTrajectoryDeltaTransform() const
{
	return *static_cast<const C_JointTransform*>(GetField( E_PoseFieldType_TrajectoryDeltaTransform, 0 ));
}

C_JointTransform& C_Pose::GetTrajectoryDeltaTransform()
{
	return *static_cast<C_JointTransform*>(GetField( E_PoseFieldType_TrajectoryDeltaTransform, 0 ));
}

//
// Custom scalar fields.
//

U32 C_Pose::GetNumCustomScalars() const
{
	return GetFieldCount( E_PoseFieldType_CustomScalar );
}

#ifndef PLATFORM_SPU
const C_StringId& C_Pose::GetCustomScalarName( U32 fieldIndex ) const
{
	return GetFieldName( E_PoseFieldType_CustomScalar, fieldIndex );
}

S32 C_Pose::FindCustomScalarByName( const C_StringId& name ) const
{
	return FindFieldByName( E_PoseFieldType_CustomScalar, name );
}
#endif

const C_ScalarField& C_Pose::GetCustomScalar( U32 fieldIndex ) const
{
	return *static_cast<const C_ScalarField*>(GetField( E_PoseFieldType_CustomScalar, fieldIndex ));
}

C_ScalarField& C_Pose::GetCustomScalar( U32 fieldIndex )
{
	return *static_cast<C_ScalarField*>(GetField( E_PoseFieldType_CustomScalar, fieldIndex ));
}


void C_Pose::SetCustomScalarDefault(U32 fieldIndex, F32 defaultValue)
{
	BIZ_ASSERT(fieldIndex < GetNumCustomScalars());
	m_scalarDefaults(m_data)[fieldIndex] = defaultValue;
}

F32 C_Pose::GetCustomScalarDefault(U32 fieldIndex) const
{
	BIZ_ASSERT(fieldIndex < GetNumCustomScalars());
	return m_scalarDefaults(m_data)[fieldIndex];
}


//
// Custom vector3 fields.
//

U32 C_Pose::GetNumCustomVector3s() const
{
	return GetFieldCount( E_PoseFieldType_CustomVector3 );
}

#ifndef PLATFORM_SPU
const C_StringId& C_Pose::GetCustomVector3Name( U32 fieldIndex ) const
{
	return GetFieldName( E_PoseFieldType_CustomVector3, fieldIndex );
}

S32 C_Pose::FindCustomVector3ByName( const C_StringId& name ) const
{
	return FindFieldByName( E_PoseFieldType_CustomVector3, name );
}
#endif

const C_Vector3Field& C_Pose::GetCustomVector3( U32 fieldIndex ) const
{
	return *static_cast<const C_Vector3Field*>(GetField( E_PoseFieldType_CustomVector3, fieldIndex ));
}

C_Vector3Field& C_Pose::GetCustomVector3( U32 fieldIndex )
{
	return *static_cast<C_Vector3Field*>(GetField( E_PoseFieldType_CustomVector3, fieldIndex ));
}

//
// Custom vector4 fields.
//

U32 C_Pose::GetNumCustomVector4s() const
{
	return GetFieldCount( E_PoseFieldType_CustomVector4 );
}

#ifndef PLATFORM_SPU
const C_StringId& C_Pose::GetCustomVector4Name( U32 fieldIndex ) const
{
	return GetFieldName( E_PoseFieldType_CustomVector4, fieldIndex );
}

S32 C_Pose::FindCustomVector4ByName( const C_StringId& name ) const
{
	return FindFieldByName( E_PoseFieldType_CustomVector4, name );
}
#endif

const C_Vector4Field& C_Pose::GetCustomVector4( U32 fieldIndex ) const
{
	return *static_cast<const C_Vector4Field*>(GetField( E_PoseFieldType_CustomVector4, fieldIndex ));
}

C_Vector4Field& C_Pose::GetCustomVector4( U32 fieldIndex )
{
	return *static_cast<C_Vector4Field*>(GetField( E_PoseFieldType_CustomVector4, fieldIndex ));
}

//
// Generic field access.
//

const void* C_Pose::GetField( E_PoseFieldType fieldType, U32 fieldIndex ) const
{
#ifdef CFG_DEBUG
	BIZ_ASSERT( m_poseLayout );
	BIZ_ASSERT( m_data.IsValid() );
#endif
	return &m_data[0] + m_poseLayout.RawPtr()->GetFieldOffset( fieldType, fieldIndex );
}

void* C_Pose::GetField( E_PoseFieldType fieldType, U32 fieldIndex )
{
#ifdef CFG_DEBUG
	BIZ_ASSERT( m_poseLayout );
	BIZ_ASSERT( m_data.IsValid() );
#endif
	return &m_data[0] + m_poseLayout.RawPtr()->GetFieldOffset( fieldType, fieldIndex );
}

const void* C_Pose::GetFirstField( E_PoseFieldType fieldType ) const
{
#ifdef CFG_DEBUG
	BIZ_ASSERT( m_poseLayout );
	BIZ_ASSERT( m_data.IsValid() );
#endif
	return &m_data[0] + m_poseLayout.RawPtr()->GetFirstFieldOffset( fieldType );
}

U32 C_Pose::GetFieldCount( E_PoseFieldType fieldType ) const
{
	return m_poseLayout ? m_poseLayout.RawPtr()->GetFieldCount( fieldType ) : 0;
}

U32 C_Pose::GetFieldSize( E_PoseFieldType fieldType ) const
{
	return C_PoseLayout::GetFieldSize( fieldType );
}
	
#ifndef PLATFORM_SPU
const C_StringId& C_Pose::GetFieldName( E_PoseFieldType fieldType, U32 index ) const
{
	BIZ_ASSERT( m_poseLayout );
	return m_poseLayout.RawPtr()->GetFieldName( fieldType, index );
}

S32 C_Pose::FindFieldByName( E_PoseFieldType fieldType, const C_StringId& name ) const
{
	BIZ_ASSERT(m_poseLayout);
	return m_poseLayout.RawPtr()->FindFieldByName( fieldType, name );
}
#endif

U32 C_Pose::LayoutMemory(const C_PoseLayout& poseLayout)
{
	// Reserve memory for all pose fields followed by default values for scalar channels.
	C_MemoryBlockSizer sizer(poseLayout.GetPoseDataSize());
	sizer.Allocate(m_scalarDefaults, poseLayout.GetFieldCount(E_PoseFieldType_CustomScalar));
	sizer.Align(16);
	return sizer.GetSize();
}

} // namespace Animation
