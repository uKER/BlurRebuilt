/////////////////////////////////////////////////////////////
// VehicleModel
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "system/automation/automation.h"
#include <system/memory.h>

#include "Vehicle/VehicleModel.h"
#include "Vehicle/PartCodes.h"
#include "Vehicle/Setup.h"
#include "Vehicle/RenderLayer.h"
#include "Vehicle/GameLayer.h"
#include "Vehicle/Damage/DamageLayer.h"
#include "Vehicle/MessageHandler.h"
#include "Vehicle/VehicleModelManager.h"
#include "Vehicle/CodeProcessor.h"
#include "Vehicle/Material/Material.h"
#include "Vehicle/Db/VehicleXt.h"

#include "system/serialise/c_filesystem.h"
#include "system/serialise/c_fileblock.h"

#include "SceneGraph/SceneGraph.h"
#include "SceneGraph/VisibleDetect.h"

#include "world_render/ModelInstance.h"
#include "world_render/LightSamplePointScene.h"

#include <Framework/application/applicationbase.h>
#include <Display/Buffer/VertexDeclarations.h>

#include <gravity/sdk/Body.h>
#define RENDERINGDATA_ALLOW_INCLUDE
#include <renderingdata\renderingdata.h>
#undef RENDERINGDATA_ALLOW_INCLUDE
#define ENABLE_SPU_DAMAGE

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

//#include <gravity/importer/Importer.h>

Debug::Logger &g_VehicleModelLog( Debug::GetLogger("mercury.vehicle.model") );

Debug::Float g_fVehicleSHScale("Vehicle.SHScale", 0.5f);

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

//#pragma optimize("", off)

//#include "world_render/RenderLevel.h"

