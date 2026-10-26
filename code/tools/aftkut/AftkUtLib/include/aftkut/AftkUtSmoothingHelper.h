#ifndef AFTK_UT_SMOOTHING_HELPER_H
#define AFTK_UT_SMOOTHING_HELPER_H

namespace AftkUt
{
	class C_SmoothingHelper
	{
	public:
		static bool CanSmooth(
			const Aftk::Vector3F32& v0, 
			const Aftk::Vector3F32& v1, 
			F32 magnitudeScale,
			F32 angularTolerance,
			bool normalise);

		// Checks system actor properties/polyflag
		static bool ShouldSmooth(
			const Aftk::C_SceneMeshNode* pMeshNode);

		static bool ShouldSmooth(
			const Aftk::C_Polygon& polygon);

		//gets the 2 adjacent edges of a triangle at the given point
		static bool GetEdgeVectors(
			const Aftk::C_Polygon& polygon,
			U32 polyVertIndex,	//index into mesh poly vert array, NOT the vertex index within the polygon
			Aftk::Vector3F32& v0,
			Aftk::Vector3F32& v1);

		//calculate weight using angle between triangle edges for tesselation independent smoothing results
		static F32 GetVertexWeight(
			const Aftk::Vector3F32& v0,
			const Aftk::Vector3F32& v1);

	};
}

#endif
