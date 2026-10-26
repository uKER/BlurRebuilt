#pragma once

/*
 * This class contains vertex buffers, rendering code etc. for one renderable D3D object
 *
 */

#include <vis_core/FaceFlags.h>
#include <vis_core/Vertex.h>
#include <vis_core/Mesh.h>

class CD3DMesh
{
public:
	CD3DMesh();
	~CD3DMesh();

	void		Render();
	
	void		CreateFromMesh(MeshNode* pMesh);

	ID3DXMesh*	GetD3DXMesh();

	void SetD3DXMesh(ID3DXMesh* pMesh);

private:
	static const D3DVERTEXELEMENT9 m_decl[];

	ID3DXMesh*	m_pD3DMesh;
	Ut::Vector<FaceFlags> m_batchFlags;
};

