#ifndef XSI_IO_ANIMATION_CURVE_IMPORT_H
#define XSI_IO_ANIMATION_CURVE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_SceneImport;

	class C_AnimationCurveImport
	{
	public:
		///
		///\brief Constructor.
		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			CSLFCurve* pXsiAnimCurve,
			Aftk::C_AnimationCurveNode* pAftkAnimCurve);
	};
}
#endif
