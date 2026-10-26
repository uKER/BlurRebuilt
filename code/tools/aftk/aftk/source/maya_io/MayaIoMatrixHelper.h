#ifndef MAYA_IO_MATRIX_HELPER_H
#define MAYA_IO_MATRIX_HELPER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class MatrixHelper
	{
	public:
		static bool SetMatrix(
			Aftk::C_SceneNode* pAftkSceneNode, 
			const MDagPath& transformDagPath,
			F32 absNumericRange);

		static bool GetMatrix(
			MMatrix& mMatrix,
			Aftk::E_RotationOrder& rotationOrder,
			const MDagPath& transformDagPath,
			F32 absNumericRange);

		static bool GetLocalPosition(
			Aftk::Vector3F32& localPosition, 
			const MFnDagNode& dagNode);

		static bool MayaToAftkMatrix44(
			Aftk::Matrix44& aftkMatrix, 
			const MMatrix& mayaMatrix,
			F32 absNumericRange);

		static void AftkToMayaMatrix44(
			const Aftk::Matrix44& aftkMatrix, 
			MMatrix& mayaMatrix);
	};
}

#endif
