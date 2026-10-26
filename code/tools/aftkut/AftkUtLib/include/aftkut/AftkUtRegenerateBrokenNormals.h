#ifndef AFTK_UT_REGENERATE_BROKEN_NORMALS
#define AFTK_UT_REGENERATE_BROKEN_NORMALS

namespace AftkUt
{
	class C_RegenerateBrokenNormals
	{

	public:
		static Aftk::C_Status Regenerate(
			Aftk::C_SceneMeshNode *pSceneMeshNode,
			F32 threshold = 30.f);

		static Aftk::C_Status Regenerate(
			RadialEdgeMesh::C_Mesh *pRadialMesh,
			Aftk::C_SceneMeshNode *pSceneMeshNode);
	};
}

#endif

