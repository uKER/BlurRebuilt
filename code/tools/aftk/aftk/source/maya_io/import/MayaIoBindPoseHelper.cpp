#include "stdafx.h"

using namespace MayaIo;

void C_BindPoseHelper::SetImporterPtr(C_Importer* pImporter)
{
	m_pImporter = pImporter;
}

Aftk::C_Status C_BindPoseHelper::GoIntoBindPose()
{
	Aftk::C_Status status;
	MStatus mStatus;

	//Save joint info.
    for (MItDag dagIt(MItDag::kDepthFirst, MFn::kTransform);
            !dagIt.isDone();
            dagIt.next())
    {
        MDagPath path;
		dagIt.getPath(path);

		bool doWeImportThisNode;
		bool doWeImportItsChildren;
		m_pImporter->DoWeImportThisNode(
			path, 
			doWeImportThisNode,
			doWeImportItsChildren,
			status);
		CHECK_ERROR_RETURN;
		if (!doWeImportThisNode)
			//Don't bother looking at this node.
			continue;

		if (path.isInstanced() && path.instanceNumber() != 0)
			// we only work on the first instance of an instanced joint.
			continue;

		if (m_pImporter->IsBonedInstanceJoint(path))
			continue;

//		std::string test = path.fullPathName().asChar();
		TransformInfo joint2Transform;
		joint2Transform.path = path;
//		OutputDebugString(test.c_str());
//		OutputDebugString("\n");

		if (path.hasFn(MFn::kJoint))
		{
			status = C_SceneJointImport::GetSSC(
				path,
				joint2Transform.segmentScaleCompensation);
			CHECK_ERROR_RETURN;
		}
		m_undoInfo.savedTransforms.PushBack(joint2Transform);
	}


    // Save current state
    m_undoInfo.ikSnap = MIkSystem::isGlobalSnap(&mStatus);
	MCHECK_ERROR_RETURN_1;
    m_undoInfo.ikSolve = MIkSystem::isGlobalSolve();

	// Turn IK off
    MIkSystem::setGlobalSnap(false);
    MIkSystem::setGlobalSolve(false);

	U32 index = 0;
    //Put joints into bind pose
    for (MItDag dagIt(MItDag::kDepthFirst, MFn::kTransform);
            !dagIt.isDone();
            dagIt.next())
    {
        MDagPath path;
		dagIt.getPath(path);

		bool doWeImportThisNode;
		bool doWeImportItsChildren;
		m_pImporter->DoWeImportThisNode(
			path, 
			doWeImportThisNode,
			doWeImportItsChildren,
			status);
		CHECK_ERROR_RETURN;
		if (!doWeImportThisNode)
			//Don't bother looking at this node.
			continue;

		if (path.isInstanced())
			// we only work on the first instance of an instanced joint.
			if (path.instanceNumber() != 0)
				continue;

		if (m_pImporter->IsBonedInstanceJoint(path))
			continue;

		MFnTransform joint(path.node());

		MTransformationMatrix currentTransform = joint.transformation(&mStatus);
		MCHECK_ERROR_CLEAN_UP_RETURN_1;

		m_undoInfo.savedTransforms[index].transform = currentTransform;

		bool foundBindPose;
		MMatrix bindPoseMatrix;
		GetBindPoseMatrixMethod1(joint, bindPoseMatrix, foundBindPose);
		if (!foundBindPose)
			GetBindPoseMatrixMethod2(joint, bindPoseMatrix, foundBindPose);

		joint.set(bindPoseMatrix);
		index++;
    }
   
	//Remember the change.
	m_inBindPose = true;
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_BindPoseHelper::UndoGoIntoBindPose()
{
	Aftk::C_Status status;
	MStatus mStatus;
	for (U32 iObject = 0; iObject < (U32)m_undoInfo.savedTransforms.Size(); iObject++)
	{
		MDagPath path = m_undoInfo.savedTransforms[iObject].path;
		MFnTransform trans(path, &mStatus);
		MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_CLEAN_UP_RETURN_END_1;
		mStatus = trans.set(m_undoInfo.savedTransforms[iObject].transform);
		MCHECK_BEGIN << TRANS_REVERT_BIND_POSE_FAIL MCHECK_ERROR_CLEAN_UP_RETURN_END_1;

        if (path.isInstanced())
        { 
			// we only work on the first instance of an instanced joint.
            if (path.instanceNumber() != 0)
                continue;
        }

		bool doWeImportThisNode;
		bool doWeImportItsChildren;
		m_pImporter->DoWeImportThisNode(
			path, 
			doWeImportThisNode,
			doWeImportItsChildren,
			status);
		CHECK_ERROR_RETURN;
		if (!doWeImportThisNode)
			//Don't bother looking at this node.
			continue;

		Aftk::C_SceneNode* pAftkSceneNode = 0;
		if (!m_pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(path, &pAftkSceneNode))
			continue;

		if (pAftkSceneNode->GetType() != Aftk::E_ObjType_SceneJoint)
			continue;

		C_SceneJointImport::SetSSC(
			pAftkSceneNode,
			m_undoInfo.savedTransforms[iObject].segmentScaleCompensation);
		CHECK_ERROR_RETURN;
    }

	// Set IK settings back to previous state.
    MIkSystem::setGlobalSnap(m_undoInfo.ikSnap);
    MIkSystem::setGlobalSolve(m_undoInfo.ikSolve);

	//Flag as being in bind pose.
	m_inBindPose = false;
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_BindPoseHelper::GetBindPoseMatrixMethod1(
	const MFnTransform& joint,
	MMatrix& matrix, 
	bool& foundBindPose)
{
	Aftk::C_Status status;
	MStatus mStatus;
    
	MPlug tempBindPosePlug = joint.findPlug("message", &mStatus);
//	MPlug tempBindPosePlug = joint.findPlug("bindPose", &mStatus);
	if (!mStatus)
	{
		foundBindPose = false;
		return Aftk::C_Status::E_Status_Success;
	}

    MPlugArray mapConnections;
    tempBindPosePlug.connectedTo(mapConnections, false, true);
//	tempBindPosePlug.connectedTo(mapConnections, true, true);

    if (mapConnections.length() != 1)
    {
		foundBindPose = false;
        //We couldn't find a bind-pose connection - use the joint's current transform.
        matrix = joint.transformation().asMatrix();
		return Aftk::C_Status::E_Status_Success;
    }

    // find the other end. actually we shouldn't call it "bindPosePlug",
    // but worldTransformPlug (as that's where it enters).

    // theoretically someone could bind the bindPose to other nodes,
    // than the bindPose-node. in this case there's a problem.
    MPlug bindPosePlug = mapConnections[0];

    // this node should be a "dagPose"-node (in case you want to look it
    // up in the help)
    MFnDependencyNode bindPoseNode(bindPosePlug.node());
	
	std::string typeName = bindPoseNode.typeName().asChar();
	if (typeName != "dagPose")
	{
		foundBindPose = false;
        //We couldn't find a bind-pose connection - use the joint's current transform.
        matrix = joint.transformation().asMatrix();
		return Aftk::C_Status::E_Status_Success;
	}

    // and as such, has the "xformMatrix"-attribute.
    MObject xformMatrixAttribute = bindPoseNode.attribute("xformMatrix");

    MPlug localTransformPlug(bindPosePlug.node(), xformMatrixAttribute);
    // xformMatrix is an array. to get our localmatrix we need to select
    // the same index, as our bindPosePlug (logicalIndex()).
    localTransformPlug.selectAncestorLogicalIndex(
                        bindPosePlug.logicalIndex(), xformMatrixAttribute);

    MObject localMatrixObject;
    localTransformPlug.getValue(localMatrixObject);
    // extract the matrix out of the object.
    matrix = MFnMatrixData(localMatrixObject).matrix();

	foundBindPose = true;
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_BindPoseHelper::GetBindPoseMatrixMethod2(
	const MFnTransform& joint,
	MMatrix& matrix,
	bool& foundBindPose)
{
	Aftk::C_Status status;
	MStatus mStatus;
    
	MPlug tempBindPosePlug = joint.findPlug("message", &mStatus);
	if (!mStatus)
	{
		foundBindPose = false;
		return Aftk::C_Status::E_Status_Success;
	}

    MPlugArray mapConnections;
    tempBindPosePlug.connectedTo(mapConnections, false, true);

    if (mapConnections.length() != 1)
    {
        //We couldn't find a bind-pose connection - use the joint's current transform.
        matrix = joint.transformation().asMatrix();
		return Aftk::C_Status::E_Status_Success;
    }

    // find the other end. actually we shouldn't call it "bindPosePlug",
    // but worldTransformPlug (as that's where it enters).

    // theoretically someone could bind the bindPose to other nodes,
    // than the bindPose-node. in this case there's a problem.
    MPlug bindPosePlug = mapConnections[0];

    // this node should be a "dagPose"-node (in case you want to look it
    // up in the help)
    MFnDependencyNode bindPoseNode(bindPosePlug.node(), &mStatus);
	MCHECK_ERROR_RETURN_1;

    // and as such, has the "xformMatrix"-attribute.
    MObject xformMatrixAttribute = bindPoseNode.attribute("xformMatrix", &mStatus);
	if (!mStatus)
	{
		foundBindPose = false;
		return Aftk::C_Status::E_Status_Success;
	}

    MPlug localTransformPlug(bindPosePlug.node(), xformMatrixAttribute);
    // xformMatrix is an array. to get our localmatrix we need to select
    // the same index, as our bindPosePlug (logicalIndex()).
    mStatus = localTransformPlug.selectAncestorLogicalIndex(
                        bindPosePlug.logicalIndex(), xformMatrixAttribute);
	MCHECK_ERROR_RETURN_1;

    MObject localMatrixObject;
    localTransformPlug.getValue(localMatrixObject);
    // extract the matrix out of the object.
    matrix = MFnMatrixData(localMatrixObject).matrix();

	foundBindPose = true;
	return Aftk::C_Status::E_Status_Success;
}

void C_BindPoseHelper::CleanUpAfterFailure()
{
	UndoGoIntoBindPose();
}

Aftk::C_Status C_BindPoseHelper::ImportBindPoses()
{
	Aftk::C_Status status;
	MStatus mStatus;

    for (MItDag dagIt(MItDag::kDepthFirst, MFn::kTransform);
            !dagIt.isDone();
            dagIt.next())
    {
        MDagPath path;
        dagIt.getPath(path);
        if (path.isInstanced() && (path.instanceNumber() != 0))
			// we only work on the first instance of an instanced joint.
			continue;

		bool doWeImportThisNode;
		bool doWeImportItsChildren;
		m_pImporter->DoWeImportThisNode(
			path, 
			doWeImportThisNode,
			doWeImportItsChildren,
			status);
		CHECK_ERROR_RETURN;
		if (!doWeImportThisNode)
			//Don't bother looking at this node.
			continue;

		Aftk::C_SceneNode* pAftkSceneNode = 0;
		if (!m_pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(path, &pAftkSceneNode))
			continue;
		
		//Our node has been reset in to its bind pose.
		pAftkSceneNode->SetBasePose(pAftkSceneNode->GetWorldMatrix());
	}

	return Aftk::C_Status::E_Status_Success;
}

