#ifndef SPLINE_RENDERER_H
#define SPLINE_RENDERER_H

namespace NexusRender
{
	/*
	Tu::Status SplineRenderer::UpdateScene(U32 nVerts)
	{
		m_nGizmoVerts = nGizmoVerts;

		D3DCAPS9 caps;
		D3DManager::GetDevicePtr()->GetDeviceCaps(&caps);

		//Determine how many VBs will be needed.
		m_maxVBSize = caps.MaxPrimitiveCount*2;
		if (m_maxVBSize > NEXUS_RENDER_GIZMO_RENDERER_MAX_VERTS_PER_VB)
			m_maxVBSize = NEXUS_RENDER_GIZMO_RENDERER_MAX_VERTS_PER_VB;
		
		if (m_maxVBSize < nVerts)
			//This should never happen, we should always be able to fit at least 6 verts in a VB.
			return Tu::Status::FAILURE;

		//Allocate space for our VB pointers.
		U32 iVB = 0;
		while(nVerts > 0)
		{
			U32 vbSize = nVerts;
			//Clamp to out maximum allowable VB size.
			if (vbSize > m_maxVBSize)
				vbSize = m_maxVBSize;
			//Make sure we can fit an entire node into a single VB. We don't want straddling of VBs.
			vbSize -= vbSize % nGizmoVerts;

			m_vertexBuffers.PushBack();
			//Create our vertex buffer of size vbSize.
			if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
				vbSize * sizeof(D3DVertexPosCol),
				D3DUSAGE_WRITEONLY, 
				0,
				D3DPOOL_MANAGED, 
				&m_vertexBuffers[iVB].pBuffer,
				NULL)))
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
			//Set the number of vertices in the vertex buffer.
			m_vertexBuffers[iVB].size = vbSize;

			//Decrement the number of remaining vertices.
			numVerts -= vbSize;
			iVB++;
		}

		return Tu::Status::SUCCESS;
	}

	void SplineRenderer::Clear()
	{
		//Destroy any vertex buffers that we might have.
		for (U32 iVB = 0; iVB < (U32)m_vertexBuffers.Size(); iVB++)
		{
			m_vertexBuffers[iVB].pBuffer->Release();
		}

		m_vertexBuffers.Clear();
	}

	void SplineRenderer::LockVertexBuffer(U32 index, D3DVertexPosCol*& pVertexBufferContents)
	{
		if (FAILED(m_vertexBuffers[index].pBuffer->Lock( 
			0, 
			m_vertexBuffers[index].size * sizeof(D3DVertexPosCol), 
			(void**)&pVertexBufferContents, 
			0)))
			ASSERT(false);
	}

	void SplineRenderer::UnlockVertexBuffer(U32 index)
	{
		if (FAILED(m_vertexBuffers[index].pBuffer->Unlock()))
			ASSERT(false);
	}

	Tu::Status SplineRenderer::Update(const Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes)
	{
		if (m_vertexBuffers.Size() == 0)
			return Tu::Status::SUCCESS;

		Tu::Status status;

		U32 currentVB = 0;
		U32 indexInVB = 0;

		D3DVertexPosCol* pVertexBufferContents;
		LockVertexBuffer(currentVB, pVertexBufferContents);

		bool vbLocked = true;
		for (U32 iNode = 0; iNode < (U32)renderNodes.Size(); iNode++)
		{
			if (!vbLocked)
			{
				LockVertexBuffer(currentVB, pVertexBufferContents);
				vbLocked = true;
			}
			RenderNode* pRenderNode = renderNodes[iNode];

			//Only bother zeroing vertices for invisible nodes if they've not been culled.
			if (!pRenderNode->IsVisible() && (!pRenderNode->GetNodeFlags().isCulled))
			{
				for (U32 j = 0; j < m_nGizmoVerts; j++)
				{
					D3DVertexPosCol* pVertex = &(pVertexBufferContents[indexInVB]);
					pVertex->SetPosition(TMathsNM::HVec3(0,0,0));
					indexInVB++;
				}
				continue;
			}

			ASSERT(indexInVB + m_nGizmoVerts < m_maxVBSize);
			ResetGizmo(
				pVertexBufferContents,
				indexInVB,
				m_nGizmoVerts,
				pRenderNode);
			ASSERT(m_vertexBuffers[currentVB].size);

			indexInVB += m_nGizmoVerts;
			if (indexInVB+m_nGizmoVerts >= m_maxVBSize)
			{
				UnlockVertexBuffer(currentVB);
				currentVB++;
				indexInVB = 0;
				vbLocked = false;
			}
		}
		if (vbLocked)
			UnlockVertexBuffer(currentVB);

		return Tu::Status::SUCCESS;
	}

		Tu::Status GizmoRenderer::Draw();
	}
	*/
}

#endif
