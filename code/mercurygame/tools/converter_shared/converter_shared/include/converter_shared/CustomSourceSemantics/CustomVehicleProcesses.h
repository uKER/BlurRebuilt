#ifndef APP_PREPROCESS_CUSTOM_VEHICLE_PROCESSES_H
#define APP_PREPROCESS_CUSTOM_VEHICLE_PROCESSES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "aftk/Aftk.h"

namespace Aftk { class C_Scene; }

namespace Vehicle
{

namespace Maintenance
{

/* 
===============================================================
					class C_ModelsToElements

NOTES: Converts models incorrectly defined in Art to elements
===============================================================
*/
class C_ModelsToElements
{
public:
	C_ModelsToElements() {}

	bool IsInteresting(Aftk::C_SceneNode* p_pSceneNode);

	Aftk::C_Status Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager);
};

} // namespace Maintenance

/* 
===============================================================
					class C_CreateInteriorGlass

NOTES: Find all windows on the vehicle and create "interior" versions of them
===============================================================
*/
class C_CreateInteriorGlass
{
public:
	C_CreateInteriorGlass() {}
	~C_CreateInteriorGlass() { m_pMeshNodes.Clear(); }

	Util::C_String CreateNewName(Util::C_String p_CurrentName);

	bool CreateInteriorVersion(Aftk::C_SceneMeshNode* p_pMeshNode);

	void CreateInteriorVersions();
	
	Btl::Vector<Aftk::C_SceneMeshNode*> m_pMeshNodes;
};

/* 
===============================================================
					class C_FindWindows

NOTES: Find all windows on the vehicle
===============================================================
*/
class C_FindWindows
{
public:
	C_FindWindows(C_CreateInteriorGlass* p_pCreator) { m_pCreator = p_pCreator; }

	bool IsInteresting(Aftk::C_SceneNode* p_pSceneNode);

	Aftk::C_Status Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager);

	C_CreateInteriorGlass*		m_pCreator;
};

/* 
===============================================================
					class C_InsertLODNodes

NOTES: Apply LOD settings by inserting LOD nodes into the car
===============================================================
*/
class C_InsertLODNodes
{
public:
	C_InsertLODNodes(Btl::Vector<Aftk::C_SceneNode*> *p) { m_pTargetNodes = p; }

	bool IsInteresting(Aftk::C_SceneNode* p_pSceneNode);

	Aftk::C_Status Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager);

	Btl::Vector<Aftk::C_SceneNode*> *m_pTargetNodes;

	void Apply(Btl::Vector<Aftk::C_SceneNode*> *p);

	void AddBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& attributeName);
};

/* 
===============================================================
					class C_TransformVertices

NOTES: Transform vertices to world space
===============================================================
*/
class C_TransformVertices
{
public:
	C_TransformVertices(F32 * pWheelPosition) {m_pWheelPivot = pWheelPosition;}

	bool IsInteresting(Aftk::C_SceneNode* p_pSceneNode);

	Aftk::C_Status Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager);

	F32*	m_pWheelPivot;
};

/* 
===============================================================
class C_ReflectCar

NOTES: Reflect car for wheel lighting calculation
===============================================================
*/
class C_ReflectCar
{
public:
	C_ReflectCar(Btl::Vector<Aftk::C_SceneMeshNode*> *p) {m_pTargetNodes = p;}

	bool IsInteresting(Aftk::C_SceneNode* p_pSceneNode);

	Btl::Vector<Aftk::C_SceneMeshNode*> *m_pTargetNodes;

	Aftk::C_Status Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager);

	void Apply(Aftk::C_Scene* pScene, Btl::Vector<Aftk::C_SceneMeshNode*> *p, F32 frontWheelPosition[2], F32 backWheelPosition[2]);
};

/* 
===============================================================
					class C_CustomVehicleProcesses

NOTES: 
===============================================================
*/
class C_CustomVehicleProcesses
{
public:
	
	static C_CustomVehicleProcesses& Instance() { return s_Instance; }

	C_CustomVehicleProcesses() : m_pAftkScene(NULL) {}

	void Process(Aftk::C_Scene* p_pAftkScene);

public:

	static C_CustomVehicleProcesses			s_Instance;

	Aftk::C_Scene*							m_pAftkScene;
};

} //namespace Vehicle

#endif




