#ifndef AFTK_UT_NORMALISE_TEXTURE_COORDINATES_H
#define AFTK_UT_NORMALISE_TEXTURE_COORDINATES_H

namespace AftkUt
{
	class C_NormaliseTextureCoordinates
	{
	public:
		static Aftk::C_Status Normalise(Aftk::C_Scene* pScene);

		static Aftk::C_Status Normalise(Aftk::C_SceneMeshNode* pMesh);

		static bool IsPolygonTiled(U32 uvSet, const Aftk::C_Polygon* pPoly);

		static void PolygonTiling(
			U32 uvSet,
			const Aftk::C_Polygon* pPoly, 
			F32& tilingU,
			F32& tilingV);

	private:
	};
}

#endif
