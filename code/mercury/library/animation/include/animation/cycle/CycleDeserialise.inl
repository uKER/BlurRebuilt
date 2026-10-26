#ifndef ANIMATION_CYCLEDESERIALISE_INL
#define ANIMATION_CYCLEDESERIALISE_INL

#include <animation/common/MemoryBlock.h>
#include <system/memory/MemoryTracker.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/common/Stats.h>

#define kCycleMemorySubTag		"animation.cycle"

namespace Animation
{
#ifndef PLATFORM_SPU
	
	template < typename T, typename TStream >
	T* ReadArray( TStream& in, U32 count, C_MemoryBlockWriter& writer )
	{
		T* dest = writer.Allocate< T >( count );		
		for ( U32 j = 0; j < count; ++j )
		{
			in >> dest[ j ];
		}
		return dest;
	}

	template < typename TStream >
	void ReadVaryingCurveKeys( TStream& in, const U16 keyCounts[ E_VaryingCurveType_Count ], C_MemoryBlockWriter& memoryBlockWriter )
	{
		ReadArray< F32 >( in, keyCounts[ E_VaryingCurveType_VaryingVector3F32 ] * 3, memoryBlockWriter );
		ReadArray< U32 >( in, keyCounts[ E_VaryingCurveType_VaryingQuaternion32 ], memoryBlockWriter );
		ReadArray< F32 >( in, keyCounts[ E_VaryingCurveType_VaryingScalarF32 ], memoryBlockWriter );
		ReadArray< U16 >( in, keyCounts[ E_VaryingCurveType_VaryingQuaternion48 ] * 3, memoryBlockWriter );
		ReadArray< U16 >( in, keyCounts[ E_VaryingCurveType_VaryingVector3U16 ] * 3, memoryBlockWriter );
		ReadArray< U8 >( in, keyCounts[ E_VaryingCurveType_VaryingVector3U8 ] * 3, memoryBlockWriter );
	}

	inline U32 AlignUp( U32 offset, U32 align )
	{
		return ( ( offset + align - 1 ) & ~( align - 1 ) );
	}
	
	inline U16 RemapChannelInfo(U16 info)
	{
		S32 fieldIndex = info & ~0x3f;
		S32 elementType = (info & 0x3f);
				
		static const U8 kOldToNewElementMapping[14] = 
		{
			// New mapping, old mapping.
			E_PoseFieldElement_JointTransformRotation,		// was ObjectTransformRotation
			E_PoseFieldElement_JointTransformTranslation,	// was ObjectTransformTranslation
			E_PoseFieldElement_JointTransformScale,			// was ObjectTransformScale
			E_PoseFieldElement_JointTransformRotation,
			E_PoseFieldElement_JointTransformTranslation,
			E_PoseFieldElement_CustomScalar,
			E_PoseFieldElement_CustomVector3,
			E_PoseFieldElement_TrajectoryDeltaTransformRotation,
			E_PoseFieldElement_TrajectoryDeltaTransformTranslation,
			E_PoseFieldElement_TrajectoryTransformRotation,
			E_PoseFieldElement_TrajectoryTransformTranslation,
			E_PoseFieldElement_CustomVector4XYZ,
			E_PoseFieldElement_CustomVector4W
		};
		elementType = kOldToNewElementMapping[elementType];
		
		return fieldIndex | (elementType & 0x3f);
	}
	
