#ifndef ANIMATION_SKELETONDESERIALISE_INL
#define ANIMATION_SKELETONDESERIALISE_INL

#include <system/memory/MemoryTracker.h>
#include <animation/skeleton/SkeletonConstraintsDeserialise.inl>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>


namespace Animation
{

#ifndef PLATFORM_SPU
	
	#define kSkeletonMemorySubTag	"animation.skeleton"
	
	enum E_SkeletonVersion
	{
		E_SkeletonVersion_Original					= 5,
		E_SkeletonVersion_JointScaling				= 6,
		E_SkeletonVersion_JointTransformWithScale	= 7
	};
	
	template < typename T >
	inline bool C_Skeleton::Deserialise( T& in )
	{
		MEMORY_TRACKER_PUSH_SUB( kSkeletonMemorySubTag );

		Destroy();
		
		static const U32 kCurrentVersion = E_SkeletonVersion_JointTransformWithScale;
		
		U32 version = 0;
		in >> version;
		
		if ( version < E_SkeletonVersion_Original || version > kCurrentVersion )
		{
			BIZ_ASSERT( !"Bad skeleton version" );
			MEMORY_TRACKER_POP_SUB( kSkeletonMemorySubTag );
			return false;
		}
		
		if (version >= E_SkeletonVersion_JointScaling && version < E_SkeletonVersion_JointTransformWithScale)
		{
			U8 supportsJointScaling = 0;
			in >> supportsJointScaling;
		}
		
		U32 numJoints = 0;
		in >> numJoints;
		BIZ_ASSERT( numJoints <= 0xffff );
		m_numJoints = numJoints;

		U32 numJoints16ByteAligned = ((m_numJoints * sizeof(S16) + 15) & (~0xf)) / sizeof(S16);
		BIZ_ASSERT(numJoints16ByteAligned >= m_numJoints);

		m_parentIndices = new S16[numJoints16ByteAligned];
		m_basePoseTransforms = new C_JointTransform[m_numJoints];
		m_invWorldBasePoseMatrices = new Maths::Mat43[m_numJoints];

#if defined(PLATFORM_PS3)
		// Check alignment for DMA.
		BIZ_ASSERT(0 == (((U32)m_parentIndices) & 0xf));
		BIZ_ASSERT(0 == (((U32)m_basePoseTransforms) & 0xf));
		BIZ_ASSERT(0 == (((U32)m_invWorldBasePoseMatrices) & 0xf));
#endif
		
		// Read parent indices.
		for ( U32 i = 0; i < numJoints; ++i )
		{
			in >> m_parentIndices[ i ];
		}
		
		// Read local base poses.
		if ( version >= E_SkeletonVersion_JointTransformWithScale )
		{
			for ( U32 i = 0; i < numJoints; ++i )
			{
				in >> m_basePoseTransforms[ i ];
			}
		}
		else
		{
			for ( U32 i = 0; i < numJoints; ++i )
			{
				m_basePoseTransforms[ i ].DeserialiseWithoutScale(in);
			}
		}
		
		
		// Read inverse world base poses.
		for ( U32 i = 0; i < numJoints; ++i )
		{
			Maths::HMat43 tmp;
			in >> tmp;
			m_invWorldBasePoseMatrices[ i ] = tmp;
		}
		
		// Read joint constraints.
		in >> m_constraints;
		
		// Read joint names.
		m_jointNames.Deserialise( in );
		
		// Calculate first parented joint index.
		m_firstParentedJointIndex = -1;
		for ( U32 i = 1; i < numJoints; ++i )
		{
			if ( GetJointParentIndex( i ) >= 0 )
			{
				m_firstParentedJointIndex = i;
				break;
			}
		}
		
		MEMORY_TRACKER_POP_SUB( kSkeletonMemorySubTag );
		
		return true;
	}
	
	#undef kSkeletonMemorySubTag
	
#endif

} // end Animation

#endif
