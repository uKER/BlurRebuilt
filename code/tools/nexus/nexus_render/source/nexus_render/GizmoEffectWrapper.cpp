#include <nexus_render/stdafx.h>
#include <nexus_render/camera.h>

using namespace NexusRender;

GizmoEffectWrapper::GizmoEffectWrapper()
{
}

Tu::Status GizmoEffectWrapper::Build(D3DShaderManager* pShaderManager)
{
	Tu::Status status;
	status = __super::Build(pShaderManager);
	if (!status) return status;

	//Build variable handles.
	m_mViewProjectionHandle = 
		m_pEffect->GetParameterByName(0, "g_mViewProjection");
	ASSERT(m_mViewProjectionHandle);

	m_mWorldHandle =
		m_pEffect->GetParameterByName(0, "g_mWorld");
	ASSERT(m_mWorldHandle);

	return Tu::Status::SUCCESS;
}

Tu::Status GizmoEffectWrapper::SetTechnique(
	const SceneRendererSettings& settings,
	D3DManager::EVertexDecl vertexDecl,
	IRenderMeshNode::EEffectMode mode)
{
	if (FAILED(m_pEffect->SetTechnique("Gizmo")))
		return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TECHNIQUE);
	return Tu::Status::SUCCESS;
}

void GizmoEffectWrapper::UpdateCameraConstants(const Camera& camera)
{
	D3DXMATRIX viewMatrix;
	camera.GetViewMatrix(viewMatrix);
	D3DXMATRIX projMatrix;
	camera.GetProjectionMatrix(projMatrix);

	D3DXMATRIX viewProjectionMatrix = viewMatrix * projMatrix;
	m_pEffect->SetMatrix(m_mViewProjectionHandle, &viewProjectionMatrix);

	//Set world matrix to identity as this is not always used.
	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	m_pEffect->SetMatrix(
		m_mWorldHandle,
		&id);
}

void GizmoEffectWrapper::UpdateNodeConstants(const RenderNode& renderNode)
{
	ASSERT(m_pEffect);
	//Update the world matrix
	m_pEffect->SetMatrix(
		m_mWorldHandle, 
		&(D3DXMATRIX&)renderNode.GetWorldMatrix());
}

void GizmoEffectWrapper::UpdateMaterialConstants(
	SceneRenderer& sceneRenderer,
	const EffectConstants& constants) 
{
}