namespace Vehicle
{

C_VehicleVertexBuffer::C_VehicleVertexBuffer()
{
	m_Used = false;
}

C_VehicleVertexBufferManager C_VehicleVertexBufferManager::s_Instance;

C_VehicleVertexBufferManager::C_VehicleVertexBufferManager()
{
	#if !CFG_GOLD
	m_Size = 0;
	#endif
}

C_VehicleVertexBuffer* C_VehicleVertexBufferManager::FindVertexBuffer(const U32 size)
{
	Btl::Set<C_VehicleVertexBuffer*>::Iterator it = m_List.Begin();
	while(it != m_List.End())
	{
		C_VehicleVertexBuffer* pVertexBuffer = *it;
		it++;
		if(!pVertexBuffer->m_Used && pVertexBuffer->m_Data.GetSize() == size) // Only reuse a vertex buffer  that isn't already used and is the same size
			return pVertexBuffer;
	}
	return NULL;
}

C_VehicleVertexBuffer* C_VehicleVertexBufferManager::FindVertexBuffer(C_VehicleVertexBuffer* pVertexBuffer)
{
	Btl::Set<C_VehicleVertexBuffer*>::Iterator it = m_List.Find(pVertexBuffer);
	return it != m_List.End() ? *it : NULL;
}

C_VehicleVertexBuffer* C_VehicleVertexBufferManager::CreateVertexBuffer(const U32 size)
{
	BIZ_ASSERT(size > 0);
	C_VehicleVertexBuffer* pVertexBuffer = FindVertexBuffer(size);
	if(pVertexBuffer)
	{
		#if !CFG_GOLD
		//LOGINFO(g_Log, "Reusing cached vehicle vertex buffer of size %i (bufferCount %i vertexSize %i)\n", size, m_List.Size(), m_Size);
		#endif

		pVertexBuffer->m_Used = true; // Found a matching vertex buffer so make sure its flagged as used
	}
	else
	{
		pVertexBuffer = new C_VehicleVertexBuffer();
		if(Display2::Device()->CreateVertexBuffer(size, 0, &pVertexBuffer->m_Data, NULL))
		{
			#if !CFG_GOLD
			m_Size += pVertexBuffer->m_Data.GetSize();
			//LOGINFO(g_Log, "Creating new vehicle vertex buffer of size %i (bufferCount %i vertexSize %i)\n", size, m_List.Size(), m_Size);
			#endif

			m_List.Insert(pVertexBuffer);
			pVertexBuffer->m_Used = true; // Created a new vertex buffer so make sure its flagged as used
		}
		else
		{
			LOGERROR(g_Log, "Failed to create vehicle vertex buffer of size %i\n", size);

			SAFE_DELETE_PTR(pVertexBuffer); // Creating the vertex buffer failed (probably out of memory). Does the game handle this?
		}		
	}
	return pVertexBuffer;
}

void C_VehicleVertexBufferManager::DestroyVertexBuffer(C_VehicleVertexBuffer* pVertexBuffer)
{
	BIZ_ASSERT(pVertexBuffer);
	BIZ_ASSERT(FindVertexBuffer(pVertexBuffer));

	#if !CFG_GOLD
	//LOGINFO(g_Log, "Freeing cached vehicle vertex buffer of size %i (bufferCount %i vertexSize %i)\n", pVertexBuffer->m_Data.GetSize(), m_List.Size(), m_Size);
	#endif

	pVertexBuffer->m_Used = false;
}

void C_VehicleVertexBufferManager::DestroyVertexBuffers()
{
	Btl::Set<C_VehicleVertexBuffer*>::Iterator it = m_List.Begin();
	while(it != m_List.End())
	{
		C_VehicleVertexBuffer* pVertexBuffer = *it;
		it++;
		if(!pVertexBuffer->m_Used)
		{
			#if !CFG_GOLD
			m_Size -= pVertexBuffer->m_Data.GetSize();
			BIZ_ASSERT(m_Size >= 0);
			#endif

			m_List.Erase(pVertexBuffer);
			Display2::Device()->Destroy(&pVertexBuffer->m_Data);
			SAFE_DELETE_PTR(pVertexBuffer);
		}
	}
}

/* 
===============================================================
					class C_VehicleModel

NOTES: 
===============================================================
*/

/*
===============
NAME: C_VehicleModel

NOTE: Constructor
===============
*/
C_VehicleModel::C_VehicleModel()
{
	m_pRenderLayer			= NULL;
	m_pGameLayer			= NULL;
	m_pDamageLayer			= NULL;

	m_pModelResource		= NULL;

	m_pLightSampler			= NULL;

	m_Initialised			= false;
	m_Hidden				= false;
	m_FocusVehicle			= false;
	m_pVisDetect			= 0;
	m_Alpha					= 1.0f;

	m_iBrakeLightsVisible = m_iHeadLightsVisible = m_iRearLightsVisible = m_iReverseLightsVisible = m_iFogLightsVisible = m_iRearAndBrakeLightsVisible = -1;
}

/*
===============
NAME: ~C_VehicleModel

NOTE: Destructor
===============
*/
C_VehicleModel::~C_VehicleModel()
{

}

/*
===============
NAME: Initialise

NOTE: Initialises model
===============
*/
void C_VehicleModel::Initialise(XtVehicle* p_pVehicle)
{
	RESOURCE_MEMORY_ID( "VehicleModel" );

	if (m_pDamageLayer)
	{		
		m_pDamageLayer->GetVehicleInfo()->m_Centre	= m_ModelInput.m_vCentre;
		m_pDamageLayer->GetVehicleInfo()->m_Extents	= m_ModelInput.m_vExtents;
		m_pDamageLayer->SetInitialDamageLevel(m_ModelSetup.m_InitialState.m_ScratchLevel, m_ModelSetup.m_InitialState.m_DamageLevel);
		m_pDamageLayer->Create(m_ModelSetup.m_Index);
		m_pDamageLayer->Setup(p_pVehicle);
		m_pDamageLayer->SetPartListPtr(&m_Parts);
		
		for (S32 iPart = 0; iPart < m_Parts.m_ActiveParts.Size(); iPart++)
		{
			if (m_Parts.m_ActiveParts[iPart].m_pNode)
			{
				C_Part* pNode = m_Parts.m_ActiveParts[iPart].m_pNode;
				C_ControlPointHandler::Part* pCP = m_Parts.m_ControlPointHandler.GetPart(pNode);
				if (pCP && m_pDamageLayer->GetNumLayers() >= 2)
				{
					S32 numWidth = m_pDamageLayer->GetNumWidth(0);
					S32 numLength = m_pDamageLayer->GetNumLength(0);
					
					if (pNode->m_PartCode == E_PartCode_Bonnet)
						pCP->m_pDamageNode = m_pDamageLayer->GetNode(1, numLength - 1, (S32)((float)numWidth / 2.0f));
					else if (pNode->m_PartCode == E_PartCode_Bumper && pNode->m_PartSubCode == E_PartSubCode_Front)
						pCP->m_pDamageNode = m_pDamageLayer->GetNode(0, numLength - 1, (S32)((float)numWidth / 2.0f));
					else if (pNode->m_PartCode == E_PartCode_Bumper && pNode->m_PartSubCode == E_PartSubCode_Back)
						pCP->m_pDamageNode = m_pDamageLayer->GetNode(0, 0, (S32)((float)numWidth / 2.0f));
					else if (pNode->m_PartCode == E_PartCode_Wing && (pNode->m_PartSubCode == E_PartSubCode_Left || pNode->m_PartSubCode == E_PartSubCode_FrontLeft))
						pCP->m_pDamageNode = m_pDamageLayer->GetNode(0, numLength - 1, 0);
					else if (pNode->m_PartCode == E_PartCode_Wing && (pNode->m_PartSubCode == E_PartSubCode_Right || pNode->m_PartSubCode == E_PartSubCode_FrontRight))
						pCP->m_pDamageNode = m_pDamageLayer->GetNode(0, numLength - 1, numWidth - 1);

					bool breakHere = true;
				}
			}
		}
	}
	
#if defined(PLATFORM_PS3)
	
	const U32 iNumberOfVertexBuffers = GetModelInstance(0)->GetRenderListNode()->GetNumVertexBuffers();
	m_damageOffsets.Resize(iNumberOfVertexBuffers);
	for(S32 i=0; i<m_damageOffsets.Size(); i++)
		m_damageOffsets[i] = NULL;

	Btl::OpenHashMap<void*, bool> vbState;
	
	for(U32 iCurrentPrimitive=0; iCurrentPrimitive<GetModelInstance(0)->GetRenderListNode()->GetNumPrimitives(); ++iCurrentPrimitive)
	{
		S16 iVertexBufferIndex	= GetModelInstance(0)->GetRenderListNode()->GetPrimitive(iCurrentPrimitive).primitiveStreams[0].vertexBuffer;	
		S32 iVertexBufferOffset	= GetModelInstance(0)->GetRenderListNode()->GetPrimitive(iCurrentPrimitive).primitiveStreams[0].vertexOffset;
		S16 iStreamIndex		= GetModelInstance(0)->GetRenderListNode()->GetPrimitive(iCurrentPrimitive).primitiveStreams[0].streamIndex;
		S32 iVertexStride		= GetModelInstance(0)->GetRenderListNode()->GetVertexBuffers(iVertexBufferIndex).vertexSize;
		
		S32 declaration			= GetModelInstance(0)->GetRenderListNode()->GetPrimitive(iCurrentPrimitive).vertexDeclaration;
		const Display2::C_VertexElement* decPtr = GetModelInstance(0)->GetRenderListNode()->GetVertexDeclaration(declaration)->GetElements();
		void* addr				= (void*)GetModelInstance(0)->GetRenderListNode()->GetVertexBuffers(iVertexBufferIndex).pBuffer;
		bool bFound = false;
		for(int i=0; i<GetModelInstance(0)->GetRenderListNode()->GetVertexDeclaration(declaration)->GetElementCount(); ++i)
		{
			if((decPtr[i].GetStream() == 3))
			{
				bFound = true;
				break;
			}
		}
		
		vbState[addr]		= bFound;
		
		BIZ_ASSERT(!iStreamIndex);
			
		RenderingData::Target::PrimitiveStream stream;
		stream.streamIndex = 3;
		stream.vertexBuffer = iVertexBufferIndex;
		stream.vertexOffset = (iVertexBufferOffset / iVertexStride) << 4;	// TODO: [Stove] Set up the stride of the damage offsets -- probably going to be 4 bytes.
		GetModelInstance(0)->AddDynamicPrimitiveStreams(stream);
	}
	
	// Create damage vertex buffers on PS3.
	for(U32 iCurrentVertexBuffer=0; iCurrentVertexBuffer<iNumberOfVertexBuffers; ++iCurrentVertexBuffer)
	{
		const RenderingData::Target::VertexBufferData& vbInfo = GetModelInstance(0)->GetRenderListNode()->GetVertexBuffers(iCurrentVertexBuffer);
		
// 			if(!vbState[(void*)vbInfo.pBuffer])
// 			{
// 				RenderingData::Target::VertexBufferData vbData;
// 				vbData.pBuffer = NULL;
// 				vbData.vertexSize = 0x0;
// 				GetModelInstance(0)->AddDynamicVertexBuffer(vbData);
// 				continue;	// skip stuff with no damage -- make dummy to match indices.
// 			}
				
		if(vbInfo.pBuffer)
		{
			U32 iSize				= (U32)vbInfo.pBuffer->GetSize();
			U32 iStride				= (U32)vbInfo.vertexSize;
			U32 iNumberOfVerts		= iSize / iStride;
			U32 iVertexBufferSize	= iNumberOfVerts * 16;	// TODO: [Stove] 11.11.10 normal format for damage offsets?
			RenderingData::Target::VertexBufferData vbData;
			m_damageOffsets[iCurrentVertexBuffer] = C_VehicleVertexBufferManager::GetInstance().CreateVertexBuffer(iVertexBufferSize);
			BIZ_ASSERT_MSG(m_damageOffsets[iCurrentVertexBuffer], "Failed to create vehicle vertex buffer");
			BIZ_ASSERT_MSG(m_damageOffsets[iCurrentVertexBuffer]->m_Data.IsCreated(), "Vertex buffer not created correctly.");
			BIZ_ASSERT_MSG(m_damageOffsets[iCurrentVertexBuffer]->m_Data.GetAddress(), "Vertex buffer address invalid.");
			vbData.pBuffer = &m_damageOffsets[iCurrentVertexBuffer]->m_Data;
			vbData.vertexSize = 0x10;						// TODO: [Stove] Set up the stride of the damage offsets -- probably going to be 4 bytes.
			
			volatile Display::Vertex_XYZ_C* pVertMemory = (volatile Display::Vertex_XYZ_C*)m_damageOffsets[iCurrentVertexBuffer]->m_Data.Lock();
			if(pVertMemory)
			{
				for(U32 iCurrentVert=0; iCurrentVert<iNumberOfVerts; ++pVertMemory, ++iCurrentVert)
				{
					pVertMemory->x = 0.0f;
					pVertMemory->y = 0.0f;
					pVertMemory->z = 0.0f;
					pVertMemory->c = 0x80000000;	// -1.0f
				}
			
				m_damageOffsets[iCurrentVertexBuffer]->m_Data.Unlock();
			}
			
			GetModelInstance(0)->AddDynamicVertexBuffer(vbData);
		}
		else
		{
			// Create a dummy to make indicies match.
			RenderingData::Target::VertexBufferData vbData;
			vbData.pBuffer = NULL;
			vbData.vertexSize = 0x0;
			GetModelInstance(0)->AddDynamicVertexBuffer(vbData);
		
			volatile int i = 0;
		}
	}
	
#endif // #if defined(PLATFORM_PS3) && defined(ENABLE_SPU_DAMAGE)

	WorldRender::C_ModelInstance* pModelInstance = NULL;
	for (S32 i = 0; i < m_pModelInstances.Size(); i++)
	{
		if (m_pModelInstances[i].m_eType == E_ModelInstanceType_Exterior)
			pModelInstance = m_pModelInstances[i].m_pModelInstance;
	}
	m_Parts.m_ControlPointHandler.Finalise(pModelInstance);

	m_ModelInput.m_bHeadLights.m_Initial		= m_ModelSetup.m_InitialState.m_NightLights;
	m_ModelInput.m_bHeadLightCones.m_Initial	= m_ModelSetup.m_InitialState.m_NightLights;
	m_ModelInput.m_bRearLights.m_Initial		= m_ModelSetup.m_InitialState.m_NightLights;
	m_ModelInput.m_bFogLights.m_Initial			= m_ModelSetup.m_InitialState.m_NightLights;
	m_ModelInput.m_bHeadLights.m_Current		= m_ModelSetup.m_InitialState.m_NightLights;
	m_ModelInput.m_bHeadLightCones.m_Current	= m_ModelSetup.m_InitialState.m_NightLights;
	m_ModelInput.m_bRearLights.m_Current		= m_ModelSetup.m_InitialState.m_NightLights;
	m_ModelInput.m_bFogLights.m_Current			= m_ModelSetup.m_InitialState.m_NightLights;

	//REMOVE_DIRT
	//m_DirtLayer.Initialise();
}

/*
===============
NAME: Create

NOTE: Creates model
===============
*/
bool C_VehicleModel::Create(const C_VehicleModelSetup* p_pModelSetup)
{
	//MEMORY_TRACKER_PUSH("vehicle.model");
	bool success = true;

	m_FocusVehicle = false;

	//Create render layer
	if (p_pModelSetup->m_bCreateRenderLayer)
	{
		if (!m_pRenderLayer && (m_pRenderLayer = new C_RenderLayer()))
		{
			m_pRenderLayer->Create();
			m_pRenderLayer->SetMessageHandler(m_pMessageHandler);
		}
		else
			success = false;
	}

	//Create game layer
	if (p_pModelSetup->m_bCreateGameLayer)
	{
		if (!m_pGameLayer && (m_pGameLayer = new C_GameLayer()))
			m_pGameLayer->SetMessageHandler(m_pMessageHandler);
		else
			success = false;
	}

	//Create damage layer
	if (p_pModelSetup->m_bCreateDamageLayer)
	{
		if (!m_pDamageLayer && (m_pDamageLayer = new C_DamageLayer(this)))
			m_pDamageLayer->SetMessageHandler(m_pMessageHandler);
		else
			success = false;
	}

	m_ModelSetup = *p_pModelSetup;

	m_Matrix = p_pModelSetup->m_Matrix;

	assert(!m_pLightSampler);

	m_pLightSampler = new WorldRender::C_LightSampler();
	WorldRender::C_LightSamplePointScene * const pLightSamplePointScene = p_pModelSetup->m_pLightSamplePointScene;
	m_pLightSampler->Initialise( pLightSamplePointScene );

	assert(m_pLightSampler);

	//MEMORY_TRACKER_POP("vehicle.model");
	return success;
}

/*
===============
NAME: Destroy

NOTE: Destroys model
===============
*/
bool C_VehicleModel::Destroy()
{
	bool success = true;

	//Destroy render layer
	if (m_pRenderLayer)
	{
		if (m_pRenderLayer->Destroy())
		{
			SAFE_DELETE_PTR(m_pRenderLayer);
		}
		else
			success = false;
	}

	//Destroy game layer
	if (m_pGameLayer)
	{
		if (m_pGameLayer->Destroy())
		{
			SAFE_DELETE_PTR(m_pGameLayer);
		}
		else
			success = false;
	}

	//Destroy damage layer
	if (m_pDamageLayer)
	{
		if (m_pDamageLayer->Destroy())
		{
			SAFE_DELETE_PTR(m_pDamageLayer);
		}
		else
			success = false;
	}
	
#ifdef PLATFORM_PS3

	// Damage jobs should have flushed by this point since the damage
	// layer is now destroyed, so kill off the vertex buffers.
	for(U32 iCurrentVertexBuffer=0; iCurrentVertexBuffer<m_damageOffsets.Size(); ++iCurrentVertexBuffer)
		if(m_damageOffsets[iCurrentVertexBuffer])
			C_VehicleVertexBufferManager::GetInstance().DestroyVertexBuffer(m_damageOffsets[iCurrentVertexBuffer]);
	
#endif // #ifdef PLATFORM_PS3	

	//SceneGraph::C_SceneGraph* pSceneGraph = C_VehicleModelManager::Instance().GetSceneGraphPtr();
	//		
	//for (U32 iModel = 0; iModel < m_pModelInstances.Size(); iModel++)
	//{
	//	if (m_pModelInstances[iModel])
	//	{
	//		pSceneGraph->Remove(m_pModelInstances[iModel]);
	//		delete m_pModelInstances[iModel];
	//	}
	//}

	m_pModelInstances.Clear();

	m_Parts.Shutdown();

	m_PaintJob.Destroy();

	m_Initialised = false;

	for (int iWheel = 0; iWheel < m_WheelBlur.Size(); iWheel++)
		m_WheelBlur[iWheel].Shutdown();
	m_WheelBlur.Clear();

	if (m_pLightSampler)
	{
		// WorldRender::C_RenderLevel::GetInstance()->GetLightSamplePoints().RemoveLightSampler(m_pLightSampler);
		SAFE_DELETE_PTR(m_pLightSampler);
	}
	
	
	return success;
}

/*
===============
NAME: GetComponent

NOTE: Gets component
===============
*/
C_Part* C_VehicleModel::GetComponent(E_PartCode p_PartCode, E_PartSubCode p_PartSubCode)
{
	return &m_Parts[p_PartCode].m_Nodes[p_PartSubCode];
}

/*
===============
NAME: SetComponent

NOTE: Sets component information 
===============
*/
C_Part* C_VehicleModel::SetComponent(	const GameResource::C_Model* p_pModel, 
												const GameResource::C_ModelElement* p_pModelElement,
												const GameResource::C_ModelResource* p_pModelResource,
												WorldRender::C_ModelInstance* p_pModelInstance,
												Gravity::C_Body* p_pBody,
												Gravity::C_Scene* p_pPhysicsScene,
												Maths::HMat43* p_pMatrix,
												E_PartCode p_PartCode,
												E_PartSubCode p_LocationCode)
{
	Util::C_String modelName = p_pModel->GetName();
	modelName.ConvertToLower();

	Util::C_String elementName = p_pModelElement->GetName();
	elementName.ConvertToLower();

	bool isModel = false;
	if (elementName.Length() > 2)
	{		
		if (elementName.Chars()[0] == 'm' && elementName.Chars()[1] == '_')
			isModel = true;
	}

	E_PartCode ePartCode		= E_PartCode_None;
	E_PartSubCode eLocCode		= E_PartSubCode_None;

	C_CodeProcessor::Instance().GetCodes(elementName, ePartCode, eLocCode);

	if (p_PartCode != E_PartCode_None)
		ePartCode = p_PartCode;
	if (p_LocationCode != E_PartSubCode_None)
		eLocCode = p_LocationCode;

	C_PartGroup* pComponent = &m_Parts[ePartCode];
	C_Part* pNode = &pComponent->m_Nodes[eLocCode];

	pNode->m_PartCode		= ePartCode;
	pNode->m_PartSubCode	= eLocCode;
	pNode->m_pPhysicsScene	= p_pPhysicsScene;
	pNode->m_pPartList	= &m_Parts;

	//pNode->SetVisible(pNode->GetVisible(true), true);
	//pNode->Hide(!pNode->GetVisible(true));
	if (ePartCode != E_PartCode_None)
	{
		pComponent->m_PartCode							= ePartCode;

		if (pNode->m_pModel || pNode->m_pModelInstance || pNode->m_pElement || pNode->m_pModelResource)
		{
		#ifndef VEHICLE_MASTER_BUILD
			LOGERROR(g_VehicleModelLog, "Trying to add '%s' when '%s' has already been added!", elementName.Chars(), pNode->debug_Name.Chars());
		#endif
			return NULL;
		}
	#ifndef VEHICLE_MASTER_BUILD
		pNode->debug_VehicleIndex = m_ModelSetup.m_Index;
	#endif
		pNode->m_pModel			= p_pModel;
		pNode->m_pModelInstance	= p_pModelInstance;

		// get the element matrix pointer out directly
		const WorldRender::MaterialParameter* pElementMatrixParam = p_pModelInstance->GetRenderModel()->GetWorldMatrixParameter(p_pModelElement->GetIndex());
		assert(pElementMatrixParam);

		pNode->m_pWorldMatrix = (MathsStorage::MatrixRow44*) p_pModelInstance->GetParameter(*pElementMatrixParam);
		pNode->m_pElement		= p_pModelElement;	
		pNode->m_pModelResource	= p_pModelResource;
		if (p_pBody)// && isModel)
			pNode->m_pPhysicsBody	= p_pBody;			
		
		if (p_pMatrix)
		{
			pNode->m_RestMatrix		= p_pModelElement->GetMatrix() * *p_pMatrix;		
			pNode->m_Matrix			= p_pModelElement->GetMatrix() * *p_pMatrix;
		}
		else
		{
			pNode->m_RestMatrix		= p_pModelElement->GetMatrix();		
			pNode->m_Matrix			= p_pModelElement->GetMatrix();
		}

	#ifndef VEHICLE_MASTER_BUILD
		pNode->debug_Name = elementName;
	#endif

		const GameResource::C_ModelElement* pParentElement = pNode->m_pElement->GetParent();
		if (pParentElement && ePartCode != E_PartCode_None)
		{
			Util::C_String parentName = pParentElement->GetName();

			ePartCode		= E_PartCode_None;
			eLocCode		= E_PartSubCode_None;
			C_CodeProcessor::Instance().GetCodes(parentName, ePartCode, eLocCode);
			C_Part* pParentPart = GetComponent(ePartCode, eLocCode);
			pNode->m_pParent = NULL;
			if (ePartCode != E_PartCode_None && ePartCode != E_PartCode_Wheel)
			{
				pNode->m_pParent = pParentPart;
				pNode->m_pChildren.PushBack(pNode);
				pNode->m_RestMatrix = pNode->m_RestMatrix * pParentPart->m_RestMatrix.InverseRotTrans();
			}
		}

		m_Parts.CreateSpawnMesh(pNode);

		return pNode;
	}

	return NULL;
}

/*
===============
NAME: CreateModels

NOTE: Create models and add them to scene graph and vehicle component list 
===============
*/
void C_VehicleModel::CreateModels()
{
	return;
}

/*
===============
NAME: Load

NOTE: Loads the model
===============
*/
bool C_VehicleModel::Load(const C_VehicleModelSetup* p_pModelSetup)
{
	return true;

	Util::C_String filePath;
	filePath = p_pModelSetup->m_FilePath + p_pModelSetup->m_FileName;
	
	m_ModelHandle.Load(filePath);

	m_pModelResource = m_ModelHandle;

	if (!m_pModelResource)
		return false;

	return true;
}

Debug::Bool g_debugMovePart("Vehicle.Debug.MovePart", false);

Debug::Enum<E_PartCode> g_debugPartCode("Vehicle.Debug.PartCode", E_PartCode_None);
Debug::Enum<E_PartSubCode> g_debugLocCode("Vehicle.Debug.LocCode", E_PartSubCode_None);
Debug::Bool g_debugHidePart("Vehicle.Debug.HidePart", false);
Debug::Bool g_debugUnHidePart("Vehicle.Debug.UnHidePart", false);

Debug::Float g_debugPartPosX("Vehicle.Debug.PartPosX", 0.0f, Debug::Range(-10.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugPartPosY("Vehicle.Debug.PartPosY", 0.0f, Debug::Range(-10.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugPartPosZ("Vehicle.Debug.PartPosZ", 0.0f, Debug::Range(-10.0f, 10.0f), Debug::Step(0.001f));

Debug::Float g_debugPartRotation("Vehicle.Debug.PartRotY", 0.0f, Debug::Range(-Maths::Pi2, Maths::Pi2), Debug::Step(0.001f));

Debug::Bool g_debugDrawDamageVolume("Vehicle.Damage.Debug.DrawVolume", false);

Debug::Bool g_debugActivatePhysicsObject("Vehicle.Debug.ActivatePhysicsObject", false);
Debug::Bool g_debugDeactivatePhysicsObject("Vehicle.Debug.DeactivatePhysicsObject", false);

Debug::Bool g_debugExplodeParts("Vehicle.Debug.ExplodeParts", true);
Debug::Float g_debugPartVelocityScale("Vehicle.Debug.PartVelocityScale", 1.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));

Debug::Int g_debugPartBehaviour("Vehicle.Damage.Parts.Constraints.DebugBehaviour", 0, Debug::Range(0, 1), Debug::Step(1));
Debug::Bool g_debugDrawParts("Vehicle.DebugRender.MasterDraw", false, Debug::Persist());

Debug::Bool g_debugRenderHeadlight("Vehicle.Debug.RenderHeadlightInfo", false, Debug::Persist());
Debug::Float g_debugHeadlightYOffset("Vehicle.Debug.HeadlightYOffset", 1.0f, Debug::Range(-4.0f, +4.0f));

Debug::Bool g_debugControlPointUpdate("Vehicle.Damage.Code.ControlPoints.UpdateControlPoint", false);

Debug::Enum<E_PartCode> g_debugControlPointPartCode("Vehicle.Damage.Code.ControlPoints.PartCode", E_PartCode_None);
Debug::Enum<E_PartSubCode> g_debugControlPointPartSubCode("Vehicle.Damage.Code.ControlPoints.PartSubCode", E_PartSubCode_None);

Debug::Float g_debugCP0OffsetX("Vehicle.Damage.Code.ControlPoints.ControlPoint0OffsetX", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugCP0OffsetY("Vehicle.Damage.Code.ControlPoints.ControlPoint0OffsetY", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugCP0OffsetZ("Vehicle.Damage.Code.ControlPoints.ControlPoint0OffsetZ", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugCP1OffsetX("Vehicle.Damage.Code.ControlPoints.ControlPoint1OffsetX", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugCP1OffsetY("Vehicle.Damage.Code.ControlPoints.ControlPoint1OffsetY", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugCP1OffsetZ("Vehicle.Damage.Code.ControlPoints.ControlPoint1OffsetZ", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugCP2OffsetX("Vehicle.Damage.Code.ControlPoints.ControlPoint2OffsetX", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugCP2OffsetY("Vehicle.Damage.Code.ControlPoints.ControlPoint2OffsetY", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugCP2OffsetZ("Vehicle.Damage.Code.ControlPoints.ControlPoint2OffsetZ", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugParticleExposure("Vehicle.ParticleExposure", 1.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));

Debug::Bool g_debugVehicleMasterDebug("Vehicle.Debug.MasterDebugSwitch_Model", false, Debug::Persist());

/*
===============
NAME: Update

NOTE: Updates model
===============
*/
void C_VehicleModel::Update(float p_FrameTime)
{
	PERF_EVENT("VehicleModel");

	if (m_ModelInput.m_bShouldRender)
	{
		for (int iWheel = 0; iWheel < m_WheelBlur.Size(); iWheel++)
			m_WheelBlur[iWheel].Update();
	}
}

void C_VehicleModel::UpdateHeadLightMatrix()
{
	if (!m_ModelInput.m_bShouldUpdate || (!m_ModelInput.m_bShouldRender && !m_Hidden))
		m_Headlight.m_Enabled = false;
	else
	{
		Maths::HMat43 mObjectMatrix = Maths::HMat43(Maths::HMat33(Maths::RotX(m_Headlight.m_Dip)), Maths::HVec3(0.0f, 0.0f, GetModelInput()->m_vExtents.Z() + m_Headlight.m_ZOffset));
		Maths::HMat43 mWorldMatrix = GetModelInput()->m_mMatrix;

		m_Headlight.m_Position	= GetModelInput()->m_mMatrix.AW() + Maths::HVec3(0.0f, g_debugHeadlightYOffset.Value(), GetModelInput()->m_vExtents.Z() + m_Headlight.m_ZOffset);
		m_Headlight.m_WorldMatrix = mObjectMatrix * mWorldMatrix;
	}

}

/*
===============
NAME: Update_Debug

NOTE: Updates debug stuff about the model
===============
*/
void C_VehicleModel::Update_Debug()
{
	if (g_debugDrawParts)
		m_Parts.DebugDraw(m_ModelInput.m_mMatrix);

	if (g_debugControlPointUpdate)
	{
		C_Part* pNode = &m_Parts[g_debugControlPointPartCode].m_Nodes[g_debugControlPointPartSubCode];
		C_ControlPointHandler::Part* pCP = m_Parts.m_ControlPointHandler.GetPart(pNode);
		if (pCP)
		{
			pCP->m_ControlPoints[0].m_Position = Maths::HVec3((float)g_debugCP0OffsetX, (float)g_debugCP0OffsetY, (float)g_debugCP0OffsetZ);
			pCP->m_ControlPoints[1].m_Position = Maths::HVec3((float)g_debugCP1OffsetX, (float)g_debugCP1OffsetY, (float)g_debugCP1OffsetZ);
			pCP->m_ControlPoints[2].m_Position = Maths::HVec3((float)g_debugCP2OffsetX, (float)g_debugCP2OffsetY, (float)g_debugCP2OffsetZ);
		}
	}

	if (g_debugHidePart || g_debugUnHidePart)
	{
		E_PartCode ePartCode	= g_debugPartCode;
		E_PartSubCode eLocCode = g_debugLocCode;

		if (g_debugHidePart || g_debugUnHidePart)
		{
			if (m_Parts[ePartCode].m_Nodes[eLocCode].m_pElement)
			{
				if (g_debugHidePart)
					m_Parts[ePartCode].m_Nodes[eLocCode].SetVisible(false);
				else if (g_debugUnHidePart)
					m_Parts[ePartCode].m_Nodes[eLocCode].SetVisible(true);
			}
			g_debugHidePart = false;
			g_debugUnHidePart = false;
		}
	}
	if (g_debugActivatePhysicsObject || g_debugDeactivatePhysicsObject)
	{
		E_PartCode ePartCode	= g_debugPartCode;
		E_PartSubCode eLocCode = g_debugLocCode;

		if (ePartCode == E_PartCode_None)
		{
			for (U32 iComponent = 0; iComponent < m_Parts.Size(); iComponent++)
			{
				for (int iNode = 0; iNode < m_Parts[iComponent].m_Nodes.Size(); iNode++)
				{
					if (m_Parts[iComponent].m_Nodes[iNode].m_pPhysicsBody)
					{
						Maths::HVec3 velocity = m_ModelInput.m_vVelocity;
						if (g_debugExplodeParts)
						{
							velocity = m_Parts[iComponent].m_Nodes[iNode].m_Matrix.AW();
							velocity -= m_ModelInput.m_mMatrix.AW();
							velocity.Normalise();							
							velocity *= (float)g_debugPartVelocityScale;
							velocity += m_ModelInput.m_vVelocity;

						}
						if (g_debugPartBehaviour == 0)
						{
							if (g_debugActivatePhysicsObject)
								m_Parts[iComponent].m_Nodes[iNode].ActivateConstraints();
							else if (g_debugDeactivatePhysicsObject)
								m_Parts[iComponent].m_Nodes[iNode].DeactivateConstraints();
						}
						else
						{
							if (g_debugActivatePhysicsObject)
								m_Parts[iComponent].m_Nodes[iNode].Detach(GetModelInput()->m_vVelocity);
							else if (g_debugDeactivatePhysicsObject)
								m_Parts[iComponent].m_Nodes[iNode].Reattach();
						}
					}
				}
			}
		}
		else
		{
			if (m_Parts[ePartCode].m_Nodes[eLocCode].m_pPhysicsBody)
			{
				Maths::HVec3 velocity(0.0f, 0.0f, 0.0f);
				if (g_debugPartBehaviour == 0)
				{
					if (g_debugActivatePhysicsObject)
						m_Parts[ePartCode].m_Nodes[eLocCode].ActivateConstraints();
					else if (g_debugDeactivatePhysicsObject)
						m_Parts[ePartCode].m_Nodes[eLocCode].DeactivateConstraints();
				}
				else
				{
					if (g_debugActivatePhysicsObject)
						m_Parts[ePartCode].m_Nodes[eLocCode].Detach(GetModelInput()->m_vVelocity);
					else if (g_debugDeactivatePhysicsObject)
						m_Parts[ePartCode].m_Nodes[eLocCode].Reattach();
				}
			}
		}

		g_debugActivatePhysicsObject = false;
		g_debugDeactivatePhysicsObject = false;
	}

	if (g_debugMovePart)
	{
		E_PartCode ePartCode	= g_debugPartCode;
		E_PartSubCode eLocCode = g_debugLocCode;

		Maths::HMat43 matrix = m_Parts[ePartCode].m_Nodes[eLocCode].m_RestMatrix;
		Maths::HVec3 translation(	matrix.AW().X() + (F32)g_debugPartPosX,
									matrix.AW().Y() + (F32)g_debugPartPosY,
									matrix.AW().Z() + (F32)g_debugPartPosZ);
		matrix.Translation(translation);

		Maths::RotY angle((float)g_debugPartRotation);
		Maths::HMat33 rotation(angle);
		matrix.Rotation(rotation);

		if (m_Parts[ePartCode].m_Nodes[eLocCode].m_pElement)
		{	
			S32 elementIndex = m_Parts[ePartCode].m_Nodes[eLocCode].m_pElement->GetIndex();
			m_Parts[ePartCode].m_Nodes[eLocCode].m_pModelInstance->SetElementMatrix(elementIndex, matrix);
		}
	}

	if (g_debugDrawDamageVolume && m_pDamageLayer)
		m_pDamageLayer->DebugDraw();

	if (g_debugRenderHeadlight)
	{
		DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
		pDebugRenderer->Draw3dMatrix(m_Headlight.m_WorldMatrix);
	}

}

/*
===============
NAME: UpdateLightSampler

NOTE: UpdateLightSampler
===============
*/
void C_VehicleModel::UpdateLightSampler()
{
	if (!m_pLightSampler)
		return;

	m_pLightSampler->SetPosition(m_ModelInput.m_mMatrix.Translation());
	m_pLightSampler->Update();

	m_Lighting.R(Maths::HVec4(0.0f, 1.0f, 0.0f, 1.0f) | Maths::HVec4(	m_pLightSampler->GetLightSampleData().m_SampleData[0],
																		m_pLightSampler->GetLightSampleData().m_SampleData[1],
																		m_pLightSampler->GetLightSampleData().m_SampleData[2],
																		m_pLightSampler->GetLightSampleData().m_SampleData[3]));
	m_Lighting.G(Maths::HVec4(0.0f, 1.0f, 0.0f, 1.0f) | Maths::HVec4(	m_pLightSampler->GetLightSampleData().m_SampleData[4],
																		m_pLightSampler->GetLightSampleData().m_SampleData[5],
																		m_pLightSampler->GetLightSampleData().m_SampleData[6],
																		m_pLightSampler->GetLightSampleData().m_SampleData[7]));
	m_Lighting.B(Maths::HVec4(0.0f, 1.0f, 0.0f, 1.0f) | Maths::HVec4(	m_pLightSampler->GetLightSampleData().m_SampleData[8],
																		m_pLightSampler->GetLightSampleData().m_SampleData[9],
																		m_pLightSampler->GetLightSampleData().m_SampleData[10],
																		m_pLightSampler->GetLightSampleData().m_SampleData[11]));
	m_Lighting.A(1.0f);

	Maths::HVec4 vLighting(m_Lighting.R() * (float)g_debugParticleExposure, m_Lighting.G() * (float)g_debugParticleExposure, m_Lighting.B() * (float)g_debugParticleExposure, 1.0f);

	vLighting = Maths::Sqrt(vLighting);
	m_Lighting.R(vLighting.X());
	m_Lighting.G(vLighting.Y());
	m_Lighting.B(vLighting.Z());
}

/*
===============
NAME: UpdateDeferred

NOTE: UpdateDeferred
===============
*/
void C_VehicleModel::UpdateDeferred()
{
	if (m_pDamageLayer)
		m_pDamageLayer->Update();

	m_Parts.Update(GetModelInput(), m_FrameTime);
	//Check for audio events
	if (m_FocusVehicle)
	{
		const Maths::HVec3 zero = Maths::Constants<Maths::HVec3>::Zero();
		for (S32 iPart = 0; iPart < m_Parts.m_ConstrainedParts.Size(); iPart++)
		{
			C_Part* pPart = m_Parts.m_ConstrainedParts[iPart];
			if (pPart->m_CurrentConstraintState == SharedPhysics::C_Constraint::E_ExceededLowLimit)
				m_pDamageLayer->AddDefferedDamageEffect(E_DefferedEvent_PartClosed, zero, zero, 1.0f, pPart->m_PartCode, pPart->m_PartSubCode);
			else if (pPart->m_CurrentConstraintState == SharedPhysics::C_Constraint::E_ExceededHighLimit)
				m_pDamageLayer->AddDefferedDamageEffect(E_DefferedEvent_PartOpened, zero, zero, 1.0f, pPart->m_PartCode, pPart->m_PartSubCode);
		}
	}

	if (m_pDamageLayer)
	{
		m_pDamageLayer->GetVehicleInfo()->m_Matrix		= m_ModelInput.m_mMatrix;
		m_pDamageLayer->GetVehicleInfo()->m_Velocity	= m_ModelInput.m_vVelocity;		
	}	

	UpdateLights();	

	m_PaintJob.Update();

	UpdateLightSampler();

	//REMOVE_DIRT
	//m_DirtLayer.Update(p_FrameTime, m_ModelInput.m_fSpeed);

	if (g_debugVehicleMasterDebug)
		Update_Debug();
}

Debug::Enum<E_PartCode> g_debugMatPartCode("Vehicle.Render.Material.PartCode", E_PartCode_None);
Debug::Enum<E_PartSubCode> g_debugMatSubCode("Vehicle.Render.Material.LocCode", E_PartSubCode_None);

/*
===============
NAME: UpdateDynamicMaterials

NOTE: Updates all dynamic materials for the parts
===============
*/
void C_VehicleModel::UpdateDynamicMaterials(XtVehicle* p_pVehicle, XtMaterialDefaults* p_pMaterialDefaults)
{
	if (g_debugMatPartCode == E_PartCode_None && g_debugMatSubCode == E_PartSubCode_None)
		m_Parts.UpdateDynamicMaterials(p_pVehicle, p_pMaterialDefaults);
	else
		m_Parts.UpdateDynamicMaterial(p_pVehicle, p_pMaterialDefaults, g_debugMatPartCode, g_debugMatSubCode);
}

Debug::Bool g_debugVehicleLightsUseNewMethod("Vehicle.Lights.UseNewMethod", true);

/*
===============
NAME: UpdateLights

NOTE: Updates a single light
===============
*/
void C_VehicleModel::UpdateLight(int iPartCode, int *piLightsVisibile, bool bVisible, bool bBaseVisible)
{
	if (g_debugVehicleLightsUseNewMethod)
	{
		int iLightsVisible = bVisible?1:0;
		if (iLightsVisible != *piLightsVisibile)
		{
			*piLightsVisibile = iLightsVisible;
			C_PartGroup* pComponent = &m_Parts[iPartCode];
			if (pComponent)
			{
				for (int iNode = 0; iNode < pComponent->m_Nodes.Size(); iNode++)
				{
					C_Part* pNode = &pComponent->m_Nodes[iNode];
					if (pNode && pNode->m_pModelInstance && pNode->m_pElement && pNode->m_pMaterialParameters && pNode->m_pMaterialParameters->m_Type == E_MaterialType_Light)
					{
						C_MaterialLight* pLight = (C_MaterialLight*)pNode->m_pMaterialParameters;
						pNode->m_MaterialParametersUpdate = true;
						if (pNode->m_PartCode == E_PartCode_RearAndBrakeLight)
						{
							pLight->m_Intensity = bBaseVisible ? 1.0f : 0.0f;
							pLight->m_Intensity += bVisible ? 1.0f : 0.0f;
						}
						else
							pLight->m_Intensity = bVisible ? 1.0f : 0.0f;
					}
				}
			}
		}
	}
	else
	{
		int iLightsVisible = bVisible?1:0;
		if (iLightsVisible != *piLightsVisibile)
		{
			*piLightsVisibile = iLightsVisible;
			C_PartGroup* pComponent = &m_Parts[iPartCode];
			if (pComponent)
			{
				for (int iNode = 0; iNode < pComponent->m_Nodes.Size(); iNode++)
				{
					C_Part* pNode = &pComponent->m_Nodes[iNode];
					if (pNode && pNode->m_pModelInstance && pNode->m_pElement)
						pNode->SetVisible(bVisible);
				}
			}
		}
	}
}

/*
===============
NAME: UpdateLights

NOTE: Updates lights
===============
*/
void C_VehicleModel::UpdateLights()
{
	PERF_EVENT("C_VehicleModel::UpdateLights");

	UpdateLight(E_PartCode_BrakeLight,	 &m_iBrakeLightsVisible,	m_ModelInput.m_fBrake > 0.01f);
	UpdateLight(E_PartCode_HeadLight,	 &m_iHeadLightsVisible,		m_ModelInput.m_bHeadLights.m_Current);
	UpdateLight(E_PartCode_RearLight,	 &m_iRearLightsVisible,		m_ModelInput.m_bRearLights.m_Current);
	UpdateLight(E_PartCode_ReverseLight, &m_iReverseLightsVisible,	m_ModelInput.m_iGear == -1);
	UpdateLight(E_PartCode_FogLight,	 &m_iFogLightsVisible,		m_ModelInput.m_bFogLights.m_Current);
	UpdateLight(E_PartCode_RearAndBrakeLight,	 &m_iRearAndBrakeLightsVisible,	m_ModelInput.m_fBrake > 0.01f, m_ModelInput.m_bRearLights.m_Current); //TODO: Needs work
}

/*
===============
NAME: Render

NOTE: Render placeholder
===============
*/
void C_VehicleModel::Render()
{
	if (!m_pRenderLayer)
		return;

	m_pRenderLayer->Render();
}

/*
===============
NAME: AddModelInstance

NOTE: Model instance add/get stuff
===============
*/
void C_VehicleModel::AddModelInstance(WorldRender::C_ModelInstance* p_pModelInstance, Util::C_String p_Name, S32 wheelIndex)
{
	m_pModelInstances.PushBack(Vehicle::C_VehicleModel::RenderModelInstance(p_pModelInstance, p_Name, p_pModelInstance->GetVisibilityFlags()));
	if (p_Name.ISearch("m_wheel") != -1 || p_Name.ISearch("m_alloy") != -1)
		m_pModelInstances.Back().m_eType = E_ModelInstanceType_Wheels;
	else if (p_Name.ISearch("m_interior") != -1)
		m_pModelInstances.Back().m_eType = E_ModelInstanceType_Interior;
	else if (p_Name.ISearch("m_shadowcaster") != -1)
		m_pModelInstances.Back().m_eType = E_ModelInstanceType_Shadow;
	else
		m_pModelInstances.Back().m_eType = E_ModelInstanceType_Exterior;

	m_pModelInstances.Back().m_WheelIndex = wheelIndex;

	CacheParameters(m_pModelInstances.Size(), p_pModelInstance);
}

/*
===============
NAME: SetupDynamicMaterials

NOTE: Sets up dynamic material stuff
===============
*/
void C_VehicleModel::SetupDynamicMaterials(XtVehicle* p_pVehicle, 
										   XtMaterialDefaults* p_pMaterialDefaults,
										   GameResource::C_ModelResource* p_pModelResource)
{
	if (!p_pModelResource)
		return;

	MEMORY_TRACKER_PUSH("graphics.vehicle.material");
	GameResource::C_ModelData *pModelDesc = p_pModelResource->GetModelData();
	WorldRender::C_RenderModelData *pGRModel = p_pModelResource->GetRenderModelData();

	// for each model inside the file
	for (int iModel = 0; iModel < pModelDesc->GetModelCount(); iModel++)
	{
		GameResource::C_Model *pModel = pModelDesc->GetModel(iModel);
		WorldRender::C_Model *pWRModel = &p_pModelResource->GetRenderModelData()->GetModel(iModel);

		// for each element inside this model
		for (int iElement=0; iElement<pModel->GetElementCount(); iElement++)
		{
			GameResource::C_ModelElement *pElement = pModel->GetElement(iElement);

			if (pElement->GetName().ISearch("_bone") != -1)
				continue;
			SetupDynamicMaterial(p_pVehicle, p_pMaterialDefaults, pWRModel, pElement);			
		}
	}
	MEMORY_TRACKER_POP("graphics.vehicle.material");
}

/*
===============
NAME: SetupDynamicMaterialsWheel

NOTE: Sets up dynamic material stuff for the wheels
===============
*/
void C_VehicleModel::SetupDynamicMaterialsWheel(XtVehicle* p_pVehicle, 
												XtMaterialDefaults* p_pMaterialDefaults)
{
	if (!m_pWheelModelResource)
		return;
//MEMORY_TRACKER_PUSH("graphics.vehicle.material.wheels");
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_WheelLod, E_PartSubCode_FrontLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_WheelLod, E_PartSubCode_FrontRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_WheelLod, E_PartSubCode_BackLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_WheelLod, E_PartSubCode_BackRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_Alloy, E_PartSubCode_FrontLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_Alloy, E_PartSubCode_FrontRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_Alloy, E_PartSubCode_BackLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_Alloy, E_PartSubCode_BackRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_AlloyRim, E_PartSubCode_FrontLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_AlloyRim, E_PartSubCode_FrontRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_AlloyRim, E_PartSubCode_BackLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_AlloyRim, E_PartSubCode_BackRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_AlloyBlur, E_PartSubCode_FrontLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_AlloyBlur, E_PartSubCode_FrontRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_AlloyBlur, E_PartSubCode_BackLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_AlloyBlur, E_PartSubCode_BackRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_Tyre, E_PartSubCode_FrontLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_Tyre, E_PartSubCode_FrontRight);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_Tyre, E_PartSubCode_BackLeft);
	SetMaterialExplicit(p_pVehicle, p_pMaterialDefaults, E_PartCode_Tyre, E_PartSubCode_BackRight);
//MEMORY_TRACKER_POP("graphics.vehicle.material.wheels");
}

/*
===============
NAME: SetupDynamicMaterial

NOTE: Sets up a dynamic material
===============
*/
void C_VehicleModel::SetupDynamicMaterial(	XtVehicle* p_pVehicle, 
											XtMaterialDefaults* p_pMaterialDefaults,
											WorldRender::C_Model* p_pWRModel, 
											GameResource::C_ModelElement* p_pElement)
{
//MEMORY_TRACKER_PUSH("graphics.vehicle.material.body");
	//Set generic material parameters
	SetMaterial(p_pVehicle, p_pMaterialDefaults, p_pElement->GetName().Lower(), p_pWRModel->GetParameter(p_pElement->GetIndex(), "g_vDamageSettings"), "g_vDamageSettings");
	SetMaterial(p_pVehicle, p_pMaterialDefaults, p_pElement->GetName().Lower(), p_pWRModel->GetParameter(p_pElement->GetIndex(), "g_vRenderSettings"), "g_vRenderSettings");
	SetMaterial(p_pVehicle, p_pMaterialDefaults, p_pElement->GetName().Lower(), p_pWRModel->GetParameter(p_pElement->GetIndex(), "g_vRenderColour"), "g_vRenderColour");
//MEMORY_TRACKER_POP("graphics.vehicle.material.body");
}

/*
===============
NAME: SetMaterial

NOTE: Sets mateiral and default values
===============
*/
void C_VehicleModel::SetMaterial(XtVehicle* p_pVehicle, 
								 XtMaterialDefaults* p_pMaterialDefaults,
								 Util::C_String p_ElementName, 
								 const WorldRender::MaterialParameter& p_Handle,
								 Util::C_String p_HandleName)
{
	E_PartCode ePartCode	= E_PartCode_None;
	E_PartSubCode eSubCode	= E_PartSubCode_None;
	
	C_CodeProcessor::Instance().GetCodes(p_ElementName, ePartCode, eSubCode);

	//Work around for material crash
	if (eSubCode == E_PartSubCode_None)
	{
		switch (ePartCode)
		{
		case E_PartCode_Wheel:
		case E_PartCode_Tyre:
		case E_PartCode_AlloyRim:
		case E_PartCode_Alloy:
		case E_PartCode_AlloyBlur:
		case E_PartCode_BrakeDisc:
		case E_PartCode_BrakeCaliper:
		case E_PartCode_Door:
		case E_PartCode_Wing:
		case E_PartCode_Bumper:
		case E_PartCode_Mirror:
		case E_PartCode_LicensePlate:
		case E_PartCode_Window:
		case E_PartCode_WindowInterior:
		case E_PartCode_Wiper:
		case E_PartCode_LightCluster:
		case E_PartCode_HeadLight:
		case E_PartCode_RearLight:
		case E_PartCode_FogLight:
		case E_PartCode_BrakeLight:
		case E_PartCode_ReverseLight:
		case E_PartCode_RunningLight:
		case E_PartCode_SignalLight:
			LOGINFO(g_VehicleModelLog, "PartCode(%d) has invalid PartSubCode(E_PartSubCode_None), elementName(%s)", ePartCode, p_ElementName.Chars());
			Automation::SendProblemFoundMsg("Vehicle", m_ModelSetup.m_Index, ("InvPartSC{?}" + p_ElementName).Chars());
			return;
		case E_PartCode_Bonnet:
		case E_PartCode_Boot:
		case E_PartCode_SteeringWheel:
		case E_PartCode_HandBrake:
		case E_PartCode_GearStick:
		case E_PartCode_PaddleShift:
		case E_PartCode_SignalStalk:
		default:
			break;
		}
	}

	//Will this MASSIVE temporary hack fix a crash?
	if (ePartCode == E_PartCode_Wheel)
		return;
		//bool breakHere= true;

	C_Part* pNode = &m_Parts[ePartCode].m_Nodes[eSubCode];
	C_MaterialBase* pMaterial = pNode->m_pMaterialParameters;

	if (!pMaterial)
	{
		pMaterial = C_MaterialManager::Instance().CreateMaterial(	p_pVehicle, 
																	p_pMaterialDefaults,
																	pNode->m_pModelInstance,
																	ePartCode, eSubCode);

		pNode->m_pMaterialParameters = pMaterial;
		if (pMaterial)
			pNode->m_MaterialParametersType = pNode->m_pMaterialParameters->m_Type;
	}

	if (!pNode->m_pMaterialParameters)
		return;

	pNode->m_pMaterialParameters->SetParameter(p_Handle, p_HandleName);
}

/*
===============
NAME: SetMaterialExplicit

NOTE: Sets mateiral handle and default values
===============
*/
void C_VehicleModel::SetMaterialExplicit(	XtVehicle* p_pVehicle, 
											XtMaterialDefaults* p_pMaterialDefaults,
										    E_PartCode p_PartCode,
											E_PartSubCode p_SubCode)
{
	C_Part* pNode = &m_Parts[p_PartCode].m_Nodes[p_SubCode];
	C_MaterialBase* pMaterial = pNode->m_pMaterialParameters;

	if (!pNode->m_pModelResource ||!pNode->m_pModel ||!pNode->m_pElement)
		return;

	if (!pMaterial)
	{
		pMaterial = C_MaterialManager::Instance().CreateMaterial(	p_pVehicle, 
																	p_pMaterialDefaults,
																	pNode->m_pModelInstance,
																	p_PartCode, p_SubCode);

		pNode->m_pMaterialParameters = pMaterial;
	}

	if (!pNode->m_pMaterialParameters)
		return;


	const GameResource::C_ModelResource* pModelResource = pNode->m_pModelResource;
	const GameResource::C_ModelData *pModelDesc = pModelResource->GetModelData();
	const WorldRender::C_RenderModelData *pGRModel = pModelResource->GetRenderModelData();
	
	const WorldRender::C_Model *pWRModel = &pModelResource->GetRenderModelData()->GetModel(pNode->m_pModel->GetIndex());
	pNode->m_pMaterialParameters->SetParameter(pWRModel->GetParameter(pNode->m_pElement->GetIndex(), "g_vDamageSettings"), "g_vDamageSettings");
	pNode->m_pMaterialParameters->SetParameter(pWRModel->GetParameter(pNode->m_pElement->GetIndex(), "g_vRenderSettings"), "g_vRenderSettings");
	pNode->m_pMaterialParameters->SetParameter(pWRModel->GetParameter(pNode->m_pElement->GetIndex(), "g_vRenderColour"), "g_vRenderColour");
}

Debug::Float g_debugLiveryBlendScale("Vehicle.Livery.BlendScale", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: SetStaticShaderParameters

NOTE: Set all shader parameters that can don't change per-frame
===============
*/
void C_VehicleModel::SetStaticShaderParameters()
{
//MEMORY_TRACKER_PUSH("graphics.vehicle.material.params");

	for (int iInstance = 0; iInstance < m_pModelInstances.Size(); iInstance++)
	{
		WorldRender::C_ModelInstance* pInstance = m_pModelInstances[iInstance].m_pModelInstance;
		if (GetDamageLayer())
		{
			WorldRender::MaterialParameter handle = GetParameter(iInstance, E_CachedParamName_VehicleVtxParams);
			if (handle.index != -1)
			{
				VehicleVtxParams *p = (VehicleVtxParams*)pInstance->GetParameter(handle);
				p->VehicleBoundsInfo.x = GetDamageLayer()->GetVehicleInfo()->m_Extents.X() * 2.1f;
				p->VehicleBoundsInfo.y = GetDamageLayer()->GetVehicleInfo()->m_Extents.Y() * 2.1f;
				p->VehicleBoundsInfo.z = GetDamageLayer()->GetVehicleInfo()->m_Extents.Z() * 2.1f;
				p->VehicleBoundsInfo.w = GetDamageLayer()->GetVehicleInfo()->m_Centre.Y();
			}
		}
		WorldRender::MaterialParameter handle = GetParameter(iInstance, E_CachedParamName_LiverySettings);
		if (handle.index != -1 && handle.offset != -1)
		{
			float fSettings[4] = {0.0f, 0.0f, 0.0f, (float)g_debugLiveryBlendScale};
			pInstance->SetParameter(handle, (void*)fSettings);
		}			
		if (pInstance && pInstance->GetRenderModel())
		{			
			WorldRender::MaterialParameter handle = pInstance->GetRenderModel()->GetParameter("CodeDamageScaleParam");
			if (handle.index != -1 && handle.offset != -1)
			{
				float fDamageSettings = 1.0f;
				pInstance->SetParameter(handle, (void*)&fDamageSettings);
			}
		}
	}
//MEMORY_TRACKER_POP("graphics.vehicle.material.params");
}

/*
===============
NAME: SetDynamicShaderParameters

NOTE: Set all shader parameters that can change per-frame
===============
*/
void C_VehicleModel::SetDynamicShaderParameters(bool force)
{
	if(m_pVisDetect && !m_pVisDetect->IsVisible() && !force)
		return;

	PERF_EVENT("C_VehicleModel::SetDynamicShaderParameters");

//MEMORY_TRACKER_PUSH("graphics.vehicle.material.params");
	for (int iInstance = 0; iInstance < m_pModelInstances.Size(); iInstance++)
	{
		WorldRender::MaterialParameter handle;
		handle.index = -1;
		handle.offset = -1;
		
		WorldRender::C_ModelInstance* pInstance = m_pModelInstances[iInstance].m_pModelInstance;
		
		handle = GetParameter(iInstance, E_CachedParamName_UndercoatColour);
		if (handle.index != -1 && handle.offset != -1)
		{
			// [Blur2PC] The retail PC vehicle shaders read .w of these two registers as damage amounts:
			// undercoat.w scales the dent offsets/normals, scuff.w is the scratch/undercoat level. The
			// paint job builds them as (r, g, b, 1), so every car rendered fully dented and scratched a
			// moment after load. Upload the colour with w = damage amount (0 = pristine; per-part race
			// damage is not wired yet, see docs/analysis.md).
			MathsStorage::Float4 colour = GetPaintJob()->GetUndercoatColour();
			colour.w = 0.0f;
			pInstance->SetParameter(handle, &colour);
		}			

		handle = GetParameter(iInstance, E_CachedParamName_ScuffColour);
		if (handle.index != -1 && handle.offset != -1)
		{
			MathsStorage::Float4 colour = GetPaintJob()->GetScuffColour();	// [Blur2PC] w = damage amount, see above
			colour.w = 0.0f;
			pInstance->SetParameter(handle, &colour);
		}			

#ifndef CFG_GOLD
		handle = GetParameter(iInstance, E_CachedParamName_LiverySettings);
		if (handle.index != -1 && handle.offset != -1)
		{
			float fSettings[4] = {0.0f, 0.0f, 0.0f, (float)g_debugLiveryBlendScale};
			pInstance->SetParameter(handle, (void*)fSettings);
		}
#endif

#ifndef PLATFORM_PS3
		if (GetDamageLayer())
		{
			handle = GetParameter(iInstance, E_CachedParamName_VehicleDamageVolumeTexture);
			if (handle.index != -1 && handle.offset != -1)
			{
				const void* pData = GetDamageLayer()->GetDamageVolumeTexture()->GetGPUTexture();
				pInstance->SetParameter(handle, &pData);
			}
			else
			{
				// [Blur2PC] Only the first instance of each car gets a per-instance handle; the others read the
				// shared parameter, which kept the FX system default (a 1x1 opaque white texel). The retail
				// vertex shader adds the sampled value to the position and uses 2*alpha as the scratch level,
				// so those parts rendered fully dented and scratched a few frames after load. Feed the shared
				// parameter the real volume. It is global, so cars that share it see the last car's damage
				// until the per-instance lookup is fixed (see docs/analysis.md).
				const void* pData = GetDamageLayer()->GetDamageVolumeTexture()->GetGPUTexture();
				FX::SetSharedParameterByName("g_sVehicleDamageVolumeTexture", (void*)&pData);
			}

			handle = GetParameter(iInstance, E_CachedParamName_VehicleDamageInfoTexture);
			if (handle.index != -1 && handle.offset != -1)
			{
				const void* pData = GetDamageLayer()->GetDamageInfoTexture()->GetGPUTexture();
				pInstance->SetParameter(handle, &pData);
			}
			else
			{
				const void* pData = GetDamageLayer()->GetDamageInfoTexture()->GetGPUTexture();	// [Blur2PC] as above
				FX::SetSharedParameterByName("g_sVehicleDamageInfoTexture", (void*)&pData);
			}
		}
#endif
		handle = GetParameter(iInstance, E_CachedParamName_PaintMaterialTexture);
		if (handle.index != -1 && handle.offset != -1)
		{
			const void* pData = m_PaintJob.GetMaterialTexture()->GetGPUTexture();
			pInstance->SetParameter(handle, &pData);
		}

		//REMOVE_DIRT
		//handle = GetParameter(iInstance, E_CachedParamName_VehicleDirtLayerTexture);
		//if (handle.index != -1 && handle.offset != -1)
		//{
		//	const void* pData = C_VehicleModelManager::Instance().m_DirtLayerTexture.GetGPUTexture();
		//	pInstance->SetParameter(handle, &pData);
		//}

		handle = GetParameter(iInstance, E_CachedParamName_VehiclePixParams);
		if (handle.index != -1 && handle.offset != -1)
		{
			VehiclePixParams *p = (VehiclePixParams*)pInstance->GetParameter(handle);
			//REMOVE_DIRT
			//p->vDirtBaseLayerSettings = m_DirtLayer.m_vBaseLayerColour;
			//p->vDirtDetailLayerSettings = m_DirtLayer.m_vDetailLayerColour;
			p->vAlphaScale.x = m_Alpha;
		}
		
		handle = GetParameter(iInstance, E_CachedParamName_VehicleVtxParams);
		if (handle.index != -1)
		{
			VehicleVtxParams *p = (VehicleVtxParams*)pInstance->GetParameter(handle);
			Maths::Store(GetModelInput()->m_mMatrix, &p->mVehicleMatrix);
			Maths::HMat43 mtx = GetModelInput()->m_mMatrix.InverseRotTrans();
			Maths::Store(mtx, &p->mVehicleInverseMatrix);
		}
	}
//MEMORY_TRACKER_POP("graphics.vehicle.material.params");
}

inline Maths::HVec4 RotateShVector(const Maths::HVec4& shVector, const Maths::HMat33& rotationMatrix)
{

	// Swizzle:                     (-w, -y, z, x)
	// Unswizzle:      (w, -y, z, -x)

	//Maths::HVec4 shVectorSwizzled = Maths::HVec4(shVector.W(), -shVector.Y(), shVector.Z(), shVector.X());
	// Rotate
	Maths::HVec4 shVectorRotatedAndSwizzled = (rotationMatrix * Maths::HVec3(shVector)) || Maths::HVecW(shVector.W());
	// Unswizzle and return.

	return shVectorRotatedAndSwizzled;//Maths::HVec4(shVectorRotatedAndSwizzled.W(), -shVectorRotatedAndSwizzled.Y(), shVectorRotatedAndSwizzled.Z(), shVectorRotatedAndSwizzled.X());
}

void RotateSH(MathsStorage::Float44& shData, const Maths::HMat33& rotateMatrix, F32 scale)
{
	Maths::HVec4 shRedRotated = RotateShVector(Maths::HVec4(shData.v[0], shData.v[1], shData.v[2], shData.v[3]), rotateMatrix);
	Maths::HVec4 shGreenRotated = RotateShVector(Maths::HVec4(shData.v[4], shData.v[5], shData.v[6], shData.v[7]), rotateMatrix);
	Maths::HVec4 shBlueRotated = RotateShVector(Maths::HVec4(shData.v[8], shData.v[9], shData.v[10], shData.v[11]), rotateMatrix);

	const float fC0 = 0.282094777; //1.0f / (2.0f * sqrtf(PI));
	const float fC1 = 0.325734973; //sqrt(3.0f) / (3.0f * sqrtf(PI));

	float fScale = scale;

	shData.v[0] = fScale *shRedRotated.W()/fC0;
	shData.v[1] = -fScale *shRedRotated.Y()/fC1;
	shData.v[2] = fScale *shRedRotated.Z()/fC1;
	shData.v[3] = -fScale *shRedRotated.X()/fC1;

	shData.v[4] = fScale *shGreenRotated.W()/fC0;
	shData.v[5] = -fScale *shGreenRotated.Y()/fC1;
	shData.v[6] = fScale *shGreenRotated.Z()/fC1;
	shData.v[7] = -fScale *shGreenRotated.X()/fC1;

	shData.v[8] = fScale *shBlueRotated.W()/fC0;
	shData.v[9] = -fScale *shBlueRotated.Y()/fC1;
	shData.v[10] = fScale *shBlueRotated.Z()/fC1;
	shData.v[11] = -fScale *shBlueRotated.X()/fC1;
}

void C_VehicleModel::SetDynamicSHLightShaderParameters( const F32 additiveSHLightData[16], const Maths::HMat43& matrix, const MathsStorage::Float4& rearLightColour )
{
	if(m_pVisDetect && !m_pVisDetect->IsVisible())
		return;

	PERF_EVENT("C_VehicleModel::SetDynamicSHLightShaderParameters");

	Maths::HMat44 vSH = Maths::Load<Maths::HMat44>((MathsStorage::Float44*)m_pLightSampler->GetLightSampleData().m_SampleData);
	Maths::HMat44 vAddSH = Maths::Load<Maths::HMat44>((MathsStorage::Float44*)additiveSHLightData);
	vSH = vAddSH + vSH;

	//Rotate sh to car local space
	Maths::HMat33 worldToVehicleRotation(matrix.Rotation());	

	MathsStorage::Float44 shVehicle;
	Maths::Store(vSH, &shVehicle);
	RotateSH(shVehicle, worldToVehicleRotation, g_fVehicleSHScale);
	
	//Rotate wheels to local space
	MathsStorage::Float44 shBackLeftWheel;
	MathsStorage::Float44 shFrontLeftWheel;
	MathsStorage::Float44 shBackRightWheel;
	MathsStorage::Float44 shFrontRightWheel;
	Maths::Store(vSH, &shBackLeftWheel);
	Maths::Store(vSH, &shFrontLeftWheel);
	Maths::Store(vSH, &shBackRightWheel);
	Maths::Store(vSH, &shFrontRightWheel);


	RotateSH(shBackLeftWheel, Maths::HMat33(m_ModelInput.m_Wheels[2].m_mMatrix) , g_fVehicleSHScale);
	RotateSH(shFrontLeftWheel, Maths::HMat33(m_ModelInput.m_Wheels[0].m_mMatrix) , g_fVehicleSHScale);
	RotateSH(shBackRightWheel, Maths::HMat33(m_ModelInput.m_Wheels[3].m_mMatrix) , g_fVehicleSHScale);
	RotateSH(shFrontRightWheel, Maths::HMat33(m_ModelInput.m_Wheels[1].m_mMatrix) , g_fVehicleSHScale);

	//MEMORY_TRACKER_PUSH("graphics.vehicle.material.params");
	for (int iInstance = 0; iInstance < m_pModelInstances.Size(); iInstance++)
	{
		WorldRender::MaterialParameter handle;
		handle.index = -1;
		handle.offset = -1;

		WorldRender::MaterialParameter vehicleSHhandle;
		vehicleSHhandle.index = -1;
		vehicleSHhandle.offset = -1;

		WorldRender::MaterialParameter backLeftWheelSHhandle;
		backLeftWheelSHhandle.index = -1;
		backLeftWheelSHhandle.offset = -1;

		WorldRender::MaterialParameter frontLeftWheelSHhandle;
		frontLeftWheelSHhandle.index = -1;
		frontLeftWheelSHhandle.offset = -1;

		WorldRender::MaterialParameter backRightWheelSHhandle;
		backRightWheelSHhandle.index = -1;
		backRightWheelSHhandle.offset = -1;

		WorldRender::MaterialParameter frontRightWheelSHhandle;
		frontRightWheelSHhandle.index = -1;
		frontRightWheelSHhandle.offset = -1;


		WorldRender::C_ModelInstance* pInstance = m_pModelInstances[iInstance].m_pModelInstance;

		if (m_pLightSampler)
		{
			handle = GetParameter(iInstance, E_CachedParamName_VehicleVtxParams);
			if (handle.index != -1)
			{
				VehicleVtxParams *p = (VehicleVtxParams*)pInstance->GetParameter(handle);
				
				//Set rear light colour
				Maths::HVec4 v = Maths::Load<Maths::HVec4>(&rearLightColour);
#if PLATFORM_PS3
				v *= Maths::HF32(1.f/255.f);
#endif
				Maths::Store(v, &p->RearLightColour);
			}
			
			if (m_pModelInstances[iInstance].m_eType != E_ModelInstanceType_Wheels)
			{
				vehicleSHhandle = GetParameter(iInstance, E_CachedParamName_VehicleSHNode);
				if (vehicleSHhandle.index != -1)
				{
					memcpy(pInstance->GetParameter(vehicleSHhandle), &shVehicle.v[0], sizeof(shVehicle));
				}
				backLeftWheelSHhandle = GetParameter(iInstance, E_CachedParamName_BackLeftSHNode);
				if (backLeftWheelSHhandle.index != -1)
				{
					memcpy(pInstance->GetParameter(backLeftWheelSHhandle), &shBackLeftWheel.v[0], sizeof(shBackLeftWheel));
				}
				frontLeftWheelSHhandle = GetParameter(iInstance, E_CachedParamName_FrontLeftSHNode);
				if (frontLeftWheelSHhandle.index != -1)
				{
					memcpy(pInstance->GetParameter(frontLeftWheelSHhandle), &shFrontLeftWheel.v[0], sizeof(shFrontLeftWheel));
				}
				backRightWheelSHhandle = GetParameter(iInstance, E_CachedParamName_BackRightSHNode);
				if (backRightWheelSHhandle.index != -1)
				{
					memcpy(pInstance->GetParameter(backRightWheelSHhandle), &shBackRightWheel.v[0], sizeof(shBackRightWheel));
				}
				frontRightWheelSHhandle = GetParameter(iInstance, E_CachedParamName_FrontRightSHNode);
				if (frontRightWheelSHhandle.index != -1)
				{
					memcpy(pInstance->GetParameter(frontRightWheelSHhandle), &shFrontRightWheel.v[0], sizeof(shFrontRightWheel));
				}
			}
			else
			{
				//Blur Wheels:  Using vehicleSH
				vehicleSHhandle = GetParameter(iInstance, E_CachedParamName_VehicleSHNode);
				if (vehicleSHhandle.index != -1)
				{
					switch (m_pModelInstances[iInstance].m_WheelIndex)
					{
					case 0:
						memcpy(pInstance->GetParameter(vehicleSHhandle), &shFrontLeftWheel.v[0], sizeof(shFrontLeftWheel));
						break;
					case 1:
						memcpy(pInstance->GetParameter(vehicleSHhandle), &shFrontRightWheel.v[0], sizeof(shFrontRightWheel));
						break;
					case 2:
						memcpy(pInstance->GetParameter(vehicleSHhandle), &shBackLeftWheel.v[0], sizeof(shBackLeftWheel));
						break;
					case 3:
						memcpy(pInstance->GetParameter(vehicleSHhandle), &shBackRightWheel.v[0], sizeof(shBackRightWheel));
						break;
					default:
						LOGERROR(g_Log, "VehicleModel: Invalid wheel index");
						break;
					}
				}
			}
		}
	}
	//MEMORY_TRACKER_POP("graphics.vehicle.material.params");
}

/*
===============
NAME: AddWheelBlur

NOTE: Adds wheel blur
===============
*/
void C_VehicleModel::AddWheelBlur(C_Part* p_pUnblurredSpokes, C_Part* p_pBlurredSpokes)
{
	if (!p_pUnblurredSpokes || !p_pBlurredSpokes)
		return;

	C_WheelInput* pInput = NULL;
	
	switch (p_pUnblurredSpokes->m_PartSubCode)
	{
		case E_PartSubCode_FrontLeft:
			pInput = m_ModelInput.m_Wheels.Size() == 0 ? NULL : &m_ModelInput.m_Wheels[0];
			break;
		case E_PartSubCode_FrontRight:
			pInput = m_ModelInput.m_Wheels.Size() < 2 ? NULL : &m_ModelInput.m_Wheels[1];
			break;
		case E_PartSubCode_BackLeft:
			pInput = m_ModelInput.m_Wheels.Size() < 3 ? NULL : &m_ModelInput.m_Wheels[2];
			break;
		case E_PartSubCode_BackRight:
			pInput = m_ModelInput.m_Wheels.Size() < 4 ? NULL : &m_ModelInput.m_Wheels[3];
			break;
	}

	if (!pInput)
		return;

	m_WheelBlur.PushBack();
	m_WheelBlur.Back().Initialise(p_pUnblurredSpokes, p_pBlurredSpokes, pInput);
}

/*
===============
NAME: SetPartVisibility

NOTE: 
===============
*/
void C_VehicleModel::SetPartVisibility(E_PartCode p_PartCode, bool p_Visible)
{
	for (S32 iNode = 0; iNode < m_Parts[p_PartCode].m_Nodes.Size(); iNode++)
	{
		m_Parts[p_PartCode].m_Nodes[iNode].SetVisible(p_Visible);
	}
}

static const char *g_pszCachedShaderParamNames[] = 
{
	//REMOVE_DIRT
	//"g_vDirtBaseLayerSettings",
	//"g_vDirtDetailLayerSettings",
	"g_sVehicleDamageVolumeTexture",
	"g_sVehicleDamageInfoTexture",
	"g_sPaintMaterialTexture",
	//REMOVE_DIRT
	//"g_sVehicleDirtLayerTexture",
	"g_VehicleVtxParams",
	"g_VehiclePixParams",
	"g_vDebugDamageSettings",
	"g_vDamageColour",
	"g_vUndercoatColour",
	"g_vScuffColour",
	"g_VehicleSHNode",
	"g_sLightEnvironmentTexture",
	"g_sLiveryTexture",
	"g_vAlphaScale",
	"g_vLiverySettings",
	"VehicleSHNode",
	"BackLeftSHNode",
	"FrontLeftSHNode",
	"BackRightSHNode",
	"FrontRightSHNode"
};

/*
===============
NAME: CacheParameters

NOTE: Cache shader parameter handles for this model
===============
*/
void C_VehicleModel::CacheParameters(int i, WorldRender::C_ModelInstance* pInstance)
{
	m_CachedParameters.PushBack();
	assert(m_CachedParameters.Size() == i);
	CachedModelParameters &parameters = m_CachedParameters.Back();
	for (int i=0; i<parameters.Size(); i++)
	{
		parameters[i] = pInstance->GetRenderModel()->GetParameter(g_pszCachedShaderParamNames[i]);
	}
}

/*
===============
NAME: SetLightEnvironmentTexture

NOTE: SetLightEnvironmentTexture
===============
*/
void C_VehicleModel::SetLightEnvironmentTexture( Display::C_TextureCube* pLightEnvironmentTexture )
{
	for (int iInstance = 0; iInstance < m_pModelInstances.Size(); iInstance++)
	{
		WorldRender::C_ModelInstance* pInstance = m_pModelInstances[iInstance].m_pModelInstance;
		WorldRender::MaterialParameter handle = GetParameter(iInstance, E_CachedParamName_LightEnvironmentTexture);
		if (handle.index != -1)
		{
			const void* pData = pLightEnvironmentTexture->GetGPUTexture();
			pInstance->SetParameter(handle, &pData);
		}
	}
}

/*
===============
NAME: SetLiveryTexture

NOTE: SetLiveryTexture
===============
*/
void C_VehicleModel::SetLiveryTexture( Display::C_Texture2D* pLiveryTexture )
{
	for (int iInstance = 0; iInstance < m_pModelInstances.Size(); iInstance++)
	{
		WorldRender::C_ModelInstance* pInstance = m_pModelInstances[iInstance].m_pModelInstance;
		WorldRender::MaterialParameter handle = GetParameter(iInstance, E_CachedParamName_LiveryTexture);
		if (handle.index != -1)
		{
			const void* pData = pLiveryTexture->GetGPUTexture();
			pInstance->SetParameter(handle, &pData);
		}
	}
}

/*
===============
NAME: Freeze

NOTE: Freeze
===============
*/
void C_VehicleModel::Freeze(bool p_bFreeze)
{
	GetComponentList().FreezeParts(p_bFreeze, m_ModelInput.m_mMatrix);
}

/*
===============
NAME: Reset

NOTE: Reset
===============
*/
void C_VehicleModel::Reset()
{
	if (m_Hidden)
		return;

	for (S32 iWheel = 0; iWheel < m_WheelBlur.Size(); iWheel++)
	{
		m_WheelBlur[iWheel].Reset();
	}
}

void C_VehicleModel::EnableVisibilityFlags( U32 visibilityFlags )
{
	for (int iInstance = 0; iInstance < m_pModelInstances.Size(); iInstance++)
	{
		RenderModelInstance& renderModelInstance = m_pModelInstances[iInstance];
		WorldRender::C_ModelInstance* pInstance = renderModelInstance.m_pModelInstance;
		pInstance->EnableVisibilityFlags(visibilityFlags & renderModelInstance.m_VisibilityFlagMask);
	}
}

void C_VehicleModel::DisableVisibilityFlags( U32 visibilityFlags )
{
	for (int iInstance = 0; iInstance < m_pModelInstances.Size(); iInstance++)
	{
		RenderModelInstance& renderModelInstance = m_pModelInstances[iInstance];
		WorldRender::C_ModelInstance* pInstance = renderModelInstance.m_pModelInstance;
		pInstance->DisableVisibilityFlags(visibilityFlags & renderModelInstance.m_VisibilityFlagMask);
	}
}

/*
===============
NAME: Reset

NOTE: Reset
===============
*/
void C_VehicleModel::ResetInitialStates()
{
	GetModelInput()->m_bHeadLights.m_Current		= GetModelInput()->m_bHeadLights.m_Initial;
	GetModelInput()->m_bHeadLightCones.m_Current	= GetModelInput()->m_bHeadLightCones.m_Initial;
	GetModelInput()->m_bFogLights.m_Current			= GetModelInput()->m_bFogLights.m_Initial;
	GetModelInput()->m_bRearLights.m_Current		= GetModelInput()->m_bRearLights.m_Initial;
}	

} //namespace Vehicle

