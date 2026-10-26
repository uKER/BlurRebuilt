#ifndef ANIMATION_POSEMAPPING_H
#define ANIMATION_POSEMAPPING_H

#include <animation/common/StringId.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseFieldTypes.h>
#include <animation/pose/PoseLayout.h>
#include <btl/containers/vector.h>

namespace Animation
{

	class C_Pose;
	class C_PoseLayout;
		
	class ALIGN16 C_PoseMapping
	{
	public:
		C_PoseMapping();
		
		C_PoseMapping(const C_Pose& sourcePose, const C_Pose& destPose, const CHAR8* namespacePrefix = NULL);
		
		~C_PoseMapping();
		
		void Create(const C_Pose& sourcePose, const C_Pose& destPose, const CHAR8* namespacePrefix = NULL);
		
		S32 GetSourceFieldIndex(E_PoseFieldType poseFieldType, S32 destFieldIndex) const;
		
		void SpuFetch();
		
		bool IsValidSourcePose(const C_Pose& sourcePose) const;

		/// Returns true if mapping does nothing at all.
		bool IsEmpty() const;
					
	private:
		void Create(const C_PoseLayout& sourcePoseLayout, const C_PoseLayout& destPoseLayout, const CHAR8* namespacePrefix);
		void Destroy();
		
		const C_PoseLayout*		m_sourcePoseLayout;
		const C_PoseLayout*		m_destPoseLayout;
		U16						m_fieldTypeStartIndex[E_PoseFieldType_Count];
		Btl::Vector<S16>		m_sourceFieldIndices;	// Source pose field indices indexed by destination field index.
		U32						m_numMappedFields;
	};
	
	inline S32 C_PoseMapping::GetSourceFieldIndex(E_PoseFieldType poseFieldType, S32 destFieldIndex) const
	{
		return m_sourceFieldIndices[m_fieldTypeStartIndex[poseFieldType] + destFieldIndex];
	}

	inline void C_PoseMapping::SpuFetch()
	{
	#ifdef PLATFORM_SPU
		Btl::SpuFetch(m_sourceFieldIndices);
		Spu::FetchAlloc(m_destPoseLayout);
	#endif
	}
	
	inline bool C_PoseMapping::IsValidSourcePose(const C_Pose& sourcePose) const
	{
		return sourcePose.GetPoseLayout() == m_sourcePoseLayout;
	}

}

#endif
