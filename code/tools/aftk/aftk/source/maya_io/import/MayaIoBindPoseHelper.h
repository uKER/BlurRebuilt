#ifndef MAYA_IO_BIND_POSE_HELPER_H
#define MAYA_IO_BIND_POSE_HELPER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
class C_Importer;

class C_BindPoseHelper
{
public:
	void SetImporterPtr(C_Importer* pImporter);

	Aftk::C_Status GoIntoBindPose();

	Aftk::C_Status UndoGoIntoBindPose();

	Aftk::C_Status ImportBindPoses();

	Aftk::C_Status GetBindPoseMatrixMethod1(
		const MFnTransform& joint,
		MMatrix& matrix, 
		bool& foundBindPose);

	Aftk::C_Status GetBindPoseMatrixMethod2(
		const MFnTransform& joint,
		MMatrix& matrix,
		bool& foundBindPose);

private:
	void CleanUpAfterFailure();

	struct TransformInfo
	{
		MDagPath path;
		MTransformationMatrix transform;
		bool segmentScaleCompensation;
	};

	struct UndoInfo
	{
		bool ikSnap;
		bool ikSolve;

		Ut::UtVector< Ut::DynamicVector<TransformInfo> > savedTransforms;
	};

	UndoInfo	m_undoInfo;
	bool		m_inBindPose;
	C_Importer*	m_pImporter;
};

}

#endif