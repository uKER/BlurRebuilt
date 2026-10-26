#ifndef FBX_IO_ANIMATION_CURVE_IMPORT_H
#define FBX_IO_ANIMATION_CURVE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace FbxIo
{
	class C_SceneImport;

	class C_AnimationCurveImport
	{
	public:

		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			KFCurve* pFbxAnimCurve,
			Aftk::C_AnimationCurveNode* pAftkAnimCurve);
	};
}
#endif
