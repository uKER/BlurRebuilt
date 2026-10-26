#pragma once

class CGenerateFaceNormals
{
public:
	CGenerateFaceNormals() {};
	~CGenerateFaceNormals() {};
	
	Aftk::C_SceneMeshNode	*m_pMesh;

	bool				GenerateFaceNormals(Aftk::C_SceneMeshNode *pMesh);
};

