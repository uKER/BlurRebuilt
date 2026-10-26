#ifndef ANIMATION_POSE_H
#define ANIMATION_POSE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "PoseFieldTypes.h"
#include <animation/common/BlendModes.h>
#include <system/scheduler/jobqueue.h>
#include <animation/common/MemoryBlock.h>

/**
	\namespace Animation
	
	Runtime animation library namespace.
*/
namespace Animation
{

	class C_PoseFormat;
	class C_PoseLayout;
	class C_PoseMapping;
	class C_StringId;
		
	class C_ScalarField;
	class C_Vector2Field;
	class C_Vector3Field;
	class C_Vector4Field;
	class C_QuaternionField;
	class C_JointTransform;
	class C_Skeleton;
	
	
	/**
		Represents the state of animated data at the current time.
		
		@note The Get*Name() and Find*ByName() functions are not supported on SPU.
	*/
	class ALIGN16 C_Pose
	{
	public:
		/// Construct an uninitialised pose.
		C_Pose();
		
		/// Construct a pose. PS3 PPU only.
		explicit C_Pose( const C_PoseFormat& poseFormat );
		
		/// Destructor. Releases internally allocated data buffer, if used.
		~C_Pose();
	
		/// Recreate pose. PS3 PPU only.
		void Create( const C_PoseFormat& poseFormat );
		
		/// Create a pose using an existing pose layout and preallocated memory block.
		/// PS3 SPU and PPU.
		/// Internal use only. 
		void CreateInPlace( const C_PoseLayout& poseLayout, U8* data );
				
		/// Destroy pose data and clear pose layout pointer.
		void Destroy();
		
		/// Returns true if pose has been created.
		bool IsValid() const;
		
		//
		// DMA operations.
		//

		/// Fetch pose data and pose layout into local memory asynchronously.
		void SpuFetchAsync( U32 tag );
				
		/// Fetch pose data and pose layout into local memory.
		void SpuFetch();
		
		/// Store pose data back into main memory.
		void SpuStore();
		
		/// Store pose data back into main memory asynchronously.
		void SpuStoreAsync(U32 tag);
		
		
		//
		// Multi-threading/SPU states.
		//
		
		static void AcquireReader(C_Pose* ppuPose);
		
		static void ReleaseReader(C_Pose* ppuPose);
		
		static void AcquireWriter(C_Pose* ppuPose);
		
		static void ReleaseWriter(C_Pose* ppuPose);
		
		/// Tests to see if a pose can be read without interference from other threads/SPUs.
		static bool IsReadable(const C_Pose* ppuPose);
		
		/// Tests to see if a pose can be modified without affecting other threads/SPUs.
		static bool IsWriteable(const C_Pose* ppuPose);
		
		//
		// Blending operations.
		//
		
		/// Clears all fields, leaving them undefined.
		void Clear();
		
		/// Clears all fields to the base pose of a skeleton. If @a skeleton is NULL, this is equivalent to Clear().
		void ClearToBasePose( const C_Skeleton* skeleton );
		
		/// Normalise all fields, leaving them defined using values from previous blend operations
		/// or set to sensible defaults. Every field will end up with a weight of 1.
		/// \todo allow an arbitrary final weight to be specified?
		void Normalise( const C_Skeleton* basePose );
		
		/// Normalise all fields, using corresponding field values from @a defaultPose where weights are zero.
		void Normalise( const C_Pose& defaultPose );
			
		/// Blend this pose into another one using the specified blending mode and weight.
		void BlendInto( C_Pose& dest, E_BlendMode blendMode, F32 weight ) const;
		
		/// Copies mapped fields from sourcePose into this pose.
		void CopyFrom( const C_Skeleton* destSkeleton, const C_Pose& sourcePose, const C_Skeleton* sourceSkeleton, const C_PoseMapping& poseMapping, bool ignoreScale );

		/// Copies default values from sourcePose.
		void CopyDefaultsFrom(const C_Pose& sourcePose);
		
		//
		// Skeleton joint transform fields.
		//
		
		/// Get number of joint transforms.
		U32 GetNumJointTransforms() const;
		
		/// Get name of specified joint transform.
		const C_StringId& GetJointTransformName( U32 fieldIndex ) const;
		
		/// Find a joint transform by name.
		S32 FindJointTransformByName( const C_StringId& name ) const;

		/// Find a joint transform by name.
		S32 FindJointTransformByName( const CHAR8* name ) const;
		
		/// Get const joint transform.
		const C_JointTransform& GetJointTransform( U32 index ) const;
		
		/// Get writable joint transform.
		C_JointTransform& GetJointTransform( U32 index );
		
		/// Get a joint transform in model space. If skeleton is NULL then joint transforms are assumed to be in local space.
		/// It is assumed that all transforms in this pose are normalised.
		C_JointTransform GetModelJointTransform( U32 index, const C_Skeleton* skeleton ) const;
		
		void SetModelJointTransform( U32 index, const C_JointTransform& transform, const C_Skeleton* skeleton );

#if 0
		// Converts all transforms into model space.
		void ConvertToModelSpace(const C_Skeleton* skeleton);

		void ConvertToLocalSpace(const C_Skeleton* skeleton);
#endif
					
		//
		// Trajectory transform fields.
		//

		/// Returns true if pose has a trajectory transform. Any pose created
		/// with a C_PoseFormat that references a skeleton will have a trajectory
		/// transform field.
		bool HasTrajectoryTransform() const;
		
		/// Get read-only reference to trajectory transform.
		const C_JointTransform& GetTrajectoryTransform() const;
		
		/// Get writeable reference to trajectory transform.
		C_JointTransform& GetTrajectoryTransform();
		