	template < typename T >
	inline bool C_Cycle::Deserialise( T& in, U32 flags )
	{
		MEMORY_TRACKER_PUSH_SUB( kCycleMemorySubTag );
				
		const U32 kCurrentVersion = 15;
		const U32 kJointTransformWithScaleVersion = 15;
		
		// Padding to prevent SIMD reads at the ends of buffers from causing access violations by
		// attempting to access unmapped memory pages.
		const U32 kSimdPadding = 15;
		
		Destroy();
		
		U32 version;
		
		in >> version;
		
		if ( version < 12 || version > kCurrentVersion )
		{
			BIZ_ASSERT( !"Unsupported cycle version." );
			MEMORY_TRACKER_POP_SUB( kCycleMemorySubTag );
			return false;
		}

		m_referenceTransform.SetIdentity();

		if ( version >= 14 )
		{
			in >> m_numReferenceTransforms;
			BIZ_ASSERT( m_numReferenceTransforms <= 1 );
			
			if ( version >= kJointTransformWithScaleVersion )
			{
				for ( U32 i = 0; i < m_numReferenceTransforms; ++i )
				{
					C_JointTransform refTransform;
					in >> refTransform;
					if ( i == 0 )
					{
						m_referenceTransform = refTransform;
					}
				}
			}
			else
			{
				for ( U32 i = 0; i < m_numReferenceTransforms; ++i )
				{
					C_JointTransform refTransform;
					refTransform.DeserialiseWithoutScale(in);
					
					if ( i == 0 )
					{
						m_referenceTransform = refTransform;
					}
				}
			}
		}
		else
		{
			m_numReferenceTransforms = 0;
		}
		
		if ( version >= 13 )
		{
			if ( version >= kJointTransformWithScaleVersion )
			{
				in >> m_trajectoryFirst;
				in >> m_trajectoryFinal;
			}
			else
			{
				m_trajectoryFirst.DeserialiseWithoutScale(in);
				m_trajectoryFinal.DeserialiseWithoutScale(in);
			}
			
			m_trajectoryDelta = m_trajectoryFirst.Inverse() * m_trajectoryFinal;
		}
		else
		{
			m_trajectoryDelta.DeserialiseWithoutScale(in);
			
			m_trajectoryFirst.SetIdentity();
			m_trajectoryFinal = m_trajectoryDelta;
		}
		
		in >> m_trajectoryRootFieldIndex;
				
		in >> m_numFrames;
		in >> m_playbackDuration;
		in >> m_frameRate;
		
		BIZ_ASSERT(m_numFrames > 0);
		
		{
			// Calculate delta transform relative to first frame.
			C_JointTransform delta = m_trajectoryFinal * m_trajectoryFirst.Inverse();
		
			// Calculate linear and angular velocities.
			const Maths::VF32 invDuration = Maths::Rcp( Maths::MakeVF32( m_playbackDuration / m_frameRate ) );
			Maths::Vector3 dt = delta.GetTranslation();
			m_linearVelocity = dt * invDuration;
			
			// Convert delta to angle-axis form and divide by time to get angular velocity.
			Maths::Quaternion dq = delta.GetRotation();
			Maths::Normal3 rotationAxis = Maths::RotationAxis( dq );
			
			#if 1
			Maths::VF32 rotationAngle = Maths::MakeVF32( acosf( Maths::GetF32( Maths::Real( dq ) ) ) * 2.f );
			#else
			Maths::VF32 rotationAngle = Maths::RotationAngle( dq ); // TODO: fails at runtime on PS3 as SIMD_ScalarArcCos not implemented.
			#endif
			m_angularVelocity = rotationAxis * rotationAngle * invDuration;

#if 0								
			LOGINFO( Debug::GetLogger("mercury.animation"), "Linear velocity: %f, %f, %f", Maths::GetF32(Maths::X(m_linearVelocity)), Maths::GetF32(Maths::Y(m_linearVelocity)), Maths::GetF32(Maths::Z(m_linearVelocity)));
			LOGINFO( Debug::GetLogger("mercury.animation"), "Angular velocity: %f, %f, %f", 
				Maths::RadToDeg(Maths::GetF32(Maths::X(m_angularVelocity))),
				Maths::RadToDeg(Maths::GetF32(Maths::Y(m_angularVelocity))),
				Maths::RadToDeg(Maths::GetF32(Maths::Z(m_angularVelocity))));
#endif
		}
		
		in >> m_numFields;
		
		if ( m_numFields > 0 )
		{
			m_fieldTypes = new U8[ AlignUp( m_numFields, 16 ) ];
			
			if (version >= kJointTransformWithScaleVersion)
			{
				for ( U32 i = 0; i < m_numFields; ++i )
				{
					in >> m_fieldTypes[ i ];
				}
			}
			else
			{
				// Remap old E_PoseFieldType enum onto new one where object transforms and joint transforms are the same.
				for ( U32 i = 0; i < m_numFields; ++i )
				{
					U8 fieldType;
					in >> fieldType;
					S32 newFieldType = Maths::Max(static_cast<S32>(fieldType) - 1, 0);
					m_fieldTypes[i] = newFieldType;
				}
			}
		}
		else
		{
			m_fieldTypes = NULL;
		}
		
		m_fieldNames.Deserialise( in );

		if (flags & kLoadHeaderOnly)
		{
			m_isPartiallyLoaded = 1;

			m_totalConstantCurves = 0;
			m_totalVaryingCurves = 0;
			m_constantCurveDataSize = 0;
			m_totalSegmentDataSize = 0;
			m_numSegments = 0;
		}
		else
		{
			m_isPartiallyLoaded = 0;
			
			in >> m_totalConstantCurves;
			in >> m_totalVaryingCurves;
			
			for ( U32 i = 0; i < E_ConstantCurveType_Count; ++i )
			{
				in >> m_numConstantCurvesOfType[ i ];
			}
			for ( U32 i = 0; i < E_VaryingCurveType_Count; ++i )
			{
				in >> m_numVaryingCurvesOfType[ i ];
			}
			
			// Load channel info (packed field indices, offset and type)
			U32 totalCurves = m_totalConstantCurves + m_totalVaryingCurves;
			
			if ( totalCurves > 0 )
			{
				m_channelInfo = new U16[ AlignUp( totalCurves, 8 ) ];
				
				if (version >= kJointTransformWithScaleVersion)
				{
					for ( U32 i = 0; i < totalCurves; ++i )
					{
						in >> m_channelInfo[ i ];	
					}
				}
				else
				{
					// Upgrade old channel info to map object transform field elements onto joint transform ones.
					for ( U32 i = 0; i < totalCurves; ++i )
					{
						U16 channelInfo;
						in >> channelInfo;
						m_channelInfo[i] = RemapChannelInfo(channelInfo);
					}
				}
			}
			else
			{
				m_channelInfo = NULL;
			}
			
			in >> m_constantCurveDataSize;
			m_constantCurveDataSize = AlignUp( m_constantCurveDataSize, 16 );
			if ( m_constantCurveDataSize > 0 )
				m_constantCurveData = new U8[ m_constantCurveDataSize + kSimdPadding];	// TODO: make 16-byte aligned
			else
				m_constantCurveData = NULL;
			
			//
			// Read constant curve data.
			//
			
			C_MemoryBlockWriter constantCurvememoryBlock( m_constantCurveData, m_constantCurveDataSize );
			
			// Read scales and offsets for vec3 u16 and vec3 u8 curves.
			{
				U32 numCurves = m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U16 ]
					+ m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U8 ];

				ReadArray< F32 >( in, numCurves * 6, constantCurvememoryBlock );
			}
			
