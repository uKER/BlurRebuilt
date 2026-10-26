#ifndef AFTK_UT_SMOOTH_TANGENTS_H
#define AFTK_UT_SMOOTH_TANGENTS_H

namespace AftkUt
{
	class C_SmoothTangents
	{
	public:
		///
		///Angles are specified in degrees. This angle is the angle above which no more 
		///smoothing is performed. So an angle of 0 would give no smoothing, and an angle
		///of 180 would give maximum smoothing.
		static Aftk::C_Status Smooth(
			const RadialEdgeMesh::C_Mesh* pRadialMesh,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode, 
			F32 magnitudeScale, 
			F32 angularTolerance, 
			bool normalise);

		///
		///Angles are specified in degrees. This angle is the angle above which no more 
		///smoothing is performed. So an angle of 0 would give no smoothing, and an angle
		///of 180 would give maximum smoothing.
		static Aftk::C_Status Smooth(
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode, 
			F32 magnitudeScale, 
			F32 angularTolerance, 
			bool normalise);

	private:
		static Aftk::C_Status SmoothTangentsOnly(
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			const RadialEdgeMesh::C_Mesh* pRadialMesh,
			U32 iSet,
			U32 maxPVs,
			F32 magnitudeScale, 
			F32 angularTolerance,
			bool normalise);

		static Aftk::C_Status SmoothBinormalsOnly(
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			const RadialEdgeMesh::C_Mesh* pRadialMesh,
			U32 iSet,
			U32 maxPVs,
			F32 magnitudeScale, 
			F32 angularTolerance,
			bool normalise);
	};
}

#endif
