#ifndef NAVMESHDATACONVERTER_H
#define NAVMESHDATACONVERTER_H

namespace NavMesh
{
	class NavMeshNode;

	class C_Converter
	{
	public:
		static NavMeshNode* Process(const Aftk::C_SceneNode* pAftkSceneMeshNode);

	};
}
#endif