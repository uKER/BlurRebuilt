#include "stdafx.h"
#include "Mesh2/Mesh/Mesh2Render.h"
#include <System/Scheduler.h>

namespace Render
{

Debug::Int g_TextureFilterBias("Graphics.Mesh2.TextureFilterBias", 0, Debug::Range(-128, 127));

void RenderMesh2(C_Mesh2* pMesh, Display2::C_ContextNoCheckNoTrack* pContext)
{
	BIZ_ASSERT(pMesh);
	BIZ_ASSERT(pContext);

	#if PLATFORM_PS3 || PLATFORM_SPU
	U32 vertexElementsBitfield = UINT_MAX;
	#endif

#if PLATFORM_SPU
	for(S32 iBatch=0; iBatch<pMesh->m_BatchList.Size(); iBatch++)
	{
		C_Mesh2Batch batch STOREALIGN16;
		Spu::Fetch(&batch, &pMesh->m_BatchList[iBatch], sizeof(C_Mesh2Batch));
		
		if(batch.m_Material != -1)
		{
			C_Mesh2Material material STOREALIGN16;
			Spu::Fetch(&material, &pMesh->m_MaterialList[batch.m_Material], sizeof(C_Mesh2Material));

			for(S32 iSampler=0; iSampler<material.m_SamplerList.Size(); iSampler++)
			{
				C_Mesh2Sampler sampler STOREALIGN16;
				Spu::Fetch(&sampler, &material.m_SamplerList[iSampler], sizeof(C_Mesh2Sampler));

				Display2::C_SamplerState samplerState;
				samplerState.SetDefaultStates();
				samplerState.SetTextureAddress(Display2::E_TextureAddress_Wrap, Display2::E_TextureAddress_Wrap, Display2::E_TextureAddress_Wrap);
				samplerState.SetTextureFilter(Display2::E_TextureFilter_Linear, Display2::E_TextureFilter_Linear, Display2::E_TextureFilter_Linear);
				pContext->SetSamplerState(sampler.m_Number, samplerState);

				Display2::C_Texture texture STOREALIGN16;
				Spu::Fetch(&texture, &pMesh->m_TextureList[sampler.m_Texture], sizeof(Display2::C_Texture));

				Display2::C_TextureState textureState;
				textureState.SetDefaultStates();
				textureState.SetTexture(&texture);
				pContext->SetTextureState(sampler.m_Number, textureState);
			}
		}

		Display2::C_VertexDeclaration vertexDeclaration STOREALIGN16;
		Spu::Fetch(&vertexDeclaration, &pMesh->m_VertexDeclarationList[batch.m_VertexDeclaration], sizeof(Display2::C_VertexDeclaration));
		pContext->SetVertexDeclaration(
			&vertexDeclaration,
			&vertexElementsBitfield);

		Display2::C_VertexBuffer vertexBuffer STOREALIGN16;
		Spu::Fetch(&vertexBuffer, &pMesh->m_VertexBufferList[batch.m_VertexBuffer], sizeof(Display2::C_VertexBuffer));
		pContext->SetVertexBuffer(
			0,
			&vertexBuffer,
			0,
			vertexDeclaration.GetStride(0), // TODO: Precompute this!
			&vertexDeclaration);

		Display2::C_IndexBuffer indexBuffer STOREALIGN16;
		Spu::Fetch(&indexBuffer, &pMesh->m_IndexBufferList[batch.m_IndexBuffer], sizeof(Display2::C_IndexBuffer));
		pContext->SetIndexBuffer(
			&indexBuffer);

		pContext->DrawIndexedPrimitive(
			batch.m_PrimitiveType,
			batch.m_VertexOffset,
			batch.m_VertexStart,
			batch.m_VertexCount,
			batch.m_IndexStart,
			batch.m_IndexCount,
			indexBuffer.m_Format,
			Display2::FlagsToLocation(indexBuffer.m_Flags),
			indexBuffer.m_Offset);
	}
#else
	for(S32 iBatch=0; iBatch<pMesh->m_BatchList.Size(); iBatch++)
	{
		C_Mesh2Batch* pBatch = &pMesh->m_BatchList[iBatch];

		if(pBatch->m_Material != -1)
		{
			C_Mesh2Material* pMaterial = &pMesh->m_MaterialList[pBatch->m_Material];

			for(S32 iSampler=0; iSampler<pMaterial->m_SamplerList.Size(); iSampler++)
			{
				C_Mesh2Sampler* pSampler = &pMaterial->m_SamplerList[iSampler];

				Display2::C_SamplerState samplerState;
				samplerState.SetDefaultStates();
				samplerState.SetTextureAddress(Display2::E_TextureAddress_Wrap, Display2::E_TextureAddress_Wrap, Display2::E_TextureAddress_Wrap);
				samplerState.SetTextureFilter(Display2::E_TextureFilter_Linear, Display2::E_TextureFilter_Linear, Display2::E_TextureFilter_Linear);
#if PLATFORM_PS3
				samplerState.SetTextureLodBias(g_TextureFilterBias);
#endif
				pContext->SetSamplerState(pSampler->m_Number, samplerState);

				Display2::C_Texture* pTexture = &pMesh->m_TextureList[pSampler->m_Texture];

				Display2::C_TextureState textureState;
				textureState.SetDefaultStates();
				textureState.SetTexture(pTexture);
				pContext->SetTextureState(pSampler->m_Number, textureState);
			}
		}

		Display2::C_VertexDeclaration* pVertexDeclaration = &pMesh->m_VertexDeclarationList[pBatch->m_VertexDeclaration];
#if PLATFORM_PS3 || PLATFORM_SPU
		pContext->SetVertexDeclaration(
			pVertexDeclaration,
			&vertexElementsBitfield);
#else
		pContext->SetVertexDeclaration(
			pVertexDeclaration);
#endif

		Display2::C_VertexBuffer* pVertexBuffer = &pMesh->m_VertexBufferList[pBatch->m_VertexBuffer];
		pContext->SetVertexBuffer(
			0,
			pVertexBuffer,
			0,
			pVertexDeclaration->GetStride(0), // TODO: Precompute this!
			pVertexDeclaration);

		Display2::C_IndexBuffer* pIndexBuffer = &pMesh->m_IndexBufferList[pBatch->m_IndexBuffer];
		pContext->SetIndexBuffer(
			pIndexBuffer);

		pContext->DrawIndexedPrimitive(
			pBatch->m_PrimitiveType,
			pBatch->m_VertexOffset,
			pBatch->m_VertexStart,
			pBatch->m_VertexCount,
			pBatch->m_IndexStart,
			pBatch->m_IndexCount,
			pIndexBuffer);
	}
#endif
}

} // namespace Render