			// Read constant vec3 f32 values.
			ReadArray< F32 >( in, m_numConstantCurvesOfType[ E_ConstantCurveType_ConstantVector3F32 ] * 3, constantCurvememoryBlock );
			
			// Read scalar F32 values.
			ReadArray< F32 >( in, m_numConstantCurvesOfType[ E_ConstantCurveType_ConstantScalarF32 ], constantCurvememoryBlock );
			
			// Read constant quaternion48 values.
			ReadArray< U16 >( in, m_numConstantCurvesOfType[ E_ConstantCurveType_ConstantQuaternion48 ] * 3, constantCurvememoryBlock );	
			
			//
			// Read segments.
			//
			
			in >> m_numSegments;
			in >> m_totalSegmentDataSize;
			
			if ( m_numSegments > 0 )
			{
				m_segments = new C_Segment[ m_numSegments ];
				m_segmentData = new U8[m_totalSegmentDataSize + kSimdPadding];
				
			#if !defined(PLATFORM_PC)
				BIZ_ASSERT( 0 == (( (U32) m_segments ) & 0xf ));	// must be 16-byte aligned.
				BIZ_ASSERT( 0 == (( (U32) m_segmentData ) & 0xf ));	// must be 16-byte aligned.
			#endif
				
				for ( U32 i = 0; i < m_numSegments; ++i )
				{
					in >> m_segments[ i ].m_startFrame;
					in >> m_segments[ i ].m_numFrames;
					in >> m_segments[ i ].m_size;
					
					BIZ_ASSERT( ( i < m_numSegments - 1 ) ? m_segments[ i ].m_numFrames > 0 : true );
				}
				
				C_MemoryBlockWriter segmentDatamemoryBlock( m_segmentData, m_totalSegmentDataSize );
				
				for ( U32 i = 0; i < m_numSegments; ++i )
				{
					C_Segment& segment = m_segments[ i ];
					
					segment.m_data = segmentDatamemoryBlock.GetTop();
					
					// Read initial frame's keys.
					ReadVaryingCurveKeys( in, m_numVaryingCurvesOfType, segmentDatamemoryBlock );
					
					if ( i != m_numSegments - 1 )
					{
						// Read key counts per curve type.
						const U16* numKeysPerCurveType = ReadArray< U16 >( in, E_VaryingCurveType_Count, segmentDatamemoryBlock );
						
						//
						// Read curve key times.
						//
						U32 keyTimeSize = 0;
						U32 totalKeysInSegment = 0;
						for ( U32 j = 0; j < E_VaryingCurveType_Count; ++j )
						{
							// One U8 storing number of keys for each curve and one U8 per key.
							keyTimeSize += numKeysPerCurveType[ j ] + m_numVaryingCurvesOfType[ j ];
							totalKeysInSegment += numKeysPerCurveType[ j ];
						}
						segment.m_totalKeys = totalKeysInSegment;
						ReadArray< U8 >( in, keyTimeSize, segmentDatamemoryBlock );

						//
						// Read keys per curve type.
						//
						ReadVaryingCurveKeys( in, numKeysPerCurveType, segmentDatamemoryBlock );
					}
					else
					{
						segment.m_totalKeys = 0;
					}

					// Align up to next block size.
					segmentDatamemoryBlock.Align( 16 );
				}
			}
			else
			{
				m_segments = NULL;
				m_segmentData = NULL;
			}
		
			// Calculate initial frame size for each segment.
			m_initialFrameSize = 0;
			m_initialFrameSize += m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3F32 ] * 12;
			m_initialFrameSize += m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingQuaternion32 ] * 4;
			m_initialFrameSize += m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingScalarF32 ] * 4;
			m_initialFrameSize += m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingQuaternion48 ] * 6;
			m_initialFrameSize += m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U16 ] * 6;
			m_initialFrameSize += m_numVaryingCurvesOfType[ E_VaryingCurveType_VaryingVector3U8 ] * 3;
		}
	
		MEMORY_TRACKER_POP_SUB( kCycleMemorySubTag );

		ANIMATION_STATS_UPDATE(memoryUsedByCycles += GetMemoryUsage() - sizeof(C_Cycle));
		
		return true;
	
	}

#endif

} // end Animation

#endif
