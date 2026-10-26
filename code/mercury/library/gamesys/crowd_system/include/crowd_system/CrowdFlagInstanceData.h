#ifndef CROWD_FLAG_INSTANCE_DATA_H
#define CROWD_FLAG_INSTANCE_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimatedCrowd
{

//! FlagInstance
struct ALIGN16 FlagInstance
{
public:
	Maths::HMat43	m_Transform;
	U32				m_InstanceIndex;
	U32				m_AnimationIndex;
	U32				m_TextureIndex;
};

//! FlagInstanceData
struct ALIGN16 FlagInstanceData
{
public:
	FlagInstance	m_Instances[CROWD_MAX_FLAG_INSTANCES];
	U32				m_NumInstances;
};

}; // namespace AnimatedCrowd

#endif // CROWD_FLAG_INSTANCE_DATA_H
