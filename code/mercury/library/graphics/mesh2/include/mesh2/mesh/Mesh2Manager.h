#ifndef RENDER_MESH2_MANAGER_H
#define RENDER_MESH2_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Mesh2/Mesh/Mesh2.h"
#include <Display2/Device/Device.h>

namespace Render
{

struct C_Mesh2Manager;

// C_Mesh2Instance

struct C_Mesh2Instance
{
	C_Mesh2Instance(C_Mesh2Manager* pManager);

	void Render();

	bool												m_Visible;
	F32													m_WorldMatrix[16];
	C_Mesh2*											m_pMesh;
	C_Mesh2Manager*										m_pManager;
};

// C_Mesh2Manager

struct C_Mesh2Manager
{
	C_Mesh2Manager(Display2::C_Device* pDevice);
	~C_Mesh2Manager();

	void Clear();
	void Destroy();
	
	C_Mesh2* AddMesh(const Util::C_String& filePath, const Util::C_String& fileRoot);
	C_Mesh2Instance* AddInstance(C_Mesh2* pMesh, const F32* worldMatrix);
	C_Mesh2Instance* AddMeshAndInstance(const Util::C_String& filePath, const Util::C_String& fileRoot, const F32* worldMatrix);
	
	Display2::C_Device*									m_pDevice;
	Btl::Vector<C_Mesh2*>								m_Meshes;
	Btl::Vector<C_Mesh2Instance*>						m_Instances;
};

} // namespace Render

#endif // RENDER_MESH2_MANAGER_H
