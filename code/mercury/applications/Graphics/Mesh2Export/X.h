#ifndef X_H
#define X_H

#pragma once

#include "D3D.h"

struct C_XMaterial
{
	Util::C_String		m_TextureFileName;
};

struct C_XMesh
{
	C_XMesh();
	~C_XMesh();

	void Destroy();

	bool Load(const Util::C_String& filePath);

	bool Clean(const D3DXCLEANTYPE cleanType);
	bool Optimize(const DWORD flags);

	bool ComputeTangentFrame();

	DWORD				m_Options;
	DWORD				m_MaterialCount;
	C_XMaterial*		m_pMaterials;
	DWORD*				m_pAdjacency;
	ID3DXMesh*			m_pMesh;
};

#endif // X_H
