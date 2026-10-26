#ifndef MAYA_IO_DG_ANIMATION_CURVE_EXPORT_H
#define MAYA_IO_DG_ANIMATION_CURVE_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{

	class C_Exporter;

	class C_DgAnimationCurveExport : public C_DgNodeExport
	{
	public:

		static Aftk::C_Status Export(
			C_Exporter *pExporter,
			MFnAnimCurve& fnAnimCurve,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_AnimationCurveNode* pAftkAnimCurveNode);

	private:

		static Aftk::C_Status ExportHermiteCurve(
			MFnAnimCurve& fnAnimCurve,
			const Aftk::C_AnimationCurveNode* pAftkAnimCurveNode,
			const Aftk::C_Scene* pAftkScene);

		static Aftk::C_Status ExportLinearCurve(
			MFnAnimCurve& fnAnimCurve,
			const Aftk::C_AnimationCurveNode* pAftkAnimCurveNode,
			const Aftk::C_Scene* pAftkScene);

	};
}

#endif