#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeUtility.h"
#include "NativeSceneNode.h"
#include "NativeSceneJointNode.h"
#include "NativeLimitMixin.h"

using namespace Aftk;

C_Status C_NativeSceneJointNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneJointNode* pSceneJointNode, 
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_SCENE_JOINT_NODE);
	}
	{
		status = C_NativeSceneNode::Serialise(
			pDiskWriter,
			pNativeFtk,
			pSceneJointNode,
			true);
		if (!status) return status;

		status = C_NativeLimitMixin::Serialise(
			pDiskWriter,
			pNativeFtk,
			(C_LimitMixin*)pSceneJointNode);
		if (!status) return status;

#ifndef AFTK_LEAN_AND_MEAN
		*pDiskWriter << pSceneJointNode->m_stiffness;
		*pDiskWriter << pSceneJointNode->m_scaleOrientation;
		*pDiskWriter << pSceneJointNode->m_orientation;
		*pDiskWriter << pSceneJointNode->m_preferredAngle;
		*pDiskWriter << pSceneJointNode->m_dof;
#else
		Matrix44 id;
		UtMaths::SetIdentity(&id);
		*pDiskWriter << Vector3F32(0,0,0);
		*pDiskWriter << id;
		*pDiskWriter << id;
		*pDiskWriter << Vector3F32(0,0,0);
		*pDiskWriter << Vector3Bool(0,0,0);
#endif
		*pDiskWriter << pSceneJointNode->m_segmentScaleCompensation;
#ifndef AFTK_LEAN_AND_MEAN
		*pDiskWriter << pSceneJointNode->m_radius;
		*pDiskWriter << pSceneJointNode->m_minDampRange;
		*pDiskWriter << pSceneJointNode->m_maxDampRange;
		*pDiskWriter << pSceneJointNode->m_minDampStrength;
		*pDiskWriter << pSceneJointNode->m_maxDampStrength;

#else
		*pDiskWriter << F32(0);
		*pDiskWriter << Vector3F32(0,0,0);
		*pDiskWriter << Vector3F32(0,0,0);
		*pDiskWriter << Vector3F32(0,0,0);
		*pDiskWriter << Vector3F32(0,0,0);

		F32 limits[E_JointLimitType_Count];
		bool isLimited[E_JointLimitType_Count];
		memset(limits, 0, sizeof(limits));
		memset(isLimited, 0, sizeof(isLimited));

		pDiskWriter->WriteBytes(
			&(limits[0]),
			sizeof(F32)*E_JointLimitType_Count);

		pDiskWriter->WriteBytes(
			&(isLimited[0]),
			sizeof(bool)*E_JointLimitType_Count);
#endif
	}
	if (useOuterBlockHeaders == true)
	{
		pDiskWriter->EndBlock();
	}
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneJointNode::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_SceneJointNode* pSceneJointNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_SCENE_JOINT_NODE))
		{
			return C_Status(C_Status::E_Status_Failure, "Failed to find joint node block.");
		}
		pDiskReader->EnterBlock();
	}
	{
		status = C_NativeSceneNode::Deserialise(
			pDiskReader,
			pNativeFtk,
			pSceneJointNode,
			true);
		if (!status) return status;

		if (pDiskReader->GetVersion() >= 210)
		{
			status = C_NativeLimitMixin::Deserialise(
				pDiskReader,
				pNativeFtk,
				(C_LimitMixin*)pSceneJointNode);
			if (!status) return status;
		}

		if (pDiskReader->GetVersion() >= 176)
		{
#ifndef AFTK_LEAN_AND_MEAN
			*pDiskReader >> pSceneJointNode->m_stiffness;
#else
			{
				Vector3F32 tmp;
				*pDiskReader >> tmp;
			}
#endif
			if (pDiskReader->GetVersion() < 200)
			{
				Vector3F32 temp;
				*pDiskReader >> temp;
			}
			else
			{
#ifndef AFTK_LEAN_AND_MEAN
				*pDiskReader >> pSceneJointNode->m_scaleOrientation;
				*pDiskReader >> pSceneJointNode->m_orientation;
#else
				Aftk::Matrix44 tmp;
				*pDiskReader >> tmp;
				*pDiskReader >> tmp;
#endif
			}
#ifndef AFTK_LEAN_AND_MEAN
			*pDiskReader >> pSceneJointNode->m_preferredAngle;
			*pDiskReader >> pSceneJointNode->m_dof;
#else
			{
				Vector3F32 temp;
				*pDiskReader >> temp;
			}
			{
				Vector3Bool temp;
				*pDiskReader >> temp;
			}
#endif
			*pDiskReader >> pSceneJointNode->m_segmentScaleCompensation;
#ifndef AFTK_LEAN_AND_MEAN
			*pDiskReader >> pSceneJointNode->m_radius;

			*pDiskReader >> pSceneJointNode->m_minDampRange;
			*pDiskReader >> pSceneJointNode->m_maxDampRange;
			*pDiskReader >> pSceneJointNode->m_minDampStrength;
			*pDiskReader >> pSceneJointNode->m_maxDampStrength;

			if (pDiskReader->GetVersion() < 210)
			{
				pDiskReader->ReadBytes(
					&(pSceneJointNode->m_limits[0]),
					sizeof(F32)*E_LimitType_Count);

				pDiskReader->ReadBytes(
					&(pSceneJointNode->m_isLimited[0]),
					sizeof(bool)*E_LimitType_Count);
			}
#else
			{F32 temp; *pDiskReader >> temp; }
			{Vector3F32 temp; *pDiskReader >> temp; }
			{Vector3F32 temp; *pDiskReader >> temp; }
			{Vector3F32 temp; *pDiskReader >> temp; }
			{Vector3F32 temp; *pDiskReader >> temp; }

			F32 limits[E_JointLimitType_Count];
			bool isLimited[E_JointLimitType_Count];
			pDiskReader->ReadBytes(
				&(limits[0]),
				sizeof(F32)*E_JointLimitType_Count);

			pDiskReader->ReadBytes(
				&(isLimited[0]),
				sizeof(bool)*E_JointLimitType_Count);
#endif
		}
	}
	if (useOuterBlockHeaders)
	{
		pDiskReader->ExitBlock();
	}
	return C_Status::E_Status_Success;
}

