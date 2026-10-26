#include "stdafx.h"
#include "Dynamic/Fog/Generic/BasicManager.h"
#include "Dynamic/Fog/Generic/BasicShapes.h"
#include "Dynamic/Fog/Manager.h"
#include "Dynamic/Fog/Volume.h"
#include "Dynamic/Node.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"

namespace DynamicEffect
{
namespace Fog
{

C_BasicManager::C_BasicManager(C_Renderer* pRenderer)
{
	m_pManager		= new C_Manager(pRenderer, E_ManagerTypeIndex_Fog);
	m_pRenderer		= pRenderer;

	m_pCuboid		= new C_ShapeCuboid(pRenderer, m_pManager, E_EffectTypeIndex_FogCuboid, E_EffectTypeIndex_FogCuboid);
	m_pCylinder		= new C_ShapeCylinder(pRenderer, m_pManager, E_EffectTypeIndex_FogCylinder, E_EffectTypeIndex_FogCylinder);
	m_pEllipsoid	= new C_ShapeEllipsoid(pRenderer, m_pManager, E_EffectTypeIndex_FogEllipsoid, E_EffectTypeIndex_FogEllipsoid);
}

C_BasicManager::~C_BasicManager()
{
	m_pRenderer->CleanEffect(m_pCuboid);
	m_pRenderer->CleanEffect(m_pCylinder);
	m_pRenderer->CleanEffect(m_pEllipsoid);

	SAFE_DELETE_PTR(m_pCuboid);
	SAFE_DELETE_PTR(m_pCylinder);
	SAFE_DELETE_PTR(m_pEllipsoid);

	SAFE_DELETE_PTR(m_pManager);
}

Volume* C_BasicManager::CreateVolume(const U32 typeIndex, const C_Node& localNode, const C_Node* pNode, const MathsStorage::Float4A16& vSize, const MathsStorage::Float4A16& vColour, const MathsStorage::Float2A16& vSettings)
{
	I_VolumeEffect* pVolumeEffect = (I_VolumeEffect*) m_pRenderer->FindEffect(typeIndex);

	Volume* pVolume = 0;
	if (pNode)
		pVolume = pVolumeEffect->NewLocalVolume();
	else
		pVolume = pVolumeEffect->NewVolume();

	pVolume->m_pEffect = pVolumeEffect;
	pVolume->m_iFlags = 0;
	pVolume->m_iVertexBuffer = 0xFFFF;
	pVolume->m_pNode = pNode;
	pVolume->m_vCentre = localNode.GetPosition();
	pVolume->m_vSize = vSize;
	pVolume->m_vRotation = localNode.GetRotation();	
	pVolume->m_vColour = vColour;
	pVolume->m_vSettings = vSettings;

	return pVolume;
}

}; // namespace Fog
}; // namespace DynamicEffect
