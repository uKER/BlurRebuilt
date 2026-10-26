#include <nexus_render/stdafx.h>

using namespace NexusRender;

MeshEffectWrapper::MeshEffectWrapper() : EffectWrapper()
{
	m_mWorldViewProjectionHandle = 0;
	m_mPaletteHandle = 0;
}

MeshEffectWrapper::~MeshEffectWrapper()
{
}

Tu::Status MeshEffectWrapper::Build(D3DShaderManager* pShaderManager)
{
	Tu::Status status;
	//Calling a pure virtual function here.
	status = LoadEffect(pShaderManager);
	if (!status) return status;

	//Build variable handles.
	m_mWorldViewProjectionHandle = 
		m_pEffect->GetParameterByName(0, "g_mWorldViewProjection");
	ASSERT(m_mWorldViewProjectionHandle);
	m_mPaletteHandle = 
		m_pEffect->GetParameterByName(0, "g_mPalette");
	ASSERT(m_mPaletteHandle);
	
	return Tu::Status::SUCCESS;
}

void MeshEffectWrapper::UpdateNodeConstants(const RenderNode& renderNode)
{
	ASSERT(m_pEffect);
	//Update the world*view*projection matrix
	m_pEffect->SetMatrix(m_mWorldViewProjectionHandle, &(D3DXMATRIX&)renderNode.GetWorldViewProjectionMatrix());

	if (renderNode.GetType() == RenderNode::RENDER_NODE_TYPE_MESH_BONED_INSTANCE)
	{
		const RenderMeshBonedInstanceNode& bonedInstance = 
			(const RenderMeshBonedInstanceNode&)renderNode;
		{

			//Upload matrix palette and matrix palette transposed inverse.
			D3DXMATRIX temp[2];
			temp[0] = (D3DXMATRIX&)bonedInstance.GetPaletteMatricesPtr()[0];
			temp[1] = (D3DXMATRIX&)bonedInstance.GetPaletteMatricesPtr()[1];
			m_pEffect->SetMatrixArray(
				m_mPaletteHandle,
				temp, 
				2);
		}
	}
}

