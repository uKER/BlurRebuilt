#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeSceneCameraNode.h"
#include "NativeUtility.h"
#include "NativeIPlug.h"
#include "NativeNullPlug.h"
#include "NativeStaticPlug.h"
#include "NativeDynamicPlug.h"

using namespace Aftk;

C_Status C_NativeSceneCameraNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_SceneCameraNode* pSceneCameraNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_CAMERA_NODE);
	{
		//Write out the fragment of this node that comes from C_SceneNode
		{
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeSceneNode::Serialise(
				pDiskWriter, 
				pNativeFtk, 
				pSceneCameraNode,
				true);
			if (!status) return status;
		}

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_focalLength);
		if (!status) return status;
		
		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_centreOfInterest);
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_horizontalFilmAperture);
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_verticalFilmAperture);
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_shutterAngle);
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_nearClipPlane);
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_farClipPlane);
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_orthographicWidth);
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_focusDistance);
		if (!status) return status;

		status = C_NativeStaticPlug<F32>::Serialise(
			pDiskWriter,
			pNativeFtk,
			&pSceneCameraNode->m_fStop);
		if (!status) return status;
		
		*pDiskWriter << pSceneCameraNode->m_isOrthographic;
		*pDiskWriter << pSceneCameraNode->m_depthOfFieldEnabled;
	}
	if (useOuterBlockHeaders)
		pDiskWriter->EndBlock();

	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneCameraNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_SceneCameraNode* pSceneCameraNode,
	bool useOuterBlockHeaders)
{
	C_Status status;
	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_CAMERA_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find scene node block.");
		pDiskReader->EnterBlock();
	}
	{
		//Read in the fragment of this node that comes from C_SceneNode
		{
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeSceneNode::Deserialise(
				pDiskReader, 
				pNativeFtk, 
				pSceneCameraNode, true);
			if (!status) return status;
		}

		if (pDiskReader->GetVersion() < 201)
		{
			F32 verticalFov, horizontalFov;
			*pDiskReader >> verticalFov; 
			pSceneCameraNode->SetVerticalFieldOfView(verticalFov);
			*pDiskReader >> horizontalFov;
			pSceneCameraNode->SetHorizontalFieldOfView(horizontalFov);
		}
		else
		{
			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_focalLength);
			if (!status) return status;
			
			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_centreOfInterest);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_horizontalFilmAperture);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_verticalFilmAperture);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_shutterAngle);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_nearClipPlane);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_farClipPlane);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_orthographicWidth);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_focusDistance);
			if (!status) return status;

			status = C_NativeStaticPlug<F32>::Deserialise(
				pDiskReader,
				pNativeFtk,
				&pSceneCameraNode->m_fStop);
			if (!status) return status;
			
			*pDiskReader >> pSceneCameraNode->m_isOrthographic;
			if (pDiskReader->GetVersion() >= 206)
			{
				*pDiskReader >> pSceneCameraNode->m_depthOfFieldEnabled;
			}
		}
	}
	if (useOuterBlockHeaders)
		pDiskReader->ExitBlock();

	return C_Status::E_Status_Success;
}

