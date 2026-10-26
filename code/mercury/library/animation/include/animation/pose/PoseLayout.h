#ifndef ANIMATION_POSELAYOUT_H
#define ANIMATION_POSELAYOUT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/pose/PoseFieldTypes.h>
#include <animation/common/StringId.h>
#include <btl/containers/vector.h>

namespace Animation
{

	class C_StringId;
	class C_CycleMapping;
	class C_PoseFormat;
	
	/**
		Describes the data fields stored in a C_Pose.
		
		C_PoseLayouts must only be created in main memory.

		Fields are laid out in decreasing order of size to minimise padding
		and ensure correct alignment. Note that this order is not necessarily 
		the same as the order implied by the E_PoseFieldType enumeration.
	*/
	class ALIGN16 C_PoseLayout
	{
		//
		// Construction.
		// 
		
	private:
		C_PoseLayout( const C_PoseFormat& poseFormat );

		~C_PoseLayout();
		
	public:
	
		static const C_PoseLayout* CreatePoseLayout( const C_PoseFormat& key );
		
		/// Returns a pose layout with a single trajectory transform, used internally by C_Cycle::EvaluateTrajectoryTransform.
		static const C_PoseLayout* GetTrajectoryPoseLayout();

		//
		// Reference counting.
		//
		void AddRef() const;
		bool DecRef() const;

		/// Get total size of pose data in bytes.
		U32 GetPoseDataSize() const;
		
		/// Returns total number of fields of all types in the pose layout.
		U32 GetTotalFieldCount() const;
		
		/// Get offset of field from start of pose data.
		U32 GetFieldOffset( E_PoseFieldType fieldType, U32 index ) const;

		/// Get offset of first field of the specified type from start of pose data.
		U32 GetFirstFieldOffset( E_PoseFieldType fieldType ) const;
		
		/// Get number of fields of specified type.
		U32 GetFieldCount( E_PoseFieldType fieldType ) const;
		
		/// Get size of a single field of the specified type (equivalent to stride).
		static U32 GetFieldSize( E_PoseFieldType fieldType );
		
		/// Get unique identifier.
		U32 GetUniqueId() const;
		
#ifndef PLATFORM_SPU

		/// Get the name of a particular field. PPU-only.
		const C_StringId& GetFieldName( E_PoseFieldType fieldType, U32 index ) const;
		
		/// Find the index of a field with a particular name. Returns -1 if no matching field found. PPU-only.
		S32 FindFieldByName( E_PoseFieldType fieldType, const C_StringId& name ) const;
		
		/// Returns the index of a field with the specified name, or -1 if no matching field foumd.
		S32 FindFieldByName( E_PoseFieldType fieldType, const CHAR8* name ) const;
		
		//
		// CycleMapping list.
		//
		
		C_CycleMapping* GetMappingListHead() const
		{
			return m_mappings;
		}
		
		void SetMappingListHead( C_CycleMapping* mapping ) const
		{
			m_mappings = mapping;
		}
		
	public:
		
		//
		// Global pose layout cache.
		//
		
		void Link();
		
		void LinkTo( C_PoseLayout*& head );
		
		void Unlink();
		
		void UnlinkFrom( C_PoseLayout*& head );
		
		C_PoseLayout* GetNext() const;

		bool operator == ( const C_PoseLayout& rhs ) const;
		bool operator == ( const C_PoseFormat& rhs ) const;
		
#endif

	private:
		mutable S32 m_refCount;
		U32 m_uniqueId;
		U32 m_totalFields;			///< Total number of fields.
		C_StringIdArray m_fieldNames;
		
		U16	m_fieldCounts[ E_PoseFieldType_Count ];
		U16 m_fieldOffsets[ E_PoseFieldType_Count ];
		U16 m_fieldStartIndices[ E_PoseFieldType_Count ];			///< Index of first field of each type.
		C_PoseLayout* m_prev;
		C_PoseLayout* m_next;
		mutable C_CycleMapping* m_mappings;
		
		static U32					s_fieldSizes[ E_PoseFieldType_Count ];
		static E_PoseFieldType		s_fieldTypesSortedBySize[ E_PoseFieldType_Count ];
		static const C_PoseLayout*	s_trajectoryPoseLayout;
		
	};
}

#include "PoseLayout.inl"

#endif
