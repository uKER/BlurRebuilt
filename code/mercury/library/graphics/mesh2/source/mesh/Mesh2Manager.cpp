#include "stdafx.h"
#include "Mesh2/Mesh/Mesh2Manager.h"
#include "Mesh2/Mesh/Mesh2Utils.h"
#include "Mesh2/Mesh/Mesh2Render.h"

namespace Render
{

// C_Mesh2Instance

C_Mesh2Instance::C_Mesh2Instance(C_Mesh2Manager* pManager) : m_Visible(true), m_pMesh(NULL), m_pManager(pManager)
{
}

void C_Mesh2Instance::Render()
{
	if(!m_pMesh)
		return;

	//RenderMesh2(m_pMesh, m_pManager->m_pDevice);
}

// C_Mesh2Manager

C_Mesh2Manager::C_Mesh2Manager(Display2::C_Device* pDevice) : m_pDevice(pDevice)
{
}

C_Mesh2Manager::~C_Mesh2Manager()
{
	Clear();
}

void C_Mesh2Manager::Clear()
{
	for(S32 i=0; i<m_Instances.Size(); i++)
		SAFE_DELETE_PTR(m_Instances[i]);
	m_Instances.Clear();

	for(S32 i=0; i<m_Meshes.Size(); i++)
		SAFE_DELETE_PTR(m_Meshes[i]);
	m_Meshes.Clear();
}

void C_Mesh2Manager::Destroy()
{
	for(S32 i=0; i<m_Meshes.Size(); i++)
		DestroyMesh2(m_Meshes[i], m_pDevice);
}

C_Mesh2* C_Mesh2Manager::AddMesh(const Util::C_String& filePath, const Util::C_String& fileRoot)
{
	C_Mesh2* pMesh = new C_Mesh2();
	if(LoadMesh2(pMesh, filePath, fileRoot, m_pDevice))
	{
		m_Meshes.PushBack(pMesh);
	}
	else
	{
		SAFE_DELETE_PTR(pMesh);
	}
	return pMesh;
}

C_Mesh2Instance* C_Mesh2Manager::AddInstance(C_Mesh2* pMesh, const F32* worldMatrix)
{
	C_Mesh2Instance* pInstance = new C_Mesh2Instance(this);
	memcpy(pInstance->m_WorldMatrix, worldMatrix, sizeof(F32)*16);
	pInstance->m_pMesh = pMesh;
	m_Instances.PushBack(pInstance);
	return pInstance;
}

C_Mesh2Instance* C_Mesh2Manager::AddMeshAndInstance(const Util::C_String& filePath, const Util::C_String& fileRoot, const F32* worldMatrix)
{
	C_Mesh2* pMesh = AddMesh(filePath, fileRoot);
	C_Mesh2Instance* pInstance = AddInstance(pMesh, worldMatrix);
	return pInstance;
}

} // namespace Render
