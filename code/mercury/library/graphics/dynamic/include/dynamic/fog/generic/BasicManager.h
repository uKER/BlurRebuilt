#ifndef RENDER_DYNAMICEFFECT_FOG_GENERIC_BASICMANAGER_H
#define RENDER_DYNAMICEFFECT_FOG_GENERIC_BASICMANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace DynamicEffect
{

class C_Renderer;
class C_Node;

namespace Fog
{

class C_Manager;
class Volume;

class C_ShapeCuboid;
class C_ShapeCylinder;
class C_ShapeEllipsoid;

class C_BasicManager
{
public:
	
	C_BasicManager(C_Renderer* pRenderer);
	~C_BasicManager();

	Volume* CreateVolume(const U32 typeIndex, const C_Node& localNode, const C_Node* pNode, const MathsStorage::Float4A16& vSize, const MathsStorage::Float4A16& vColour, const MathsStorage::Float2A16& vSettings);

private:

	C_Manager*			m_pManager;
	C_Renderer*			m_pRenderer;

	C_ShapeCuboid*		m_pCuboid;
	C_ShapeCylinder*	m_pCylinder;
	C_ShapeEllipsoid*	m_pEllipsoid;

};

}; // namespace Fog
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_FOG_GENERIC_BASICMANAGER_H
