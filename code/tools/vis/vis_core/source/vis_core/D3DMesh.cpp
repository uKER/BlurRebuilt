#include "stdafx.h"

#include "vis_core\PolyRef.h"
#include "vis_core\D3DRenderer.h"
#include "vis_core\D3DMesh.h"
#include "vis_core\Util.h"
#include "vis_core\Mesh.h"

const D3DVERTEXELEMENT9 CD3DMesh::m_decl[] =
        {
            {0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
            D3DDECL_END()
        };

CD3DMesh::CD3DMesh()
{
	m_pD3DMesh = NULL;
}

CD3DMesh::~CD3DMesh()
{
	if (m_pD3DMesh)
	{
		m_pD3DMesh->Release();
		m_pD3DMesh = NULL;
	}
}

// create mesh from a vertex list
void CD3DMesh::CreateFromMesh(MeshNode *pInMesh)
{
	Ut::Vector<Vertex> &vertices = pInMesh->GetVertices();
	Ut::Vector<U16> &indices = pInMesh->GetIndices();

	HRESULT hr;
	IDirect3DVertexBuffer9 *pVertexBuffer;
	IDirect3DIndexBuffer9 *pIndexBuffer;
	unsigned int *pIndices;
	ID3DXMesh *pMesh;

	int nFaces = indices.Size()/3;

	// create mesh
	hr = D3DXCreateMesh(nFaces,vertices.Size(),D3DXMESH_MANAGED,m_decl,CD3DRenderer::m_pD3DDevice,&pMesh);

	// get vertex buffer
	pMesh->GetVertexBuffer(&pVertexBuffer);

	// lock vertex buffer
	Vertex *pVerts;
	pVertexBuffer->Lock(0, 0, (void**)&pVerts, 0);

	// copy over vertex data
	memcpy(pVerts, &vertices[0], vertices.Size() * sizeof(Vertex));

	pVertexBuffer->Unlock();
	pVertexBuffer->Release();

	// get face index buffer
	pMesh->GetIndexBuffer(&pIndexBuffer);

	// lock face index buffer
	pIndexBuffer->Lock(0,0,(void**)&pIndices,0);

	memcpy(pIndices, &indices[0], indices.Size() * sizeof(U16));

	pIndexBuffer->Unlock();
	pIndexBuffer->Release();

	// generate an attribute buffer 
	{
		DWORD* pAttr;
		pMesh->LockAttributeBuffer(0, &pAttr);
		//Map from batch key to batch number.
		Ut::OpenHashMap<U32, U32>	m_batchIds;
		for (U32 i = 0; i < nFaces; i++)
		{
			//Just look at the flag for the first ver
			FaceFlags flags = pInMesh->GetFaceFlags(i);

			//Use our flag bits to determine our batchId.
			U32 batchKey = flags.bits & FACE_FLAG_MASK;
			
			//Track the batch ids we're using.
			Ut::OpenHashMap<U32, U32>::InsertionReturnType result = m_batchIds.Insert(batchKey);
			//If we've not encountered this batch before...

			//Determine our dense batch index.
			U32 batchIndex;
			if (result.second)
			{
				result.first->GetValue() = (U32)m_batchFlags.Size();
				batchIndex = (U32)m_batchFlags.Size();
				m_batchFlags.PushBack(flags);
			}
			else
			{
				//We've had this batch before.
				batchIndex = result.first->GetValue();
			}

			//Use our batch id in our attribute buffer.
			pAttr[i] = batchIndex;
		}
	}	
	pMesh->UnlockAttributeBuffer();

	m_pD3DMesh = pMesh;

	// generate adjacency and optimize the mesh for drawing on the current hardware
	DWORD *pAdjacency = new DWORD [indices.Size()];

	pMesh->GenerateAdjacency(0.0001, pAdjacency);

	pMesh->OptimizeInplace(
		D3DXMESHOPT_COMPACT|D3DXMESHOPT_VERTEXCACHE,
		pAdjacency,
		NULL, 
		NULL, 
		NULL);

	delete pAdjacency;
}

ID3DXMesh* CD3DMesh::GetD3DXMesh()
{
	return m_pD3DMesh;
}

void CD3DMesh::SetD3DXMesh(ID3DXMesh* pMesh)
{
	m_pD3DMesh = pMesh;
}

// render this mesh's vertex buffer
void CD3DMesh::Render()
{
	//If we've got no batches we're probably just rendering a bounding box.
	if (m_batchFlags.Size() == 0)
	{
		m_pD3DMesh->DrawSubset(0);
		return;
	}

	for (U32 iBatch = 0; iBatch < (U32)m_batchFlags.Size(); iBatch++)
	{
		//Grab the flags that will determine our rendering state.
		FaceFlags faceFlags = m_batchFlags[iBatch];

		// if we have alpha triangles, draw them in wireframe so they're see-through.
		if (faceFlags.alpha)
			CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		// Double-sided rendering is required for the tree sprites.
		if (faceFlags.doubleSided)
			CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		
		//Actually render our batch.
		m_pD3DMesh->DrawSubset(iBatch);

		//Restore our state.
		if (faceFlags.alpha)
			CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		if (faceFlags.doubleSided)
			CD3DRenderer::m_pInterface->m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}
}
