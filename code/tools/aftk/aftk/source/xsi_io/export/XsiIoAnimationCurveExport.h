#ifndef XSI_IO_ANIMATION_CURVE_EXPORT_H
#define XSI_IO_ANIMATION_CURVE_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_SceneExport;

	class C_AnimationCurveExport
	{
	public:

		static Aftk::C_Status Export(
			C_SceneExport* pExporter,
			CSLFCurve* pXsiAnimCurve,
			const Aftk::C_AnimationCurveNode* pAftkAnimCurve,
			const Aftk::C_SceneNode* pAftkSceneNode);

	private:

		static Aftk::C_Status ExportLinearCurve(
			C_SceneExport* pExporter,
			CSLFCurve* pXsiAnimCurve,
			const Aftk::C_AnimationCurveNode* pAftkAnimCurve,
			const Aftk::C_SceneNode* pAftkSceneNode);

		static Aftk::C_Status ExportHermiteCurve(
			C_SceneExport* pExporter,
			CSLFCurve* pXsiAnimCurve,
			const Aftk::C_AnimationCurveNode* pAftkAnimCurve,
			const Aftk::C_SceneNode* pAftkSceneNode);

	};
}


#endif