		/// Returns true if pose has a trajectory delta transform. Any pose created
		/// with a C_PoseFormat that references a skeleton will have a trajectory
		/// delta transform field.
		bool HasTrajectoryDeltaTransform() const;
		
		/// Get read-only reference to trajectory delta transform.
		const C_JointTransform& GetTrajectoryDeltaTransform() const;
		
		/// Get writeable reference to trajectory delta transform.
		C_JointTransform& GetTrajectoryDeltaTransform();
				
		//
		// Custom scalar fields.
		//
		
		/// Get number of custom scalars in this pose.
		U32 GetNumCustomScalars() const;
		
		/// Get name of a particular custom scalar field.
		const C_StringId& GetCustomScalarName( U32 fieldIndex ) const;
		
		/// Find a custom scalar field by name.
		S32 FindCustomScalarByName( const C_StringId& name ) const;
		
		/// Get reference to a read-only custom scalar field.
		const C_ScalarField& GetCustomScalar( U32 fieldIndex ) const;
		
		/// Get reference to writeable custom scalar field.
		C_ScalarField& GetCustomScalar( U32 fieldIndex );

		void SetCustomScalarDefault(U32 fieldIndex, F32 defaultValue);

		F32 GetCustomScalarDefault(U32 fieldIndex) const;


		//
		// Custom vector3 fields.
		//
		
		/// Get number of custom vector3 fields in this pose.
		U32 GetNumCustomVector3s() const;
		
		/// Get name of a particular custom vector3 field.
		const C_StringId& GetCustomVector3Name( U32 fieldIndex ) const;
		
		/// Find a custom vector3 field by name.
		S32 FindCustomVector3ByName( const C_StringId& name ) const;
		
		/// Get reference to read only custom vector3 field.
		const C_Vector3Field& GetCustomVector3( U32 fieldIndex ) const;
				
		/// Get reference to writeable custom vector3 field.
		C_Vector3Field& GetCustomVector3( U32 fieldIndex );
		
		//
		// Custom vector4 fields.
		//
		
		/// Get number of custom vector4 fields in this pose.
		U32 GetNumCustomVector4s() const;
		
		/// Get name of a particular custom vector4 field.
		const C_StringId& GetCustomVector4Name( U32 fieldIndex ) const;
		
		/// Find a custom vector4 field by name.
		S32 FindCustomVector4ByName( const C_StringId& name ) const;
		
		/// Get reference to read only custom vector4 field.
		const C_Vector4Field& GetCustomVector4( U32 fieldIndex ) const;
				
		/// Get reference to writeable custom vector4 field.
		C_Vector4Field& GetCustomVector4( U32 fieldIndex );		
		
		//
		// Field element access (internal use only)
		//
		
		/// Get a quaterion field element.
		C_QuaternionField* GetQuaternionFieldElement( E_PoseFieldElement element, U32 fieldIndex );
		
		/// Get a vector3 field element.
		C_Vector3Field* GetVector3FieldElement( E_PoseFieldElement element, U32 fieldIndex );
		
		/// Get a scalar field element.
		C_ScalarField* GetScalarFieldElement( E_PoseFieldElement element, U32 fieldIndex );
		
		/// Get raw field element.
		void* GetFieldElement( E_PoseFieldElement element, U32 fieldIndex );

		/// Get pose layout (used to obtain a CycleMapping in CycleInstance)
		const C_PoseLayout* GetPoseLayout() const
		{
			return m_poseLayout;
		}				

		/// Get raw pointer to pose data. Internal use only.
		void* GetPoseData();

		/// Return size of internal data buffer.
		U32 GetPoseDataSize() const		{ return m_data.GetSize(); }

	//private:
	
		//
		// Untyped field access.
		//
	
		/// Get const pointer to a data field.
		const void* GetField( E_PoseFieldType fieldType, U32 fieldIndex ) const;
		
		/// Get pointer to a data field.
		void* GetField( E_PoseFieldType fieldType, U32 fieldIndex );

		/// Get const pointer to the first field of the specified type.
		const void* GetFirstField( E_PoseFieldType fieldType ) const;

		// ** Only used in cycle to check existence of trajectory field.
		/// Get number of fields of specified type.
		U32 GetFieldCount( E_PoseFieldType fieldType ) const;
		
		/// Get size of a single field of the specified type (equivalent to stride).
		U32 GetFieldSize( E_PoseFieldType fieldType ) const;

		
	#ifndef PLATFORM_SPU
		/// Get the name of a particular field. PPU-only.
		const C_StringId& GetFieldName( E_PoseFieldType fieldType, U32 index ) const;
		
		/// Find the index of a field with a particular name. Returns -1 if no matching field found. PPU-only.
		S32 FindFieldByName( E_PoseFieldType fieldType, const C_StringId& name ) const;

	#endif
	
	private:
		U32 LayoutMemory(const C_PoseLayout& poseLayout);

		// TODO: could replace this Spu::Ptrs if a C_Pose is never written to using DMA.
		Spu::Ptr<const C_PoseLayout>	m_poseLayout;
		C_MemoryBlock					m_data;
		
		// NB. to avoid corrupting these counters, do not DMA a C_Pose back to main memory! 
		volatile S32					m_readerCount;	
		volatile S32					m_writerCount;
		
		C_RelativePtr<F32>				m_scalarDefaults;
			
		struct C_ElementInfo
		{
			E_PoseFieldType		fieldType;
			U8					offset;
		};
		
		static C_ElementInfo s_elementInfo[ E_PoseFieldElement_Count ];

#ifndef PLATFORM_SPU
	public:
		struct C_Stats
		{
			C_Stats();

			U32		numPoses;
			U32		memoryUsedByPoseData;
		};
		static void GetStats(C_Stats& outStats);
	
	private:
		static C_Stats	s_stats;
#endif

	};


}


#endif

