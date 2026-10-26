#include <nexus_render/stdafx.h>
#include <nexus_render/camera.h>

using namespace NexusRender;

MeshBasicEffectWrapper::MeshBasicEffectWrapper() : MeshEffectWrapper()
{
	m_mViewHandle = 0;
	m_mPaletteTransposedInverseHandle = 0;
}

MeshBasicEffectWrapper::~MeshBasicEffectWrapper()
{
}

Tu::Status MeshBasicEffectWrapper::Build(D3DShaderManager* pShaderManager)
{
	Tu::Status status;
	status = __super::Build(pShaderManager);
	if (!status) return status;

	//Build variable handles.
	m_mWorldTransposedInverseHandle = 
		m_pEffect->GetParameterByName(0, "g_mWorldTransposedInverse");
	ASSERT(m_mWorldTransposedInverseHandle);

	m_mViewHandle = 
		m_pEffect->GetParameterByName(0, "g_mView");

	m_mPaletteTransposedInverseHandle = 
		m_pEffect->GetParameterByName(0, "g_mPaletteTransposedInverse");
	ASSERT(m_mPaletteTransposedInverseHandle);

	return Tu::Status::SUCCESS;
}

Tu::Status MeshBasicEffectWrapper::SetTechnique(
	const SceneRendererSettings& settings, 
	D3DManager::EVertexDecl decl,
	IRenderMeshNode::EEffectMode mode)
{
	static char techniqueName[256];
	const char* modeStr = IRenderMeshNode::GetEffectModeAsString(mode);
	const char* declStr = D3DManager::GetVertexDeclAsString(decl);
	//Effect mode
	strcpy(techniqueName, modeStr);
	strcat(techniqueName, "_");
	//Vertex declaration
	strcat(techniqueName, declStr);

	//Component mode.
	if (settings.componentMode != 0)
	{
		strcat(techniqueName, "_CM");
		static char componentModeStr[11];
		itoa(settings.componentMode, componentModeStr, 10);
		strcat(techniqueName, componentModeStr);
	}

	//Real-time lighting
	if (settings.useRealTimeLighting)
		strcat(techniqueName, "_RTL1");

	if (FAILED(m_pEffect->SetTechnique(techniqueName)))
		return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TECHNIQUE);

	return Tu::Status::SUCCESS;
}

const char* MeshBasicEffectWrapper::GetEffectName() const
{
	#ifdef LOAD_SHADERS_FROM_FILE
		static const char* pEffectName = "../nexus_render/Shaders/MeshBasic.fx";
	#else
		static const char* pEffectName = "IDR_MESH_BASIC_EFFECT";
	#endif
	return pEffectName;
}

void MeshBasicEffectWrapper::UpdateCameraConstants(const Camera& camera)
{
	D3DXMATRIX m;
	camera.GetViewMatrix(m);
	m_pEffect->SetMatrix(m_mViewHandle, &m);
}

void MeshBasicEffectWrapper::UpdateNodeConstants(const RenderNode& renderNode)
{
	__super::UpdateNodeConstants(renderNode);

	m_pEffect->SetMatrix(
		m_mWorldTransposedInverseHandle, 
		&(D3DXMATRIX)renderNode.GetWorldMatrixTransposedInverse());

	if (renderNode.GetType() == RenderNode::RENDER_NODE_TYPE_MESH_BONED_INSTANCE)
	{
		const RenderMeshBonedInstanceNode& bonedInstance = 
			(const RenderMeshBonedInstanceNode&)renderNode;
		{
			//Upload matrix palette and matrix palette transposed inverse.
			D3DXMATRIX temp[2];
			temp[0] = (D3DXMATRIX)bonedInstance.GetPaletteMatricesTransposedInversePtr()[0];
			temp[1] = (D3DXMATRIX)bonedInstance.GetPaletteMatricesTransposedInversePtr()[1];
			m_pEffect->SetMatrixArray(
				m_mPaletteTransposedInverseHandle,  
				temp,
				2);
		}
	}
}

EffectWrapper::EEffectType MeshBasicEffectWrapper::GetType() const
{
	return EffectWrapper::EFFECT_TYPE_MESH_BASIC;
}
