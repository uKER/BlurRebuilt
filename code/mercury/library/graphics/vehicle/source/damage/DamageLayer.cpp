/////////////////////////////////////////////////////////////
// DamageLayer
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Damage/DamageLayer.h"
#include "Vehicle/Component.h"
#include "Vehicle/Db/VehicleXt.h"
#include "Vehicle/Damage/DamageBehaviours.h"

#include <RacingPhysics/RacingPhysics.h>

#include <System/Debug/I_DebugRender.h>

#include <gravity/sdk/Body.h>
#include <gravity/sdk/Scene.h>
#include <gravity/narrowphase/Shapes.h>	// [Blur2PC] Collision::C_CompositeShapeIterator

#include <system/Memory.h>

#include <fx/fx.h>
#include <game_resource\ModelData.h>
#include <vehicle\VehicleModel.h>
#include <world_render/ModelInstance.h>
#include <SceneGraph\renderingdata\RenderListNode.h>
#define RENDERINGDATA_ALLOW_INCLUDE
#include <renderingdata\renderingdata.h>
#undef RENDERINGDATA_ALLOW_INCLUDE

Debug::Logger& g_DamageLog = Debug::GetLogger("mercury.vehicle.damage");

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

#define ENABLE_VEHICLE_SHADER_DAMAGE
#if !defined(PLATFORM_PS3)
#define ENABLE_SPU_DAMAGE
#endif // #if !defined(PLATFORM_PS3)
//#define _DAMAGE_USE_F32
// [Blur2PC] The retail PC vehicle shaders (shaders-speed.pak) were built with _DAMAGE_USE_F32: the
// vertex shader adds the sampled damage volume straight to the position (no *4-2 decode). This source
// uploaded the 8-bit encoding (127 = no damage), so every high-detail car rendered crumpled and scratched.
// Only the volume texture changes; the info texture is read as a plain height and works in 8 bits.
#if defined(PLATFORM_PC)
#define BLUR2PC_DAMAGE_VOLUME_F32
#endif

//#pragma optimize("", off)



Debug::Bool g_debugFakeArtDamage("Vehicle.Damage.Helpers.FakeArtDamage", false);
Debug::Float g_debugFakeArtScratchLevel("Vehicle.Damage.Helpers.FakeArtScratchLevel", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Float g_debugFakeArtDeformLevel("Vehicle.Damage.Helpers.FakeArtDeformLevel", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f), Debug::Persist());

Debug::Bool g_debugActuallyBurstTyre("Vehicle.Damage.Code.Wheels.ActuallyBurstTyre", false);
Debug::Bool g_debugActuallyLoseWheel("Vehicle.Damage.Code.Wheels.ActuallyLoseWheel", false);
#if PLATFORM_PS3
Debug::Bool	g_bDisableSPUDamage("Vehicle.Damage.DisableSPUDamage", false);
#else
Debug::Bool	g_bDisableSPUDamage("Vehicle.Damage.DisableSPUDamage", false);
#endif


#if defined(PLATFORM_PS3)
namespace Vehicle
{
	SpuDamage::C_ContextManager C_DamageLayer::m_contextManager;
}
#endif // PLATFORM_PS3


namespace Vehicle
{

/* 
===============================================================
					class C_DamageLayer

NOTES: Handles damage processing
===============================================================
*/

/*
===============
NAME: C_DamageLayer

NOTE: Constructor
===============
*/
// No calls to this thanks very much.
C_DamageLayer::C_DamageLayer()
{
	return;
}

C_DamageLayer::C_DamageLayer(C_VehicleModel* pParent) :
	m_pParentModel(pParent)
#ifdef PLATFORM_PS3
	, m_pSpuDamageContext(NULL)
#endif
{
	m_UpdateVolumeTexture	= false;
	m_UpdateInfoTexture		= false;

	m_pPartList				= NULL;

	m_pPhysicsScene			= NULL;

	m_ZOffset				= 0.0f;

	m_CurrentTexture		= 0;
	m_CurrentInfoTexture	= 0;

	m_NextVolumeTextureUpdate	= 0;
	m_NextInfoTextureUpdate		= 0;

	m_FatalDamage			= false;
	m_Transitioning			= false;
	m_TransitionPeriod		= 0.0f;
	m_TransitionTime		= 0.0f;

	m_InitialScratchLevel	= 0.0f;
	m_InitialDamageLevel	= 0.0f;

	m_pVehiclePhysics		= NULL;
	m_BufferValue			= 0;					// set the value of the double buffer to the first lattice.
#if defined(PLATFORM_PC) // [Blur2PC] PC left m_pDamageVolume uninitialised
	m_pDamageVolume			= &m_DamageVolume;
#endif // #if defined(PLATFORM_XENON)

#if defined(PLATFORM_PS3)
	m_pDamageVolume			= &m_DamageVolume[m_BufferValue];

	// need to figure out where this is going to live properly.
	m_contextManager.Startup();

	// allocate the context.
	m_pSpuDamageContext = m_contextManager.Allocate();
	BIZ_ASSERT_MSG(m_pSpuDamageContext, "Couldn't allocate damage context.");
#endif // #if defined(PLATFORM_PS3)

	return;
}

#if PLATFORM_PS3
void C_DamageLayer::InitSPUDamage(MathsStorage::Float4 *pData)
{
	U32 iBodyWorkModelInstanceIndex = 0;			// Chad says (14:57):
													// in a "hacky" way, "0" will always be the body, and 1-4 will be wheels :)
	U32 iNumberOfVertexBuffers = static_cast<U32>(m_pParentModel->GetModelInstance(iBodyWorkModelInstanceIndex)->GetRenderListNode()->GetNumVertexBuffers());
	BIZ_ASSERT_MSG(iNumberOfVertexBuffers <= SpuDamage::maxNumberOfVertexBuffers, "Too many vertex buffers on car body model instance.");
	U32 iUsedVBIndex = 0;

	// get vertex declaration info for each vertex buffer on the model
	Btl::Vector<U32> positionOffsets, normalOffsets, controlPointOffsets, colourOffsets;
	positionOffsets.Resize(iNumberOfVertexBuffers);
	normalOffsets.Resize(iNumberOfVertexBuffers);
	controlPointOffsets.Resize(iNumberOfVertexBuffers);
	colourOffsets.Resize(iNumberOfVertexBuffers);
	for (int i=0; i<iNumberOfVertexBuffers; i++)
		positionOffsets[i] = normalOffsets[i] = controlPointOffsets[i] = colourOffsets[i] = UINT_MAX;

	const RenderingData::Target::RenderListNode *pRenderList = m_pParentModel->GetModelInstance(iBodyWorkModelInstanceIndex)->GetRenderListNode();

	for (U32 iPrim=0; iPrim<pRenderList->GetNumPrimitives(); iPrim++)
	{
		const RenderingData::Target::RenderListNode::Primitive &prim = pRenderList->GetPrimitive(iPrim);

		// get the vertex buffer index for this primitive
		U32 iVtxBuffer = -1;
		for (int iStream=0; iStream<prim.primitiveStreams.Size(); iStream++)
		{
			if (prim.primitiveStreams[iStream].streamIndex == 0)
			{
				iVtxBuffer = prim.primitiveStreams[iStream].vertexBuffer;
				break;
			}
		}

		BIZ_ASSERT(iVtxBuffer != -1);

		// have we already got the info for this vertex buffer?
		if (colourOffsets[iVtxBuffer] != UINT_MAX)
			continue;

		// examine the vertex declaration
		const Display2::C_VertexDeclaration *pVtxDecl = pRenderList->GetVertexDeclaration(prim.vertexDeclaration);
		const Display2::C_VertexElement *pElements = pVtxDecl->GetElements();
		for (int iElement=0; iElement< pVtxDecl->GetElementCount(); iElement++)
		{
			U8 iUsage = pElements[iElement].GetUsage(); 
			if (iUsage == Display2::E_VertexElementUsage_TexCoord)
			{
				// is this an element we're interested in? if so store the offset
				switch (pElements[iElement].GetUsageIndex())
				{
				default:
					break;
				case 2:	// position offset
					positionOffsets[iVtxBuffer] = pElements[iElement].GetOffset();
					break;
				case 3: // normal offset
					normalOffsets[iVtxBuffer] = pElements[iElement].GetOffset();
					break;
				case 4: // control point info
					controlPointOffsets[iVtxBuffer] = pElements[iElement].GetOffset();
					break;
				}
			}
			if (iUsage == Display2::E_VertexElementUsage_Colour)
			{
				colourOffsets[iVtxBuffer] = pElements[iElement].GetOffset();
				break;
			}
		}
	}

	// fill out spu damage context info
	U32 nVtxBuffers = ((iNumberOfVertexBuffers>SpuDamage::maxNumberOfVertexBuffers)?SpuDamage::maxNumberOfVertexBuffers:iNumberOfVertexBuffers);
	for(U32 iCurrentVB=0; iCurrentVB<nVtxBuffers; ++iCurrentVB)
	{
		{ 
			const RenderingData::Target::VertexBufferData& vbInfo = m_pParentModel->GetModelInstance(iBodyWorkModelInstanceIndex)->GetRenderListNode()->GetVertexBuffers(iCurrentVB);
			if(vbInfo.pBuffer)
			{
				U32 ea		= (U32)vbInfo.pBuffer->GetAddress();
				U32 size	= (U32)vbInfo.pBuffer->GetSize();
				U16 stride	= (U16)vbInfo.vertexSize;

				m_pSpuDamageContext->inputVertexBuffers[iUsedVBIndex]	= ea;
				m_pSpuDamageContext->inputStride[iUsedVBIndex]			= stride;
				m_pSpuDamageContext->inputBufferSize[iUsedVBIndex]		= size;
				m_pSpuDamageContext->positionOffsetOffset[iUsedVBIndex] = positionOffsets[iCurrentVB];
				m_pSpuDamageContext->normalOffsetOffset[iUsedVBIndex]   = normalOffsets[iCurrentVB];
				m_pSpuDamageContext->controlPointOffset[iUsedVBIndex]   = controlPointOffsets[iCurrentVB];
				m_pSpuDamageContext->colourOffset[iUsedVBIndex]		= colourOffsets[iCurrentVB];
				
				iUsedVBIndex++;
			}
		}
	}
	
	m_pSpuDamageContext->vertexBufferCount	= ((iNumberOfVertexBuffers>SpuDamage::maxNumberOfVertexBuffers)?SpuDamage::maxNumberOfVertexBuffers:iUsedVBIndex);
	m_pSpuDamageContext->transitioning		= m_Transitioning;
	m_pSpuDamageContext->fatalDamage		= m_FatalDamage;
	m_pSpuDamageContext->heightProfilePtr	= (float*)m_HeightProfile.First();
	m_pSpuDamageContext->boundsInfo[0]		= GetVehicleInfo()->m_Extents.X() * 2.1f;
	m_pSpuDamageContext->boundsInfo[1]		= GetVehicleInfo()->m_Extents.Y() * 2.1f;
	m_pSpuDamageContext->boundsInfo[2]		= GetVehicleInfo()->m_Extents.Z() * 2.1f;
	m_pSpuDamageContext->boundsInfo[3]		= GetVehicleInfo()->m_Centre.Y();
	m_pSpuDamageContext->pControlPointData	= (F32*)pData;

	m_pSpuDamageContext->forceRun = true;
	m_pSpuDamageContext->forceRunThisFrame = false;

	FinaliseSPUDamageJob();

	m_contextManager.Spin();
}
#endif

/*
===============
NAME: ForceDamageContextToRun

NOTE: Force the SPU damage context to run, regardless of number run this frame
===============
*/
void C_DamageLayer::ForceDamageContextToRun()
{
#ifdef PLATFORM_PS3
	if (m_pSpuDamageContext)
		m_pSpuDamageContext->forceRunThisFrame = true;
#endif
}

/*
===============
NAME: ~C_DamageLayer

NOTE: Constructor
===============
*/
C_DamageLayer::~C_DamageLayer()
{
#if defined(PLATFORM_PS3)
	// Free our damage context -- spins internally.
	if(m_pSpuDamageContext)
		m_contextManager.Free(m_pSpuDamageContext);
#endif // #ifdef PLATFORM_PS3
}

/*
===============
NAME: Create

NOTE: Creates layer
===============
*/
bool C_DamageLayer::Create(U32 vehicleIndex)
{
	CreateVolumeTexture(vehicleIndex);

	m_Transitioning			= false;
	m_TransitionPeriod		= 0.0f;
	m_TransitionTime		= 0.0f;

	return true;
}

/*
===============
NAME: Destroy

NOTE: Destroys layer
===============
*/
bool C_DamageLayer::Destroy()
{
	m_pPhysicsScene	= NULL;

	m_pPartList = NULL;

	m_ImpactEvents.Clear();
	m_CachedImpactEvents.Clear();
	m_pPartList = NULL;

#if !defined(ENABLE_VEHICLE_SHADER_DAMAGE)
	return true;
#endif

	m_HeightProfile.Clear();

	//Display::Device()->InvalidateState();
	//Display::Device()->SetVertexTexture(0, NULL); 

#ifdef _DAMAGE_USE_3D_TEX
	for (int iTex = 0; iTex < m_DamageVolumeTextures.Size(); iTex++)
		m_DamageVolumeTextures[iTex] = NULL;
#else
	for (U32 iTex = 0; iTex < m_DamageVolumeTextures.Size(); iTex++)
		m_DamageVolumeTextures[iTex] = NULL;
#endif

	for (int iTex = 0; iTex < m_DamageInfoTextures.Size(); iTex++)
		m_DamageInfoTextures[iTex] = NULL;
	
	m_CurrentTexture = 0;
	m_CurrentInfoTexture = 0;

	m_DamageState.Clear();

	m_CollisionShapeVertices.Clear();

	return true;
}

/*
===============
NAME: Setup

NOTE: Setup nodes
===============
*/
void C_DamageLayer::Setup(XtVehicle* p_pXtVehicle)
{
	if (!p_pXtVehicle || !p_pXtVehicle->m_DamageCurveSetup)
		return;

	m_FatalDamage		= false;
	m_SlowMotionDamage	= false;

MEMORY_TRACKER_PUSH("graphics.vehicle.damage");
	m_WidthCurve.Clear();
	m_LengthCurve.Clear();
	m_HeightCurve.Clear();

	m_ZOffset		= p_pXtVehicle->m_DamageCurveSetup->m_ZOffset;
	m_WidthCurve	= C_HermiteCurve(p_pXtVehicle->m_DamageCurveSetup->m_Width);
	m_LengthCurve	= C_HermiteCurve(p_pXtVehicle->m_DamageCurveSetup->m_Length);
	m_HeightCurve	= C_HermiteCurve(p_pXtVehicle->m_DamageCurveSetup->m_Height);
	m_HeightRange	= p_pXtVehicle->m_DamageCurveSetup->m_HeightRange == 0.0f ? 1.0f : p_pXtVehicle->m_DamageCurveSetup->m_HeightRange;
MEMORY_TRACKER_POP("graphics.vehicle.damage");


#ifdef PLATFORM_PS3

	m_pDamageVolume = &m_DamageVolume[1];
	CreateVolume();
	m_pDamageVolume = &m_DamageVolume[0];
	
#endif // #ifdef PLATFORM_PS3

	CreateVolume();	// Create the first of the double buffer -- single buffer on 360.
	SetupDeformRanges(p_pXtVehicle->m_DeformationSetup);
	SetupWheelDamage(&p_pXtVehicle->m_WheelDamageSetup);

	m_CanBurstTyres = !p_pXtVehicle->m_WheelDamageSetup.m_DontBurstTyres;
	m_CanLoseWheels = !p_pXtVehicle->m_WheelDamageSetup.m_DontLoseWheels;
}

Debug::Bool g_debugModifyVolume("Vehicle.Damage.Debug.ModifyVolume", false);
Debug::Bool g_debugUpdateVolumeTexture("Vehicle.Damage.Debug.UpdateVolumeTexture", false);

Debug::Int g_debugDamageVolumeCurrentLayer("Vehicle.Damage.Debug.CurrentLayer", -1, Debug::Range(-1, 3), Debug::Step(1));
Debug::Int g_debugDamageVolumeCurrentWidthNode("Vehicle.Damage.Debug.CurrentWidthNode", -1, Debug::Range(-1, 10), Debug::Step(1));
Debug::Int g_debugDamageVolumeCurrentLengthNode("Vehicle.Damage.Debug.CurrentLengthNode", -1, Debug::Range(-1, 20), Debug::Step(1));

Debug::Float g_debugNodeOffsetX("Vehicle.Damage.Debug.NodeOffsetX", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugNodeOffsetY("Vehicle.Damage.Debug.NodeOffsetY", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugNodeOffsetZ("Vehicle.Damage.Debug.NodeOffsetZ", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_debugClearCachedEvents("Vehicle.Damage.Debug.ClearCachedEvents", false);
Debug::Bool g_debugResetDamageVolume("Vehicle.Damage.ResetVolume", false);

Debug::Bool g_debugDamageSaveTexture("Vehicle.Damage.SaveTexture", false);

Debug::Bool g_debugSaveDamageState("Vehicle.Damage.Replay.SaveState", false);
Debug::Bool g_debugRestoreDamageState("Vehicle.Damage.Replay.RestoreState", false);
Debug::Bool g_debugRepairVehicle("Vehicle.Damage.Replay.Repair", false);

Debug::Int g_debugModifyWheelIndex("Vehicle.Damage.Wheels.PerformanceModifiers.WheelIndex", 0, Debug::Range(0, 3), Debug::Step(1));
Debug::Bool g_debugUpdateWheelPerformanceModifiers("Vehicle.Damage.Wheels.PerformanceModifiers.Update", false);
Debug::Bool g_debugLoseTyre("Vehicle.Damage.Wheels.PerformanceModifiers.LoseTyre", false);
Debug::Bool g_debugLoseWheel("Vehicle.Damage.Wheels.PerformanceModifiers.LoseWheel", false);
Debug::Float g_debugTyreLossGripScale("Vehicle.Damage.Wheels.PerformanceModifiers.TyreLossGripScale", 1.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));
Debug::Float g_debugTyreLossFrictionScale("Vehicle.Damage.Wheels.PerformanceModifiers.TyreLossFrictionScale", 0.0f, Debug::Range(0.0f, 500.0f), Debug::Step(0.001f));
Debug::Float g_debugWheelLossGripScale("Vehicle.Damage.Wheels.PerformanceModifiers.WheelLossGripScale", 1.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));
Debug::Float g_debugWheelLossFrictionScale("Vehicle.Damage.Wheels.PerformanceModifiers.WheelLossFrictionScale", 0.0f, Debug::Range(0.0f, 500.0f), Debug::Step(0.001f));

Debug::Int g_debugUpdatesBetweenVolumeTextureUpdate("Vehicle.Damage.Perf.UpdatesBetweenVolumeTextureUpdates", 4, Debug::Range(0, 30), Debug::Step(1));
Debug::Int g_debugUpdatesBetweenInfoTextureUpdate("Vehicle.Damage.Perf.UpdatesBetweenInfoTextureUpdates", 4, Debug::Range(0, 30), Debug::Step(1));

/*
===============
NAME: SetUpdateVolumeTexture

NOTE: Set update for volume texture
===============
*/
void C_DamageLayer::SetUpdateVolumeTexture(bool p_Update)
{
	if (m_UpdateVolumeTexture)
		return;

	m_UpdateVolumeTexture = p_Update;
	//m_NextVolumeTextureUpdate = (int)g_debugUpdatesBetweenVolumeTextureUpdate;
}

/*
===============
NAME: SetUpdateInfoTexture

NOTE: Set update for info texture
===============
*/
void C_DamageLayer::SetUpdateInfoTexture(bool p_Update)
{
	if (m_UpdateInfoTexture)
		return;

	m_UpdateInfoTexture = true;
	
	#ifdef PLATFORM_PS3
		m_pSpuDamageContext->forceRun = true;
	#endif
	
	//m_NextInfoTextureUpdate = (int)g_debugUpdatesBetweenInfoTextureUpdate;
}

/*
Debug::Bool g_debugFakeArtDamage("Vehicle.Damage.Helpers.FakeArtDamage", false);
Debug::Float g_debugFakeArtScratchLevel("Vehicle.Damage.Helpers.FakeArtScratchLevel", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f), Debug::Persist());
Debug::Float g_debugFakeArtDeformLevel("Vehicle.Damage.Helpers.FakeArtDeformLevel", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f), Debug::Persist());

Debug::Bool g_debugActuallyBurstTyre("Vehicle.Damage.Code.Wheels.ActuallyBurstTyre", false);
Debug::Bool g_debugActuallyLoseWheel("Vehicle.Damage.Code.Wheels.ActuallyLoseWheel", false);
*/

Debug::Bool g_debugDoSomeScraping("Vehicle.Damage.Scrape.Simulate", false);

Debug::Float g_debugScrapePosX("Vehicle.Damage.Scrape.PosX", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(1.0f));
Debug::Float g_debugScrapePosY("Vehicle.Damage.Scrape.PosY", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugScrapePosZ("Vehicle.Damage.Scrape.PosZ", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugScrapeDirX("Vehicle.Damage.Scrape.DirX", 0.0f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));
Debug::Float g_debugScrapeDirY("Vehicle.Damage.Scrape.DirY", 0.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugScrapeDirZ("Vehicle.Damage.Scrape.DirZ", -1.0f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: Update

NOTE: Updates layer
===============
*/
void C_DamageLayer::Update()
{
	PERF_EVENT("C_DamageLayer::Update");

#ifndef CFG_GOLD

	if (g_debugActuallyBurstTyre || g_debugActuallyLoseWheel)
		UpdateWheelDamage(NULL);

	bool bFakeArtDamage = g_debugFakeArtDamage;
	if (bFakeArtDamage)
	{
		for (S32 iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
		{
			for (S32 iNode = 0; iNode < (*m_pDamageVolume)[iLayer].Size(); iNode++)
			{
				C_Node* pNode = &(*m_pDamageVolume)[iLayer].m_Nodes[iNode];
				pNode->m_ScratchLevel = (float)g_debugFakeArtScratchLevel;
				pNode->m_DeformLevel = (float)g_debugFakeArtDeformLevel;
			}
		}
		m_UpdateVolumeTexture = true;
		m_UpdateInfoTexture = true;
		g_debugFakeArtDamage = false;
	}
#endif

	if (g_debugUpdateWheelPerformanceModifiers)
	{
		C_WheelDamage* pWheel = GetWheel(g_debugModifyWheelIndex);
		if (pWheel && g_debugLoseTyre)
		{
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].radiusScale = m_WheelSetup.m_FrontTyreLossRadiusScale;
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].gripScale = pWheel->m_Grip * (float)g_debugTyreLossGripScale;
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].rollingFriction = pWheel->m_Friction + (float)g_debugTyreLossFrictionScale;
		}
		if (pWheel && g_debugLoseWheel)
		{
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].radiusScale = m_WheelSetup.m_FrontWheelLossRadiusScale;
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].gripScale = pWheel->m_Grip * (float)g_debugWheelLossGripScale;
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].rollingFriction = pWheel->m_Friction + (float)g_debugWheelLossFrictionScale;
		}
		if (!g_debugLoseTyre && !g_debugLoseWheel)
		{
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].radiusScale = 1.0f;
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].gripScale = pWheel->m_Grip;
			m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[pWheel->m_Index].rollingFriction = pWheel->m_Friction;
		}
	}

	if (g_debugSaveDamageState)
	{
		SaveDamageState(&m_DamageState);
		g_debugSaveDamageState = false;
	}
	if (g_debugRestoreDamageState)
	{
		RestoreDamageState(&m_DamageState);
		g_debugRestoreDamageState = false;
	}
	if (g_debugRepairVehicle)
	{
		Repair(1.0f, 0.0f, 1.0f);
		g_debugRepairVehicle = false;
	}

	//TODO: Need a timer class so don't hard-code this!
	UpdateTransition(1.0f / 30.0f);

	m_DefferedEvents.Clear();

	if (g_debugDoSomeScraping)
	{
		C_ImpactEvent impact;
		float x = Maths::Lerp(m_VehicleInfo.m_Centre.X() + m_VehicleInfo.m_Extents.X(), m_VehicleInfo.m_Centre.X() - m_VehicleInfo.m_Extents.X(), g_debugScrapePosX);
		float y = Maths::Lerp(m_VehicleInfo.m_Centre.Y() + m_VehicleInfo.m_Extents.Y(), m_VehicleInfo.m_Centre.Y() - m_VehicleInfo.m_Extents.Y(), g_debugScrapePosY);
		float z = Maths::Lerp(m_VehicleInfo.m_Centre.Z() + m_VehicleInfo.m_Extents.Z(), m_VehicleInfo.m_Centre.Z() - m_VehicleInfo.m_Extents.Z(), g_debugScrapePosZ);
		impact.m_LocalPosition = Maths::HVec3(x, y, z);
		float dx = -g_debugScrapeDirX;
		if (g_debugScrapePosX < 0.5f)
			dx *= -1.0f;
		impact.m_LocalFriction = Maths::HVec3(-g_debugScrapeDirX, -g_debugScrapeDirY, -g_debugScrapeDirZ);
		impact.m_LocalFriction.Normalise();
		Scrape(&impact);
		m_UpdateVolumeTexture = true;
		m_UpdateInfoTexture = true;
	}

	if (!g_bDisableSPUDamage)
	{
		ProcessDeformEvents();
		ProcessImpactEvents();
	}
	if (g_debugClearCachedEvents)
	{
		m_CachedImpactEvents.Clear();
		g_debugClearCachedEvents = false;
	}

	if (g_debugModifyVolume)
	{
		S32 currentLayer		= g_debugDamageVolumeCurrentLayer;
		S32 currentWidthNode	= g_debugDamageVolumeCurrentWidthNode;
		S32 currentLengthNode	= g_debugDamageVolumeCurrentLengthNode;

		for (S32 iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
		{
			bool layerActive = (currentLayer == iLayer || currentLayer == -1) ? true : false;

			for (S32 iWidth = 0; iWidth < (*m_pDamageVolume)[iLayer].GetWidth(); iWidth++)
			{
				bool widthActive = (currentWidthNode == iWidth || currentWidthNode == -1) ? true : false;

				for (S32 iLength = 0; iLength < (*m_pDamageVolume)[iLayer].GetLength(); iLength++)
				{
					bool lengthActive = (currentLengthNode == iLength || currentLengthNode == -1) ? true : false;

					S32 index = (iLength * (*m_pDamageVolume)[iLayer].GetWidth()) + iWidth;

					C_Node* pNode = &(*m_pDamageVolume)[iLayer].m_Nodes[index];
	
					pNode->m_Active = (layerActive && widthActive && lengthActive);

					if (pNode->m_Active)
					{
						Maths::HVec3 offset((float)g_debugNodeOffsetX, (float)g_debugNodeOffsetY, (float)g_debugNodeOffsetZ);
						pNode->m_Position = pNode->m_OriginalPosition + offset;
					}
				}
			}
		}

		if (g_debugUpdateVolumeTexture)
			UpdateVolumeTexture();
	}

	if (g_debugResetDamageVolume)
	{
		ResetVolume();
		g_debugResetDamageVolume = false;
	}
	
	UpdateDamageTextures();

	if (m_TransitionTime >= m_TransitionPeriod)
	{
		m_TransitionTime	= m_TransitionPeriod;
		m_Transitioning		= false;	
		m_SlowMotionDamage	= false;
	}


}

/*
===============
NAME: UpdateDamageTextures

NOTE: Updates damage textures
===============
*/
void C_DamageLayer::UpdateDamageTextures()
{
#ifndef PLATFORM_PS3
	m_NextVolumeTextureUpdate--;
	
	if (m_SlowMotionDamage || (m_UpdateVolumeTexture && m_NextVolumeTextureUpdate < 0))
	{
		UpdateVolumeTexture();
		m_UpdateVolumeTexture = false;
	}
	if (m_NextVolumeTextureUpdate < 0)
		m_NextVolumeTextureUpdate = 0;
	
	m_NextInfoTextureUpdate--;
	if (m_SlowMotionDamage || (m_UpdateInfoTexture && m_NextInfoTextureUpdate < 0))
	{
		UpdateInfoTexture();
		m_UpdateInfoTexture = false;
	}
	if (m_NextInfoTextureUpdate < 0)
		m_NextInfoTextureUpdate = 0;
#else
	// smooth update on ps3 ;-)
	if(m_UpdateVolumeTexture)
		m_pSpuDamageContext->forceRun = true;
#endif

}

/*
===============
NAME: ResetVolume

NOTE: Resets the damage volume
===============
*/
void C_DamageLayer::ResetVolume()
{
	for (int iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
	{
		for (int iColumn = 0; iColumn < (*m_pDamageVolume)[iLayer].GetWidth(); iColumn++)
		{
			for (int iRow = 0; iRow < (*m_pDamageVolume)[iLayer].GetLength(); iRow++)
			{
				C_Node* pNode = (*m_pDamageVolume)[iLayer].GetNode(iRow, iColumn);
				pNode->Reset(true);
			}
		}
	}
}

Debug::Float g_debugDeformTransition("Vehicle.Damage.DeformTransitionPeriod", 0.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));

/*
===============
NAME: ProcessImpactEvents

NOTE: Process any stored impact events
===============
*/
void C_DamageLayer::ProcessImpactEvents()
{
	if (m_Transitioning)
		return;

	for (int iEvent = 0; iEvent < m_ImpactEvents.Size(); iEvent++)
	{
		C_ImpactEvent* pEvent = &m_ImpactEvents[iEvent];
		if (pEvent->m_bProcessParts)
			ProcessParts(pEvent);
		if (pEvent->m_bDeform)
			UpdateWheelDamage(pEvent);
		if (pEvent->m_bScrape)
			Scrape(pEvent);
	}

	m_ImpactEvents.Clear();
}

/*
===============
NAME: ProcessDeformEvents

NOTE: Process any stored impact events
===============
*/
void C_DamageLayer::ProcessDeformEvents()
{
	if (m_Transitioning)
		return;


	for (int iEvent = 0; iEvent < m_ImpactEvents.Size(); iEvent++)
	{
		C_ImpactEvent* pEvent = &m_ImpactEvents[iEvent];
		if (pEvent->m_bDeform)
		{
			Deform(pEvent, (float)g_debugDeformTransition);
#if defined(PLATFORM_PS3)
			m_pSpuDamageContext->forceRun = true;			// run the damage.
#endif // #if defined(PLATFORM_PS3)
		}
	}
	
	FinaliseSPUDamageJob();
}

void C_DamageLayer::FinaliseSPUDamageJob()
{
#if PLATFORM_PS3

	if(m_pSpuDamageContext)
	{
		// Dump the damage offset vertex buffers!
		U32 iNumberOfVertexBuffers = static_cast<U32>(m_pParentModel->GetModelInstance(0)->GetRenderListNode()->GetNumVertexBuffers());
			
		if(m_pSpuDamageContext->forceRun)		// kick if repairing.
		{
			m_pSpuDamageContext->damageLayerPtr		= (uint32_t)&(*m_pDamageVolume)[0];	// Pointer to the damage volume.

			U32 iUsedOutputBuffers = 0;			
			// Output vertex buffer EAs -- one for each input vertex buffer.
			for(U32 iCurrentVB=0; iCurrentVB<((iNumberOfVertexBuffers>SpuDamage::maxNumberOfVertexBuffers)?SpuDamage::maxNumberOfVertexBuffers:iNumberOfVertexBuffers); ++iCurrentVB)
			{
				if(m_pParentModel->GetDamageOffsetVertexBuffers()[iCurrentVB])
				{
					BIZ_ASSERT(m_pParentModel->GetDamageOffsetVertexBuffers()[iCurrentVB]->m_Data.GetSize());
					BIZ_ASSERT(m_pParentModel->GetDamageOffsetVertexBuffers()[iCurrentVB]->m_Data.GetAddress());

					m_pSpuDamageContext->outputVertexBuffers[iUsedOutputBuffers]	= (U32)m_pParentModel->GetDamageOffsetVertexBuffers()[iCurrentVB]->m_Data.GetAddress(); 
					m_pSpuDamageContext->outputBufferSize[iUsedOutputBuffers]		= m_pParentModel->GetDamageOffsetVertexBuffers()[iCurrentVB]->m_Data.GetSize(); 
					m_pSpuDamageContext->outputStride[iUsedOutputBuffers]			= 0x10;		// TODO: [Stove] sort this out.
					iUsedOutputBuffers++;
				}
			}
			
			m_contextManager.FinaliseContext(m_pSpuDamageContext);
			if (!m_pSpuDamageContext->forceRun)
				SetUpdateVolumeTexture(false);
			SwapBuffers();		// swap the lattice.
		}
	}
	
#endif // #ifndef PLATFORM_PS3
}

Debug::Float g_debugDetachedWheelRadiusScale("Vehicle.Damage.Parts.DetachedWheelRadiusScale", 0.25f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugPartImpactSizeToHinge("Vehicle.Damage.Parts.ImpactSizeToHinge", 0.1f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugPartImpactSizeToDetach("Vehicle.Damage.Parts.ImpactSizeToDetach", 0.3f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugPartImpactRadiusScale("Vehicle.Damage.Parts.RadiusScale", 3.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugPartDamageAmount("Vehicle.Damage.Parts.DamageAmount", 0.35f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Enum<E_PartCode> g_debugRTPartCode("Vehicle.Damage.Debug.RayTracePartCode", E_PartCode_None);
Debug::Enum<E_PartSubCode> g_debugRTSubCode("Vehicle.Damage.Debug.RayTraceSubCode", E_PartSubCode_None);

Debug::Bool g_debugEnableFlappingParts("Vehicle.Damage.Parts.EnableFlapping", true);
Debug::Bool g_debugLetWheelsFallOff("Vehicle.Damage.Parts.LetWheelsFallOff", true);
Debug::Bool g_debugOnlyAllowOneWheelOff("Vehicle.Damage.Parts.OnlyKnockOneWheelOff", true);

#ifdef WIP_PART_CONSTRAINTS
static Util::C_Random g_PartRandom;
#endif

/*
===============
NAME: UpdateDamageBehaviour

NOTE: Update the behaviour status of the damaged part
===============
*/
void C_DamageLayer::UpdateDamageBehaviour(C_DamageFilter* p_pFilter, C_Part* p_pPart, bool repairing)
{
	if (!p_pPart || !m_pPhysicsScene || (p_pPart->m_Damage.m_Detached && !repairing))
		return;

	if (repairing)
	{
		bool forceReattach = false;

		//if (p_pPart->m_PartCode == E_PartCode_Wheel && p_pPart->m_Damage.m_Detached)
		//{
		//	U32 index = 9999;
		//	switch (p_pPart->m_PartSubCode)
		//	{
		//		case E_PartSubCode_FrontLeft:
		//			index = 0;
		//			break;
		//		case E_PartSubCode_FrontRight:
		//			index = 1;
		//			break;
		//		case E_PartSubCode_BackLeft:
		//			index = 2;
		//			break;
		//		case E_PartSubCode_BackRight:
		//			index = 3;
		//			break;
		//	}
		//	if (index < m_WheelNodes.Size())
		//		m_WheelNodes[index].m_Detached = false;
		//	if (index < m_WheelNodes.Size())
		//		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[index].radiusScale = m_WheelNodes[index].m_Radius;
		//	forceReattach = true;
		//}

		if (p_pPart->m_pPhysicsBody && p_pPart->m_Damage.m_Detached &&
			(p_pPart->m_Damage.m_DamageLevel < p_pPart->m_Damage.m_Behaviour.m_DetachDamageLevel || forceReattach))
		{
			p_pPart->Reattach();
		}
		else if (	p_pPart->m_pPhysicsBody && 
					p_pPart->m_Damage.m_DamageLevel < p_pPart->m_Damage.m_Behaviour.m_DetachDamageLevel &&
					!p_pPart->m_Damage.m_Detached)
		{
			p_pPart->DeactivateConstraints();
		}

		return;
	}

	bool allowDetaching = (p_pFilter && p_pFilter->m_DetachParts) ? true : false;

	//if (p_pPart->m_PartCode == E_PartCode_Wheel)
	//{
	//	allowDetaching = m_FatalDamage ? true : false;
	//	if (g_debugLetWheelsFallOff)
	//	{
	//		allowDetaching = true;
	//		if (g_debugOnlyAllowOneWheelOff && m_WheelNodes.Size() == 4)
	//		{
	//			if (
	//				(p_pPart->m_PartSubCode == E_PartSubCode_FrontLeft && m_WheelNodes[1].m_Detached) ||
	//				(p_pPart->m_PartSubCode == E_PartSubCode_FrontRight && m_WheelNodes[0].m_Detached) ||
	//				(p_pPart->m_PartSubCode == E_PartSubCode_BackLeft && m_WheelNodes[3].m_Detached) ||
	//				(p_pPart->m_PartSubCode == E_PartSubCode_BackRight && m_WheelNodes[2].m_Detached)
	//				)
	//				allowDetaching = false;
	//		}
	//	}
	//}
	
	if (p_pPart->m_pPhysicsBody && p_pPart->m_Damage.m_DamageLevel >= p_pPart->m_Damage.m_Behaviour.m_DetachDamageLevel && 
		p_pPart->m_Damage.m_ImpactSize >= (float)g_debugPartImpactSizeToDetach && !p_pPart->m_Damage.m_Detached)
	{
		if (allowDetaching)
		{
			p_pPart->Detach(m_VehicleInfo.m_Velocity);	

			//if (p_pPart->m_PartCode == E_PartCode_Wheel && p_pPart->m_Damage.m_Detached)
			//{
			//	U32 index = 9999;
			//	switch (p_pPart->m_PartSubCode)
			//	{
			//		case E_PartSubCode_FrontLeft:
			//			index = 0;						
			//			break;
			//		case E_PartSubCode_FrontRight:
			//			index = 1;
			//			break;
			//		case E_PartSubCode_BackLeft:
			//			index = 2;
			//			break;
			//		case E_PartSubCode_BackRight:
			//			index = 3;
			//			break;
			//	}
			//	if (index < m_WheelNodes.Size())
			//		m_WheelNodes[index].m_Detached = true;
			//	if (index < m_WheelNodes.Size())
			//		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[index].radiusScale = (float)g_debugDetachedWheelRadiusScale;
			//}
		}
	}
	else if (p_pPart->m_Damage.m_DamageLevel >= p_pPart->m_Damage.m_Behaviour.m_FlapDamageLevel && 
		p_pPart->m_Damage.m_ImpactSize >= (float)g_debugPartImpactSizeToHinge && !p_pPart->m_Damage.m_Detached && !p_pPart->m_Damage.m_HingeActive &&
		g_debugEnableFlappingParts)
	{
		if (p_pFilter && p_pFilter->m_DislodgeParts)
		{
#ifdef WIP_PART_CONSTRAINTS
			if (p_pPart->m_PartCode == E_PartCode_Bumper)
			{
				p_pPart->m_CurrentConstraint = g_PartRandom.Rand(p_pPart->m_pConstraints.Size()-1);
			}
#endif
			p_pPart->ActivateConstraints();
		}
	}

	////Check for wheel clipping with wings
	//if (!repairing && p_pPart->m_PartCode == E_PartCode_Wing)
	//{
	//	int index = -1;

	//	C_Node* pNode = NULL;
	//	switch (p_pPart->m_PartSubCode)
	//	{
	//	case E_PartSubCode_FrontLeft:
	//		index = 0;
	//		pNode = m_WheelNodes[index].m_pMinNode;
	//		break;
	//	case E_PartSubCode_BackLeft:
	//		index = 2;
	//		pNode = m_WheelNodes[index].m_pMaxNode;
	//		break;
	//	case E_PartSubCode_FrontRight:
	//		index = 1;
	//		pNode = m_WheelNodes[index].m_pMinNode;
	//		break;
	//	case E_PartSubCode_BackRight:
	//		index = 3;
	//		pNode = m_WheelNodes[index].m_pMaxNode;
	//		break;
	//	}

	//	if (index != -1 && pNode)
	//	{
	//		Maths::HVec3 offsetVec = m_WheelNodes[index].m_pNode->m_Position - pNode->m_Position;
	//		float offsetLen = offsetVec.Length();
	//		if (offsetLen <= m_WheelNodes[index].m_Radius)
	//			p_pPart->Detach(m_VehicleInfo.m_Velocity);
	//	}
	//}
}

Debug::Bool g_debugDisablePartDamage("Vehicle.Damage.Parts.DisablePartDamage", false);
Debug::Float g_debugPartRadiusHack("Vehicle.Damage.Parts.RadiusHack", 0.5f, Debug::Range(0.0f, 2.0f), Debug::Step(0.001f));
Debug::Bool g_debugDamageFindClosestPart("Vehicle.Damage.Parts.FindClosest", true);
Debug::Float g_debugPartDamageRadius("Vehicle.Damage.NEW.Parts.ImpactRadius", 1.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));

extern Debug::Bool g_debugKeepDamageStateUpToDate;

/*
===============
NAME: ProcessParts

NOTE: See if any parts need damaging
===============
*/
void C_DamageLayer::ProcessParts(C_ImpactEvent* p_pEvent)
{
	PERF_EVENT("C_DamageLayer.ProcessParts");

	if (!m_pPartList || g_debugDisablePartDamage)
		return;

	float fImpactRadius = p_pEvent->m_ImpactSize * (float)g_debugPartDamageRadius;
	Maths::C_Sphere impactSphere(p_pEvent->m_LocalPosition, fImpactRadius);

	S32 partOffset = 8;
	S32 numNodes = (*m_pDamageVolume).Size() > 1 ? (*m_pDamageVolume)[1].GetWidth() * (*m_pDamageVolume)[1].GetLength() : 0;

	for (int iPart = 0; iPart < m_pPartList->m_DamageParts.Size(); iPart++)
	{
		C_Part* pNode = m_pPartList->m_DamageParts[iPart];
		
		//if (g_debugKeepDamageStateUpToDate)
		//{
		//	S32 nodeIndex = iPart - partOffset;
		//	if (nodeIndex >= 0 && nodeIndex < numNodes)
		//	{
		//		m_DamageState.m_State[nodeIndex].m_PartCode		= pNode->m_PartCode;
		//		m_DamageState.m_State[nodeIndex].m_PartSubCode	= pNode->m_PartSubCode;
		//		m_DamageState.m_State[nodeIndex].m_Hinged		= (DWORD)(pNode->m_Damage.m_HingeActive ? 1 : 0);
		//		m_DamageState.m_State[nodeIndex].m_Detached		= (DWORD)(pNode->m_Damage.m_Detached ? 1 : 0);
		//		m_DamageState.m_State[nodeIndex].m_Level		= (DWORD)(pNode->m_Damage.m_DamageLevel * 15.0f);
		//	}
		//}

		if (pNode->m_Damage.m_Detached)
			continue;

		//Maths::C_Sphere partSphere = pNode->m_pElement->GetLocalBounds().BoundingSphere();
		/*Maths::HVec3 centre = pNode->m_RestMatrix.AW() + pNode->m_pElement->GetLocalBounds().BoundingSphere().Centre();
		Maths::C_Sphere partSphere(centre, pNode->m_pElement->GetLocalBounds().BoundingSphere().Radius()); */

		Maths::HVec3 centre = pNode->m_RestMatrix.AW() + pNode->m_pElement->GetBoundingSphere().Centre();
		float is = pNode->m_Damage.m_Behaviour.m_ImpactScale == 0.0f ? 1.0f : pNode->m_Damage.m_Behaviour.m_ImpactScale;		
		float fPartRadius = pNode->m_pElement->GetBoundingSphere().Radius() * is;
		Maths::C_Sphere partSphere(centre, fPartRadius); 

		//impactSphere.Distance(partSphere);

		if (!impactSphere.Intersect(partSphere))
			continue;

		float fDist2 = (impactSphere.Centre() - partSphere.Centre()).LengthSquared();
		float fRange2 = (impactSphere.Radius() + fPartRadius) * (impactSphere.Radius() + fPartRadius);
		float fScale = Maths::Clamp(fDist2 / fRange2, 0.0f, 1.0f);
		fScale *= p_pEvent->m_ImpactSize;

		float damageAmount = fScale * (float)g_debugPartDamageAmount * is;

		float newDamageLevel = Maths::Clamp(pNode->m_Damage.m_DamageLevel + damageAmount, 0.0f, 1.0f);

		pNode->SetImpactSize(fScale);

		if (m_Transitioning)
			pNode->SetTargetDamageLevel(newDamageLevel);
		else
		{
			E_DefferedEvent dEvent = pNode->HandleDamage(newDamageLevel);
			if (dEvent != E_DefferedEvent_Unknown || (pNode->m_PartCode > E_PartCode_WheelLod && pNode->m_PartCode < E_PartCode_SteeringWheel))
				AddDefferedDamageEffect(dEvent, pNode->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), fScale, pNode->m_PartCode, pNode->m_PartSubCode);
			E_PartCode newPartCode = pNode->m_PartCode;
			E_PartSubCode newPartSubCode = pNode->m_PartSubCode;
			dEvent = HandleDamageDependancies(dEvent, pNode, newPartCode, newPartSubCode);
			if (dEvent != E_DefferedEvent_Unknown)
				AddDefferedDamageEffect(dEvent, pNode->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), fScale, pNode->m_PartCode, pNode->m_PartSubCode);
						
			if (pNode->m_pDamageBehaviour)
			{
				dEvent = pNode->m_pDamageBehaviour->HandleImpactDamage(fScale, &p_pEvent->m_Filter);
				if (dEvent != E_DefferedEvent_Unknown)
				{
					AddDefferedDamageEffect(dEvent, pNode->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), fScale, pNode->m_PartCode, pNode->m_PartSubCode);					
					E_PartCode newPartCode = pNode->m_PartCode;
					E_PartSubCode newPartSubCode = pNode->m_PartSubCode;
					HandleDamageDependancies(dEvent, pNode, newPartCode, newPartSubCode);
				}
			}
			
			UpdateDamageBehaviour(&p_pEvent->m_Filter, pNode);
		}

		for (int iTrans = 0; iTrans < pNode->m_Damage.m_Transfer.Size(); iTrans++)
		{
			C_Part* pTransfer = pNode->m_Damage.m_Transfer[iTrans].m_pNode;
		
			if (!pTransfer)
				continue;

			float tis = pTransfer->m_Damage.m_Behaviour.m_ImpactScale == 0.0f ? 1.0f : pTransfer->m_Damage.m_Behaviour.m_ImpactScale;
			float newDamageAmount = Maths::Clamp(damageAmount * tis * pNode->m_Damage.m_Transfer[iTrans].m_Percentage, 0.0f, 1.0f);
			newDamageLevel = Maths::Clamp(pTransfer->m_Damage.m_DamageLevel + newDamageAmount, 0.0f, 1.0f);

			pTransfer->SetImpactSize(fScale * pNode->m_Damage.m_Transfer[iTrans].m_Percentage);

			if (m_Transitioning)
				pTransfer->SetTargetDamageLevel(newDamageLevel);			
			else
			{
				E_DefferedEvent dEvent = pTransfer->HandleDamage(newDamageLevel);				
				if (dEvent != E_DefferedEvent_Unknown)
					AddDefferedDamageEffect(dEvent, pTransfer->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), p_pEvent->m_ImpactSize, pTransfer->m_PartCode, pTransfer->m_PartSubCode);
				
				if (pTransfer->m_pDamageBehaviour)
				{
					dEvent = pTransfer->m_pDamageBehaviour->HandleImpactDamage(pNode->m_Damage.m_Transfer[iTrans].m_Percentage * p_pEvent->m_ImpactSize, &p_pEvent->m_Filter);
					if (dEvent != E_DefferedEvent_Unknown)
						AddDefferedDamageEffect(dEvent, pTransfer->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), fScale, pTransfer->m_PartCode, pTransfer->m_PartSubCode);		

					E_PartCode newPartCode = pTransfer->m_PartCode;
					E_PartSubCode newPartSubCode = pTransfer->m_PartSubCode;
					dEvent = HandleDamageDependancies(dEvent, pTransfer, newPartCode, newPartSubCode);
					if (dEvent != E_DefferedEvent_Unknown)
						AddDefferedDamageEffect(dEvent, pTransfer->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), fScale, newPartCode, newPartSubCode);
				}
				UpdateDamageBehaviour(&p_pEvent->m_Filter, pTransfer);
			}		
		}
	}

	//m_FatalDamage = false;
}

/*
===============
NAME: HandleDamageDependancies

NOTE: Handles any damage dependancies
===============
*/
E_DefferedEvent C_DamageLayer::HandleDamageDependancies(E_DefferedEvent p_Event, C_Part* p_pPart, E_PartCode& p_PartCode, E_PartSubCode& p_PartSubCode)
{
	if (p_Event == E_DefferedEvent_LightCrack || p_Event == E_DefferedEvent_LightSmash)
	{
		C_Part* pLightCover = NULL;
		if (p_pPart->m_PartCode == E_PartCode_HeadLight)
			pLightCover = &m_pPartList->m_Parts[E_PartCode_HeadlightCover].m_Nodes[p_pPart->m_PartSubCode];
		else if (p_pPart->m_PartCode == E_PartCode_FogLight)
			pLightCover = &m_pPartList->m_Parts[E_PartCode_FogLightCover].m_Nodes[p_pPart->m_PartSubCode];

		if (pLightCover && pLightCover->m_pDamageBehaviour && pLightCover->m_pDamageBehaviour->GetType() == Damage::E_BehaviourType_GlassSmash)
		{
			p_PartCode		= pLightCover->m_PartCode;
			p_PartSubCode	= pLightCover->m_PartSubCode;
			Damage::C_BehaviourGlassSmash* pBehaviour = (Damage::C_BehaviourGlassSmash*)pLightCover->m_pDamageBehaviour;
			return pBehaviour->Break();
		}
	}

	return E_DefferedEvent_Unknown;
}

//void C_DamageLayer::ProcessParts(C_ImpactEvent* p_pEvent)
//{
//	if (!m_pPartList || g_debugDisablePartDamage)
//		return;
//
//	LOGDEBUG(g_DamageLog, "C_DamageLayer::ProcessParts()");
//	
//	//TODO: Ray trace to find what part was hit - for now hard-code it though...
//	C_Part* pHitPart = &m_pPartList->m_Parts[g_debugRTPartCode].m_Nodes[g_debugRTSubCode];
//
//	if (g_debugDamageFindClosestPart)
//	{
//		pHitPart = NULL;
//
//		float fDist = 9999.0f;
//
//		for (U32 iPart = 0; iPart < m_pPartList->m_ActiveParts.Size(); iPart++)
//		{
//			C_Part* pNode = m_pPartList->m_ActiveParts[iPart].m_pNode;
//			if (pNode->m_Damage.m_Detached || !pNode->m_pPhysicsBody)
//				continue;
//
//			Collision::C_CRAxisBox box = pNode->m_pPhysicsBody->GetLocalBoundingBox();
//			box.centre = pNode->m_RestMatrix.AW();
//
//			float dist = (p_pEvent->m_LocalPosition - box.centre).Length();
//			if (dist < fDist)
//			{
//				pHitPart = pNode;
//				fDist = dist;
//			}
//		}
//	}
//
//	if (!pHitPart)
//		return;
//	
//	//Quick hack - get distance to centre
//	float dist = (p_pEvent->m_LocalPosition - pHitPart->m_RestMatrix.AW()).Length();
//	float radius = (float)g_debugPartRadiusHack;
//	dist -= radius;
//
//	float distScale = 1.0f - Maths::Clamp(dist / (p_pEvent->m_ImpactRadius * (float)g_debugPartImpactRadiusScale), 0.0f, 1.0f);
//	
//#ifndef VEHICLE_MASTER_BUILD
//	LOGDEBUG(g_DamageLog, "Node Name: %s", pHitPart->debug_Name.Chars());
//#endif
//	
//	if (distScale == 0.0f)
//		return;
//
//	float impactSize = p_pEvent->m_ImpactSize * distScale;
//
//	float is = pHitPart->m_Damage.m_Behaviour.m_ImpactScale == 0.0f ? 1.0f : pHitPart->m_Damage.m_Behaviour.m_ImpactScale;		
//	float damageAmount = impactSize * (float)g_debugPartDamageAmount * is;
//
//	float newDamageLevel = Maths::Clamp(pHitPart->m_Damage.m_DamageLevel + damageAmount, 0.0f, 1.0f);
//
//	LOGDEBUG(g_DamageLog, "Apply damage %.3f damage to part '%s' - old damage = %.3f, new damage = %.3f", 
//								pHitPart->debug_Name.Chars(), pHitPart->m_Damage.m_DamageLevel, newDamageLevel);
//
//	pHitPart->SetImpactSize(impactSize);
//
//	if (m_Transitioning)
//		pHitPart->SetTargetDamageLevel(newDamageLevel);
//	else
//	{
//		pHitPart->HandleDamage(newDamageLevel);
//		if (pHitPart->m_pDamageBehaviour)
//			pHitPart->m_pDamageBehaviour->HandleImpactDamage(impactSize);
//		UpdateDamageBehaviour(pHitPart);
//	}
//
//	for (U32 iPart = 0; iPart < pHitPart->m_Damage.m_Transfer.Size(); iPart++)
//	{
//		C_Part* pTransfer = pHitPart->m_Damage.m_Transfer[iPart].m_pNode;
//		if (!pTransfer)
//			continue;
//		float tis = pTransfer->m_Damage.m_Behaviour.m_ImpactScale == 0.0f ? 1.0f : pTransfer->m_Damage.m_Behaviour.m_ImpactScale;
//		float newDamageAmount = Maths::Clamp(damageAmount * tis * pHitPart->m_Damage.m_Transfer[iPart].m_Percentage, 0.0f, 1.0f);
//		newDamageLevel = Maths::Clamp(pTransfer->m_Damage.m_DamageLevel + newDamageAmount, 0.0f, 1.0f);
//	#ifndef VEHICLE_MASTER_BUILD
//		LOGDEBUG(g_DamageLog, "Transfer %.3f damage to part '%s' - old damage = %.3f, new damage = %.3f", 
//								pTransfer->debug_Name.Chars(), pTransfer->m_Damage.m_DamageLevel, newDamageLevel);
//	#endif
//		pTransfer->SetImpactSize(impactSize * pHitPart->m_Damage.m_Transfer[iPart].m_Percentage);
//
//		if (m_Transitioning)
//			pTransfer->SetTargetDamageLevel(newDamageLevel);			
//		else
//		{
//			pTransfer->HandleDamage(newDamageLevel);
//			if (pTransfer->m_pDamageBehaviour)
//				pTransfer->m_pDamageBehaviour->HandleImpactDamage(tis);
//			UpdateDamageBehaviour(pTransfer);
//		}		
//	}
//
//	m_FatalDamage = false;
//}

Debug::Float g_debugMaxCollisionImpulse("Vehicle.Damage.MaxCollisionImpulse", 30.0f, Debug::Range(0.1f, 500.0f), Debug::Step(0.001f));
Debug::Float g_debugMinImpactSize("Vehicle.Damage.MinImpactSize", 0.001f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_debugDamageNodeClampToEdge("Vehicle.Damage.ClampToEdge", false);
Debug::Int g_debugDamageDebugColourToAdd("Vehicle.Damage.DebugColourToAdd", 32, Debug::Range(0, 255), Debug::Step(1));

Debug::Float g_debugCollisionEllipseMaxWidth("Vehicle.Damage.Impact.DeformEllipseWidth", 1.5f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));
Debug::Float g_debugCollisionEllipseMaxDepth("Vehicle.Damage.Impact.DeformEllipseDepth", 1.5f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));

Debug::Bool g_debugTreatContactPointsSeparately("Vehicle.Damage.TreatEachContactPointAsSeparate", true);

Debug::Float g_debugMinImpactSizeForPartDamage("Vehicle.Damage.MinImpactSizeForPartDamage", 0.1f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugMinImpactSizeForDeformation("Vehicle.Damage.MinImpactSizeForDeformation", 0.025f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugMinImpactSizeForScraping("Vehicle.Damage.MinImpactSizeForScraping", 0.001f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Int g_debugMaxContactPoints("Vehicle.Damage.MaxContactPoints", 4, Debug::Range(0, 10), Debug::Step(1));

/*
===============
NAME: ProcessCollision

NOTE: Processes collisions from the vehicle actor
===============
*/
C_ImpactEvent* C_DamageLayer::ProcessCollision(Maths::HMat43 p_Matrix, const CollisionInfo& p_CollisionInfo, const C_DamageFilter& p_Filter)
{
	C_ImpactEvent* pReturnEvent = NULL;
	C_ImpactEvent impactEvent;
	//impactEvent.m_ImpactSize		= p_CollisionInfo.m_GameplayImpulse;//Maths::Clamp(p_CollisionInfo.m_MaxImpulse / (float)g_debugMaxCollisionImpulse, 0.0f, 1.0f);
	impactEvent.m_ImpactSize		= p_CollisionInfo.m_DamageImpulse;//Maths::Clamp(p_CollisionInfo.m_MaxImpulse / (float)g_debugMaxCollisionImpulse, 0.0f, 1.0f);
	if (p_Filter.m_FatalDamage)
		SetFatalDamageFlag(p_Filter.m_FatalDamage);
	if (impactEvent.m_ImpactSize >= (float)g_debugMinImpactSize)
	{
		impactEvent.m_CollisionInfo		= p_CollisionInfo;
		impactEvent.m_Filter			= p_Filter;
		impactEvent.m_Matrix			= p_Matrix;
		if (p_CollisionInfo.m_ImpactRadiusOverride > 0.0f)
			impactEvent.m_ImpactRadius = p_CollisionInfo.m_ImpactRadiusOverride;
		else
			impactEvent.m_ImpactRadius		= impactEvent.m_ImpactSize * Maths::Max((float)g_debugCollisionEllipseMaxWidth, (float)g_debugCollisionEllipseMaxDepth);
		impactEvent.m_bProcessParts = false;
		impactEvent.m_bDeform = false;
		impactEvent.m_bScrape = false;
		if (g_debugTreatContactPointsSeparately)
		{
			for (int iPoint = 0; iPoint < p_CollisionInfo.m_ContactPoints.Size(); iPoint++)
			{
				impactEvent.ConstructCollisionPoint(p_CollisionInfo.m_ContactPoints[iPoint].m_ContactPoint, p_CollisionInfo.m_ContactPoints[iPoint].m_Friction, p_CollisionInfo.m_ContactPoints[iPoint].m_PenetrationDepth);				
				if (impactEvent.m_ImpactSize >= (float)g_debugMinImpactSizeForPartDamage)
					impactEvent.m_bProcessParts = true;
				if (impactEvent.m_ImpactSize >= (float)g_debugMinImpactSizeForDeformation)
					impactEvent.m_bDeform = true;
				if (p_CollisionInfo.m_PrimaryVehicle && impactEvent.m_ImpactSize >= (float)g_debugMinImpactSizeForScraping)
					impactEvent.m_bScrape = true;
				//FAIL!!!!!!!!!!!
				//if (impactEvent.m_bDeform && impactEvent.m_bScrape)
				//{
				//	AddImpactEvent(impactEvent);
				//	if (!pReturnEvent)
				//		pReturnEvent = &m_ImpactEvents.Back();
				//}
				AddImpactEvent(impactEvent);
				return &m_ImpactEvents.Back();
			}		
		}
		else
		{
			impactEvent.ConstructCollisionPlane(Maths::HVec3(m_VehicleInfo.m_Extents.X(), m_VehicleInfo.m_Extents.Y(), m_VehicleInfo.m_Extents.Z()));
			AddImpactEvent(impactEvent);
			return &m_ImpactEvents.Back();
		}
	}

	return pReturnEvent;
}

Debug::Int g_debugMaxCachedEvents("Vehicle.Damage.MaxCachedEvents", 10, Debug::Range(0, 50), Debug::Step(1));
Debug::Int g_debugMaxImpactEvents("Vehicle.Damage.MaxImpactEvents", 10, Debug::Range(0, 250), Debug::Step(1));

/*
===============
NAME: AddImpactEvent

NOTE: Adds an impact event to the cache
===============
*/
void C_DamageLayer::AddImpactEvent(C_ImpactEvent p_ImpactEvent)
{
	if (g_bDisableSPUDamage)
		return;

	if (m_ImpactEvents.Size() < g_debugMaxImpactEvents)
		m_ImpactEvents.PushBack(p_ImpactEvent);
//	else
//		LOGINFO(g_DamageLog, "Too many vehicle impact events to process!");

#ifndef CFG_GOLD
	if (m_CachedImpactEvents.Size() > (int)g_debugMaxCachedEvents)
		m_CachedImpactEvents.Clear();
	m_CachedImpactEvents.PushBack(p_ImpactEvent);
#endif
}

Debug::Bool g_debugDamageYPosHack("Vehicle.Damage.Debug.ImpactPosYHack", false);

Debug::Float g_debugArtDamageRadiusScale("Vehicle.Damage.Debug.ArtDeformRadiusScale", 1.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugArtScratchDamageRadiusScale("Vehicle.Damage.Debug.ArtScratchRadiusScale", 1.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugArtScratchScale("Vehicle.Damage.Debug.ArtScratchScale", 10.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugArtDeformScale("Vehicle.Damage.Debug.ArtDeformScale", 10.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));

//Debug::Float g_debugMeshDeformScale("Vehicle.Damage.Debug.CodeScale", 0.5f, Debug::Range(0.0f, 2.0f), Debug::Step(0.001f));
Debug::Float g_debugMeshDeformScale("Vehicle.Damage.Debug.CodeScale", 0.75f, Debug::Range(0.0f, 2.0f), Debug::Step(0.001f));

Debug::Float g_debugDeformMaxXDisplacement("Vehicle.Damage.MaxXDisplacement", 0.15f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));
Debug::Float g_debugDeformMaxYDisplacement0("Vehicle.Damage.MaxYDisplacement0", 0.0f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));
Debug::Float g_debugDeformMaxYDisplacement1("Vehicle.Damage.MaxYDisplacement1", 0.05f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));
Debug::Float g_debugDeformMaxYDisplacement2("Vehicle.Damage.MaxYDisplacement2", 0.25f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));
Debug::Float g_debugDeformMaxZDisplacement("Vehicle.Damage.MaxZDisplacement", 0.2f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));

Debug::Float g_debugZoneMinZ("Vehicle.Damage.NonDamageZoneMinZ", -1.75f, Debug::Range(0.0f, -2.5f), Debug::Step(0.001f));
//Debug::Float g_debugZoneMaxZ("Vehicle.Damage.NonDamageZoneMaxZ", 1.0f, Debug::Range(0.0f, 2.5f), Debug::Step(0.001f));
Debug::Float g_debugZoneMaxZ("Vehicle.Damage.NonDamageZoneMaxZ", 0.34f, Debug::Range(0.0f, 2.5f), Debug::Step(0.001f));

Debug::Float g_debugZoneMinX("Vehicle.Damage.NonDamageZoneMinX", -0.85f, Debug::Range(0.0f, -1.5f), Debug::Step(0.001f));
Debug::Float g_debugZoneMaxX("Vehicle.Damage.NonDamageZoneMaxX", 0.85f, Debug::Range(0.0f, 1.5f), Debug::Step(0.001f));

Debug::Float g_debugZoneMinY("Vehicle.Damage.NonDamageZoneMinY", -0.1f, Debug::Range(-1.0f, -1.5f), Debug::Step(0.001f));
Debug::Float g_debugZoneMaxY("Vehicle.Damage.NonDamageZoneMaxY", 0.5f, Debug::Range(0.0f, 1.5f), Debug::Step(0.001f));
Debug::Float g_debugMinImpactRadius("Vehicle.Damage.MinImpactRadius", 0.4f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugNodeOffsetBaseDamageLevel("Vehicle.Damage.Code.BaseDamageLevelMaxDisplacement", 0.05f, Debug::Range(0.01f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugWheelMovementToKnockOff("Vehicle.Damage.Code.Wheels.DisplacementToKnockOff", 0.15f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugWheelMovementToBurstTyre("Vehicle.Damage.Code.Wheels.DisplacementToBurstTyre", 0.075f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Int g_debugImpactStage("Vehicle.Damage.Code.Stages.Stage", 1, Debug::Range(0, 4), Debug::Step(1));
Debug::Float g_debugImpactStage1MaxMovement("Vehicle.Damage.Code.Stages.Stage1.MaxMovement", 0.05f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugImpactStage2MaxMovement("Vehicle.Damage.Code.Stages.Stage2.MaxMovement", 0.2f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugImpactStage3MaxMovement("Vehicle.Damage.Code.Stages.Stage3.MaxMovement", 0.4f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugImpactStage4MaxMovement("Vehicle.Damage.Code.Stages.Stage4.MaxMovement", 0.75f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugVehicleDamageDebugRenderYOffset("Vehicle.Damage.Code.RenderYOffset", 0.5f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: RenderDefomationInformation

NOTE: Renders deformation zone information
===============
*/
void C_DamageLayer::RenderDefomationInformation(Maths::HMat43 p_Matrix)
{
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	
	float y = (float)g_debugVehicleDamageDebugRenderYOffset;

	//CrumpleZone
	Maths::HVec3 vCentre(m_VehicleInfo.m_Centre.X(), m_VehicleInfo.m_Centre.Y(), m_VehicleInfo.m_Centre.Z());

	//Front
	Maths::HVec3 start = Maths::HVec3(vCentre + Maths::HVec3(-m_DeformSetup.m_SideCrumpleZone, y, m_DeformSetup.m_FrontCrumpleZone)) * p_Matrix;
	Maths::HVec3 end = Maths::HVec3(vCentre + Maths::HVec3(m_DeformSetup.m_SideCrumpleZone, y, m_DeformSetup.m_FrontCrumpleZone)) * p_Matrix;
	pDebugRenderer->Draw3dLine(start, end, Maths::RGBA(0, 255, 0, 255));

	//Back
	start = Maths::HVec3(vCentre + Maths::HVec3(-m_DeformSetup.m_SideCrumpleZone, y, m_DeformSetup.m_RearCrumpleZone)) * p_Matrix;
	end = Maths::HVec3(vCentre + Maths::HVec3(m_DeformSetup.m_SideCrumpleZone, y, m_DeformSetup.m_RearCrumpleZone)) * p_Matrix;
	pDebugRenderer->Draw3dLine(start, end, Maths::RGBA(0, 255, 0, 255));

	//Left
	start = Maths::HVec3(vCentre + Maths::HVec3(-m_DeformSetup.m_SideCrumpleZone, y, m_DeformSetup.m_RearCrumpleZone)) * p_Matrix;
	end = Maths::HVec3(vCentre + Maths::HVec3(-m_DeformSetup.m_SideCrumpleZone, y, m_DeformSetup.m_FrontCrumpleZone)) * p_Matrix;
	pDebugRenderer->Draw3dLine(start, end, Maths::RGBA(0, 255, 0, 255));

	//Right
	start = Maths::HVec3(vCentre + Maths::HVec3(m_DeformSetup.m_SideCrumpleZone, y, m_DeformSetup.m_RearCrumpleZone)) * p_Matrix;
	end = Maths::HVec3(vCentre + Maths::HVec3(m_DeformSetup.m_SideCrumpleZone, y, m_DeformSetup.m_FrontCrumpleZone)) * p_Matrix;
	pDebugRenderer->Draw3dLine(start, end, Maths::RGBA(0, 255, 0, 255));

	//NoDamageZone

	//Front
	start = Maths::HVec3(vCentre + Maths::HVec3(-m_DeformSetup.m_SideNoDamageZone, y, m_DeformSetup.m_FrontNoDamageZone)) * p_Matrix;
	end = Maths::HVec3(vCentre + Maths::HVec3(m_DeformSetup.m_SideNoDamageZone, y, m_DeformSetup.m_FrontNoDamageZone)) * p_Matrix;
	pDebugRenderer->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 255));

	//Back
	start = Maths::HVec3(vCentre + Maths::HVec3(-m_DeformSetup.m_SideNoDamageZone, y, m_DeformSetup.m_RearNoDamageZone)) * p_Matrix;
	end = Maths::HVec3(vCentre + Maths::HVec3(m_DeformSetup.m_SideNoDamageZone, y, m_DeformSetup.m_RearNoDamageZone)) * p_Matrix;
	pDebugRenderer->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 255));

	//Left
	start = Maths::HVec3(vCentre + Maths::HVec3(-m_DeformSetup.m_SideNoDamageZone, y, m_DeformSetup.m_RearNoDamageZone)) * p_Matrix;
	end = Maths::HVec3(vCentre + Maths::HVec3(-m_DeformSetup.m_SideNoDamageZone, y, m_DeformSetup.m_FrontNoDamageZone)) * p_Matrix;
	pDebugRenderer->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 255));

	//Right
	start = Maths::HVec3(vCentre + Maths::HVec3(m_DeformSetup.m_SideNoDamageZone, y, m_DeformSetup.m_RearNoDamageZone)) * p_Matrix;
	end = Maths::HVec3(vCentre + Maths::HVec3(m_DeformSetup.m_SideNoDamageZone, y, m_DeformSetup.m_FrontNoDamageZone)) * p_Matrix;
	pDebugRenderer->Draw3dLine(start, end, Maths::RGBA(255, 0, 0, 255));
}

/*
===============
NAME: SetupWheelDamage

NOTE: Setup wheel damage information
===============
*/
void C_DamageLayer::SetupWheelDamage(XtWheelDamageSetup* p_pSetup)
{
	if (!p_pSetup)
	{
		m_WheelSetup.m_FrontTyreLossRadiusScale		= 1.0f;
		m_WheelSetup.m_RearTyreLossRadiusScale		= 1.0f;
		m_WheelSetup.m_FrontWheelLossRadiusScale	= 1.0f;
		m_WheelSetup.m_RearWheelLossRadiusScale		= 1.0f;
		return;
	}
	
	m_WheelSetup.m_FrontTyreLossRadiusScale		= p_pSetup->m_FrontTyreLossRadiusScale	== 0.0f ? 1.0f : p_pSetup->m_FrontTyreLossRadiusScale;
	m_WheelSetup.m_RearTyreLossRadiusScale		= p_pSetup->m_RearTyreLossRadiusScale	== 0.0f ? 1.0f : p_pSetup->m_RearTyreLossRadiusScale;
	m_WheelSetup.m_FrontWheelLossRadiusScale	= p_pSetup->m_FrontWheelLossRadiusScale == 0.0f ? 1.0f : p_pSetup->m_FrontWheelLossRadiusScale;
	m_WheelSetup.m_RearWheelLossRadiusScale		= p_pSetup->m_RearWheelLossRadiusScale	== 0.0f ? 1.0f : p_pSetup->m_RearWheelLossRadiusScale;
}

/*
===============
NAME: SetupDeformRanges

NOTE: Setup node damage information
===============
*/
void C_DamageLayer::SetupDeformRanges(XtDeformationSetup* p_pSetup)
{
	if (!p_pSetup)
	{
		m_DeformSetup.m_Valid = false;
	}
	else
	{
		m_DeformSetup.m_FrontCrumpleZone	= p_pSetup->m_FrontCrumpleZone + p_pSetup->m_ZOffset;
		m_DeformSetup.m_RearCrumpleZone		= -p_pSetup->m_RearCrumpleZone + p_pSetup->m_ZOffset;
		m_DeformSetup.m_SideCrumpleZone		= p_pSetup->m_SideCrumpleZone;
		m_DeformSetup.m_FrontNoDamageZone	= p_pSetup->m_FrontNoDamageZone + p_pSetup->m_ZOffset;
		m_DeformSetup.m_RearNoDamageZone	= -p_pSetup->m_RearNoDamageZone + p_pSetup->m_ZOffset;
		m_DeformSetup.m_SideNoDamageZone	= p_pSetup->m_SideNoDamageZone;
		m_DeformSetup.m_FrontYMovement		= p_pSetup->m_FrontYMovement;
		m_DeformSetup.m_RearYMovement		= p_pSetup->m_RearYMovement;
		m_DeformSetup.m_SideYMovement		= p_pSetup->m_SideYMovement;
		m_DeformSetup.m_Valid				= true;
	}	

	for (S32 iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
	{
		for (S32 iColumn = 0; iColumn < (*m_pDamageVolume)[iLayer].GetWidth(); iColumn++)
		{
			for (S32 iRow = 0; iRow < (*m_pDamageVolume)[iLayer].GetLength(); iRow++)
			{
				C_Node* pNode = (*m_pDamageVolume)[iLayer].GetNode(iRow, iColumn);
				SetupDeformRange(pNode, iLayer);
			}
		}
	}
}

Debug::Bool g_debugVehicleDamageUseSorceryBasedControl("Vehicle.Damage.Code.UseSorceryBasedControl", true);
Debug::Float g_debugVehicleDrivableCrumpleZoneScale("Vehicle.Damage.Code.DrivableCrumpleZoneScale", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: SetupDeformRange

NOTE: Setup node damage information
===============
*/
void C_DamageLayer::SetupDeformRange(C_Node* p_pNode, S32 p_Layer)
{
	if (!p_pNode)
		return;

	if (m_DeformSetup.m_Valid == false)
	{
		p_pNode->m_DeformRange.m_MinX = p_pNode->m_OriginalPosition.X();
		p_pNode->m_DeformRange.m_MaxX = p_pNode->m_OriginalPosition.X();
		p_pNode->m_DeformRange.m_MinY = p_pNode->m_OriginalPosition.Y();
		p_pNode->m_DeformRange.m_MaxY = p_pNode->m_OriginalPosition.Y();
		p_pNode->m_DeformRange.m_MinZ = p_pNode->m_OriginalPosition.Z();
		p_pNode->m_DeformRange.m_MaxZ = p_pNode->m_OriginalPosition.Z();

		p_pNode->m_DriveableDeformRange.m_MinX = p_pNode->m_OriginalPosition.X();
		p_pNode->m_DriveableDeformRange.m_MaxX = p_pNode->m_OriginalPosition.X();
		p_pNode->m_DriveableDeformRange.m_MinY = p_pNode->m_OriginalPosition.Y();
		p_pNode->m_DriveableDeformRange.m_MaxY = p_pNode->m_OriginalPosition.Y();
		p_pNode->m_DriveableDeformRange.m_MinZ = p_pNode->m_OriginalPosition.Z();
		p_pNode->m_DriveableDeformRange.m_MaxZ = p_pNode->m_OriginalPosition.Z();
		
		return;
	}

	float fLayer = (float)p_Layer / 2.0f;
	
	//Setup Z and Y movement
	if (p_pNode->m_OriginalPosition.Z() >= m_DeformSetup.m_FrontNoDamageZone)
	{
		float fRange	= m_VehicleInfo.m_Extents.Z() - m_DeformSetup.m_FrontNoDamageZone;
		float fScale	= 1.0f - Maths::Clamp((float)(p_pNode->m_OriginalPosition.Z() - m_DeformSetup.m_FrontNoDamageZone) / fRange, 0.0f, 1.0f);
		fRange = Maths::Lerp(m_DeformSetup.m_FrontCrumpleZone, m_DeformSetup.m_FrontNoDamageZone, fScale);
		p_pNode->m_DeformRange.m_MinZ = Maths::Min(p_pNode->m_OriginalPosition.Z(), fRange);
		p_pNode->m_DeformRange.m_MaxZ = Maths::Max(p_pNode->m_OriginalPosition.Z(), fRange);
		p_pNode->m_DeformRange.m_MinY = p_pNode->m_OriginalPosition.Y() - (fLayer * m_DeformSetup.m_FrontYMovement);
		p_pNode->m_DeformRange.m_MaxY = p_pNode->m_OriginalPosition.Y() + (fLayer * m_DeformSetup.m_FrontYMovement);

		float fDrivableCrumple = m_DeformSetup.m_FrontCrumpleZone + (m_VehicleInfo.m_Extents.Z() - m_DeformSetup.m_FrontCrumpleZone) * (float)g_debugVehicleDrivableCrumpleZoneScale;
		fRange = Maths::Lerp(fDrivableCrumple, m_DeformSetup.m_FrontNoDamageZone, fScale);
		p_pNode->m_DriveableDeformRange.m_MinZ = Maths::Min(p_pNode->m_OriginalPosition.Z(), fRange);
		p_pNode->m_DriveableDeformRange.m_MaxZ = Maths::Max(p_pNode->m_OriginalPosition.Z(), fRange);
		p_pNode->m_DriveableDeformRange.m_MinY = p_pNode->m_OriginalPosition.Y() - (fLayer * m_DeformSetup.m_FrontYMovement);
		p_pNode->m_DriveableDeformRange.m_MaxY = p_pNode->m_OriginalPosition.Y() + (fLayer * m_DeformSetup.m_FrontYMovement);
	}
	else if (p_pNode->m_OriginalPosition.Z() <= m_DeformSetup.m_RearNoDamageZone)
	{
		float fRange	= m_VehicleInfo.m_Extents.Z() - Maths::Abs(m_DeformSetup.m_RearNoDamageZone);
		float fScale	= 1.0f - Maths::Clamp((float)(Maths::Abs(p_pNode->m_OriginalPosition.Z()) - Maths::Abs(m_DeformSetup.m_RearNoDamageZone)) / fRange, 0.0f, 1.0f);
		fRange = Maths::Lerp(m_DeformSetup.m_RearCrumpleZone, m_DeformSetup.m_RearNoDamageZone, fScale);
		p_pNode->m_DeformRange.m_MinZ = Maths::Min(p_pNode->m_OriginalPosition.Z(), fRange);
		p_pNode->m_DeformRange.m_MaxZ = Maths::Max(p_pNode->m_OriginalPosition.Z(), fRange);
		p_pNode->m_DeformRange.m_MinY = p_pNode->m_OriginalPosition.Y() - (fLayer * m_DeformSetup.m_RearYMovement);
		p_pNode->m_DeformRange.m_MaxY = p_pNode->m_OriginalPosition.Y() + (fLayer * m_DeformSetup.m_RearYMovement);

		float fDrivableCrumple = m_DeformSetup.m_RearCrumpleZone - (m_VehicleInfo.m_Extents.Z() - Maths::Abs(m_DeformSetup.m_RearCrumpleZone)) * (float)g_debugVehicleDrivableCrumpleZoneScale;
		fRange = Maths::Lerp(fDrivableCrumple, m_DeformSetup.m_RearNoDamageZone, fScale);
		p_pNode->m_DriveableDeformRange.m_MinZ = Maths::Min(p_pNode->m_OriginalPosition.Z(), fRange);
		p_pNode->m_DriveableDeformRange.m_MaxZ = Maths::Max(p_pNode->m_OriginalPosition.Z(), fRange);
		p_pNode->m_DriveableDeformRange.m_MinY = p_pNode->m_OriginalPosition.Y() - (fLayer * m_DeformSetup.m_RearYMovement);
		p_pNode->m_DriveableDeformRange.m_MaxY = p_pNode->m_OriginalPosition.Y() + (fLayer * m_DeformSetup.m_RearYMovement);
	}	
	else 
	{		
		p_pNode->m_DeformRange.m_MinZ = p_pNode->m_OriginalPosition.Z();
		p_pNode->m_DeformRange.m_MaxZ = p_pNode->m_OriginalPosition.Z();
		p_pNode->m_DeformRange.m_MinY = p_pNode->m_OriginalPosition.Y() - (fLayer * m_DeformSetup.m_SideYMovement);
		p_pNode->m_DeformRange.m_MaxY = p_pNode->m_OriginalPosition.Y() + (fLayer * m_DeformSetup.m_SideYMovement);

		p_pNode->m_DriveableDeformRange.m_MinZ = p_pNode->m_OriginalPosition.Z();
		p_pNode->m_DriveableDeformRange.m_MaxZ = p_pNode->m_OriginalPosition.Z();
		p_pNode->m_DriveableDeformRange.m_MinY = p_pNode->m_OriginalPosition.Y() - (fLayer * m_DeformSetup.m_SideYMovement);
		p_pNode->m_DriveableDeformRange.m_MaxY = p_pNode->m_OriginalPosition.Y() + (fLayer * m_DeformSetup.m_SideYMovement);
	}

	//Setup X movement
	if (p_pNode->m_OriginalPosition.X() >= m_DeformSetup.m_SideCrumpleZone)
	{
		float fRange	= m_VehicleInfo.m_Extents.X() - m_DeformSetup.m_SideNoDamageZone;
		float fScale	= 1.0f - Maths::Clamp((float)(p_pNode->m_OriginalPosition.X() - m_DeformSetup.m_SideNoDamageZone) / fRange, 0.0f, 1.0f);
		fRange = Maths::Lerp(m_DeformSetup.m_SideCrumpleZone, m_DeformSetup.m_SideNoDamageZone, fScale);
		p_pNode->m_DeformRange.m_MinX = Maths::Min(p_pNode->m_OriginalPosition.X(), fRange);
		p_pNode->m_DeformRange.m_MaxX = Maths::Max(p_pNode->m_OriginalPosition.X(), fRange);

		float fDrivableCrumple = m_DeformSetup.m_SideCrumpleZone + (m_VehicleInfo.m_Extents.X() - m_DeformSetup.m_SideCrumpleZone) * (float)g_debugVehicleDrivableCrumpleZoneScale;
		fRange = Maths::Lerp(fDrivableCrumple, m_DeformSetup.m_SideNoDamageZone, fScale);
		p_pNode->m_DriveableDeformRange.m_MinX = Maths::Min(p_pNode->m_OriginalPosition.X(), fRange);
		p_pNode->m_DriveableDeformRange.m_MaxX = Maths::Max(p_pNode->m_OriginalPosition.X(), fRange);		
	}
	else if (p_pNode->m_OriginalPosition.X() <= -m_DeformSetup.m_SideCrumpleZone)
	{
		float fRange	= m_VehicleInfo.m_Extents.X() - Maths::Abs(m_DeformSetup.m_SideNoDamageZone);
		float fScale	= 1.0f - Maths::Clamp((float)(Maths::Abs(p_pNode->m_OriginalPosition.X()) - Maths::Abs(m_DeformSetup.m_SideNoDamageZone)) / fRange, 0.0f, 1.0f);
		fRange = -Maths::Lerp(m_DeformSetup.m_SideCrumpleZone, m_DeformSetup.m_SideNoDamageZone, fScale);
		p_pNode->m_DeformRange.m_MinX = Maths::Min(p_pNode->m_OriginalPosition.X(), fRange);
		p_pNode->m_DeformRange.m_MaxX = Maths::Max(p_pNode->m_OriginalPosition.X(), fRange);

		float fDrivableCrumple = m_DeformSetup.m_SideCrumpleZone + (m_VehicleInfo.m_Extents.X() - Maths::Abs(m_DeformSetup.m_SideCrumpleZone)) * (float)g_debugVehicleDrivableCrumpleZoneScale;
		fRange = -Maths::Lerp(fDrivableCrumple, m_DeformSetup.m_SideNoDamageZone, fScale);
		p_pNode->m_DriveableDeformRange.m_MinX = Maths::Min(p_pNode->m_OriginalPosition.X(), fRange);
		p_pNode->m_DriveableDeformRange.m_MaxX = Maths::Max(p_pNode->m_OriginalPosition.X(), fRange);
	}
	else
	{
		p_pNode->m_DeformRange.m_MinX = p_pNode->m_OriginalPosition.X();
		p_pNode->m_DeformRange.m_MaxX = p_pNode->m_OriginalPosition.X();
		p_pNode->m_DriveableDeformRange.m_MinX = p_pNode->m_OriginalPosition.X();
		p_pNode->m_DriveableDeformRange.m_MaxX = p_pNode->m_OriginalPosition.X();
	}
}

/*
===============
NAME: ClampNodePosition

NOTE: Clamps node to min/max allowable position
===============
*/
void C_DamageLayer::ClampNodePosition(C_Node* p_pNode, Maths::HVec3 p_Position)
{	
	MathsStorage::Float3 pos;

	if (m_SlowMotionDamage || m_FatalDamage)
	{
		pos.x = Maths::Clamp((float)p_Position.X(), p_pNode->m_DeformRange.m_MinX, p_pNode->m_DeformRange.m_MaxX);
		pos.y = Maths::Clamp((float)p_Position.Y(), p_pNode->m_DeformRange.m_MinY, p_pNode->m_DeformRange.m_MaxY);
		pos.z = Maths::Clamp((float)p_Position.Z(), p_pNode->m_DeformRange.m_MinZ, p_pNode->m_DeformRange.m_MaxZ);
	}
	else
	{
		pos.x = Maths::Clamp((float)p_Position.X(), p_pNode->m_DriveableDeformRange.m_MinX, p_pNode->m_DriveableDeformRange.m_MaxX);
		pos.y = Maths::Clamp((float)p_Position.Y(), p_pNode->m_DriveableDeformRange.m_MinY, p_pNode->m_DriveableDeformRange.m_MaxY);
		pos.z = Maths::Clamp((float)p_Position.Z(), p_pNode->m_DriveableDeformRange.m_MinZ, p_pNode->m_DriveableDeformRange.m_MaxZ);		
	}

	p_pNode->m_Position = Maths::Load<Maths::HVec3, MathsStorage::Float3>(&pos);
}

/*
===============
NAME: CalculateNodeExtents

NOTE: Calculates how far node can move
===============
*/
void C_DamageLayer::CalculateNodeExtents(S32 p_Layer, C_Node* p_pNode, float& p_X, float& p_Y, float& p_Z, int p_Stage, Maths::HVec3 p_Position)
{	
	if (p_Layer == 0)
		p_Y = (float)g_debugDeformMaxYDisplacement0;
	else if (p_Layer == 1)
		p_Y = (float)g_debugDeformMaxYDisplacement1;
	else if (p_Layer == 2)
		p_Y = (float)g_debugDeformMaxYDisplacement2;

	{
		float minZ = (float)g_debugZoneMinZ;
		float maxZ = (float)g_debugZoneMaxZ;
		float rangeZ = 1.0f;
		float posZ = 0.0f;
		float zScale = 0.0f;
		if (p_pNode->m_OriginalPosition.Z() > maxZ)
		{
			rangeZ = m_VehicleInfo.m_Extents.Z() - maxZ;
			posZ = p_pNode->m_OriginalPosition.Z() - maxZ;
		}
		else 
		{
			rangeZ = m_VehicleInfo.m_Extents.Z() - Maths::Abs(minZ);
			posZ = Maths::Abs(p_pNode->m_OriginalPosition.Z()) - Maths::Abs(minZ);
		}

		if (rangeZ > 0.0f)
			zScale = Maths::Clamp(posZ / rangeZ, 0.0f, 1.0f);
			
		switch (p_Stage)
		{
		case 1:
			rangeZ = (float)g_debugImpactStage1MaxMovement;
			break;
		case 2:
			rangeZ = (float)g_debugImpactStage2MaxMovement;
			break;
		case 3:
			rangeZ = (float)g_debugImpactStage3MaxMovement;
			break;
		case 4:
			rangeZ = (float)g_debugImpactStage4MaxMovement;
			break;
		}

		p_Z = Maths::Lerp(0.0f, rangeZ, zScale);
	}

	{
		float minX = (float)g_debugZoneMinX;
		float maxX = (float)g_debugZoneMaxX;
		float rangeX = 1.0f;
		float posX = 0.0f;
		float xScale = 0.0f;
		if (p_pNode->m_OriginalPosition.X() > maxX)
		{
			rangeX = m_VehicleInfo.m_Extents.X() - maxX;
			posX = p_pNode->m_OriginalPosition.X() - maxX;
		}
		else 
		{
			rangeX = m_VehicleInfo.m_Extents.X() - Maths::Abs(minX);
			posX = Maths::Abs(p_pNode->m_OriginalPosition.X()) - Maths::Abs(minX);
		}

		if (rangeX > 0.0f)
			xScale = Maths::Clamp(posX / rangeX, 0.0f, 1.0f);
			
		p_X = Maths::Lerp(0.0f, rangeX, xScale);
	}
}

Debug::Float g_debugArtDeformMinAmount("Vehicle.Damage.Code.Art.MinDeformAmount", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugScratchMinAmount("Vehicle.Damage.Code.Art.MinScratchAmount", 2.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_debugKeepDamageStateUpToDate("Vehicle.Damage.Replay.KeepStateUpToDate", true);

/*
===============
NAME: Deform

NOTE: Deform the node lattice
===============
*/
void C_DamageLayer::Deform(C_ImpactEvent* p_pEvent, float p_Duration)
{
	PERF_EVENT("C_DamageLayer.Deform");

	if (m_Transitioning)
		return;

//	LOGINFO(g_DamageLog, "Impulse(%.3f), ImpactSize(%.3f)", p_pEvent->m_CollisionInfo.m_MaxImpulse, p_pEvent->m_ImpactSize);

	if (p_Duration > 0.0f)
	{
		m_Transitioning = true;
		m_SlowMotionDamage = true;
		m_TransitionPeriod = p_Duration;
		m_TransitionTime = 0.0f;
		m_CachedDamageEvent = *p_pEvent;
	}
	else
		m_SlowMotionDamage = false;

	float fRadius = (float)g_debugCollisionEllipseMaxDepth * p_pEvent->m_ImpactSize;
	fRadius = Maths::Max(fRadius, (float)g_debugMinImpactRadius);
	fRadius = fRadius * fRadius;
	Maths::HVec3 impactPosition = p_pEvent->m_LocalPosition;

	if (g_debugDamageYPosHack)
		impactPosition = Maths::HVec3(impactPosition.X(), 0.0f, impactPosition.Z());

	Maths::HVec3 vOffsetVec			= p_pEvent->m_LocalNormal * p_pEvent->m_ImpactSize * p_pEvent->m_Filter.m_CodeDeformAmount * (float)g_debugMeshDeformScale;

	float fArtDeformRadiusScale		= fRadius * (float)g_debugArtDamageRadiusScale;
	float fArtScratchRadiusScale	= fRadius * (float)g_debugArtScratchDamageRadiusScale;

	float fScratchAdditionScale		= p_pEvent->m_ImpactSize * p_pEvent->m_Filter.m_ArtDeformAmount * (float)g_debugArtScratchScale;
	float fDeformAdditionScale		= p_pEvent->m_ImpactSize * p_pEvent->m_Filter.m_ArtDeformAmount * (float)g_debugArtDeformScale;

	for (int iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
	{
		for (int iColumn = 0; iColumn < (*m_pDamageVolume)[iLayer].GetWidth(); iColumn++)
		{
			for (int iRow = 0; iRow < (*m_pDamageVolume)[iLayer].GetLength(); iRow++)
			{
				C_Node* pNode = (*m_pDamageVolume)[iLayer].GetNode(iRow, iColumn);
				
				float distToNode = (pNode->m_Position - impactPosition).Length2();
				float radius = fRadius;

				pNode->m_CachedPosition	= pNode->m_Position;							
				pNode->m_TargetPosition = pNode->m_Position;		
				
				if (distToNode < radius)
				{
					float distScale = 1.0f - Maths::Clamp(distToNode / radius, 0.0f, 1.0f);
					//Maths::HVec3 newPosition = pNode->m_Position - p_pEvent->m_LocalNormal * p_pEvent->m_ImpactSize * distScale * p_pEvent->m_Filter.m_CodeDeformAmount * (float)g_debugMeshDeformScale;
					Maths::HVec3 newPosition = pNode->m_Position -  vOffsetVec * distScale;
					SetUpdateVolumeTexture(true);
					SetUpdateInfoTexture(true);
			
					if (!m_Transitioning)
					{
						ClampNodePosition(pNode, newPosition);
						pNode->m_OffsetVector = pNode->m_Position - pNode->m_OriginalPosition;
						pNode->m_OffsetLength = pNode->m_OffsetVector.Length();	
					}
					else
					{
						pNode->m_TargetPosition = newPosition;
					}
				}

				float scratchAmount = 0.0f;
				float deformAmount = 0.0f;
				float baseLevel = Maths::Clamp(pNode->m_OffsetLength / (float)g_debugNodeOffsetBaseDamageLevel, 0.0f, 1.0f);
				if (p_pEvent->m_Filter.m_ScratchDamage)
				{					
					//float scratchAddition = Maths::Max((float)g_debugScratchMinAmount * scratchDistScale, p_pEvent->m_ImpactSize * scratchDistScale * p_pEvent->m_Filter.m_ArtDeformAmount * (float)g_debugArtDamageScale);
					float scratchDistScale = 1.0f - Maths::Clamp(distToNode / fArtScratchRadiusScale, 0.0f, 1.0f);
					float scratchAddition = Maths::Max((float)g_debugScratchMinAmount * scratchDistScale, fScratchAdditionScale * scratchDistScale);
					scratchAmount = baseLevel + scratchAddition;
				}
				if (p_pEvent->m_Filter.m_DeformParts)
				{					
					//float deformAddition = Maths::Max((float)g_debugArtDeformMinAmount * distScale, p_pEvent->m_ImpactSize * distScale * p_pEvent->m_Filter.m_ArtDeformAmount * (float)g_debugArtDamageScale);
					float distScale = 1.0f - Maths::Clamp(distToNode / fArtDeformRadiusScale, 0.0f, 1.0f);
					float deformAddition = Maths::Max((float)g_debugArtDeformMinAmount * distScale, fDeformAdditionScale * distScale);
					deformAmount = baseLevel + deformAddition;
				}				
						
				if (m_Transitioning)
				{
					pNode->m_CachedDeformLevel	= pNode->m_DeformLevel;					
					if (deformAmount > 0.0f)
						pNode->m_TargetDeformLevel = Maths::Clamp(pNode->m_DeformLevel + deformAmount, 0.0f, 1.0f);

					pNode->m_CachedScratchLevel	= pNode->m_ScratchLevel;
					if (scratchAmount > 0.0f)
						pNode->m_TargetScratchLevel = Maths::Clamp(pNode->m_ScratchLevel + scratchAmount, 0.0f, 1.0f);
				}
				else
				{
					if (deformAmount > 0.0f)
						pNode->m_DeformLevel = Maths::Clamp(pNode->m_DeformLevel + deformAmount, 0.0f, 1.0f);
					if (scratchAmount > 0.0f)
						pNode->m_ScratchLevel = Maths::Clamp(pNode->m_ScratchLevel + scratchAmount, 0.0f, 1.0f);
				}
				
				if (iLayer == 1 && g_debugKeepDamageStateUpToDate)
				{
					S32 index = iRow * (*m_pDamageVolume)[1].GetWidth() + iColumn;
					m_DamageState.m_State[index].m_DX = (DWORD)(Maths::Clamp((2.0f + (float)pNode->m_OffsetVector.X()) / 4.0f, 0.0f, 1.0f) * 32.0f);
					m_DamageState.m_State[index].m_DZ = (DWORD)(Maths::Clamp((2.0f + (float)pNode->m_OffsetVector.Z()) / 4.0f, 0.0f, 1.0f) * 64.0f);
					m_DamageState.m_State[index].m_Scratch	= (DWORD)(pNode->m_ScratchLevel * 7.0f);
					m_DamageState.m_State[index].m_Deform	= (DWORD)(pNode->m_DeformLevel * 7.0f);
				}
			}
		}
	}

	for (S32 iVertex = 0; iVertex < m_CollisionShapeVertices.Size(); iVertex++)
		m_CollisionShapeVertices[iVertex].UpdateVertex();

	//Now, sort out the wheels, as needed
	//UpdateWheelDamage(p_pEvent);
}

Debug::Float g_debugScrapeScratchAmount("Vehicle.Damage.Code.Scratch.Amount", 0.05f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugVehicleScratchDotThreshold("Vehicle.Damage.Code.Scratch.DotThreshold", 0.45f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Int g_debugScratchNodeBleed("Vehicle.Damage.Code.Scratch.Bleed", 0, Debug::Range(0, 7), Debug::Step(1));
Debug::Float g_DebugVelocityScrapeFactor("Vehicle.Damage.Code.Scratch.VelocityFactor", 0.025f);
Debug::Float g_DebugVelocityScrapeScaleDirX("Vehicle.Damage.Code.Scratch.ScaleDirX", 0.25f);
/*
===============
NAME: Scratch

NOTE: Scratch the car
===============
*/
void C_DamageLayer::Scrape(C_ImpactEvent* p_pEvent)
{
	PERF_EVENT("C_DamageLayer.Scratch");

	Maths::HVec3 impactPosition = p_pEvent->m_LocalPosition;

	Maths::HVec3 vSide(1.0f, 0.0f, 0.0f);
	float fDot = Maths::Abs(vSide | p_pEvent->m_LocalFriction);
	if (fDot >= (float)g_debugVehicleScratchDotThreshold)
		return;

	float fScale = 1.0f - (fDot / (float)g_debugVehicleScratchDotThreshold);

	if (fScale <= 0.0f)
		return;

	//Select nodes
	S32 numW = (*m_pDamageVolume)[1].GetWidth();
	S32 numL = (*m_pDamageVolume)[1].GetLength();
	S32 iLayer = 1;
	S32 iRow = -1;
	S32 iRowMin = -1;
	S32 iRowMax = -1;
	S32 iCol = -1;
	S32 iCol2 = -1;
	//Left side
	C_Node* pLeftMinNode	= (*m_pDamageVolume)[iLayer].GetNode(0, 0);
	C_Node* pLeftMaxNode	= (*m_pDamageVolume)[iLayer].GetNode(numL-1, 0);
	C_Node* pRightMinNode	= (*m_pDamageVolume)[iLayer].GetNode(0, numW-1);
	C_Node* pRightMaxNode	= (*m_pDamageVolume)[iLayer].GetNode(numL-1, numW-1);

	C_Node* pFrontNode	= (*m_pDamageVolume)[iLayer].GetNode(numL-2, numW/2);
	C_Node* pBackNode	= (*m_pDamageVolume)[iLayer].GetNode(1, numW/2);

	//pNode->m_ScratchLevel

	float fBleedAmount = fScale * (float)g_debugScratchNodeBleed;
	float fRounded = Maths::RoundToIntNearest(fBleedAmount);
	S32 iBleedAmount = (S32)fRounded;

	Maths::HVec3 vDir;
	Maths::HVec3 vPointVelocity = p_pEvent->m_CollisionInfo.m_ContactPoints[0].m_PtVelA;
	E_DefferedEvent deferredEvent;

	if(p_pEvent->m_CollisionInfo.m_ContactPoints[0].m_PtVelB.Length() != Maths::Constants<Maths::HF32>::Zero())
	{
		// if this collision is between 2 dynamic bodies then get the average point velocity
		// then get the direction as the point velocity in local space of the event's matrix
		vPointVelocity += p_pEvent->m_CollisionInfo.m_ContactPoints[0].m_PtVelB;
		vPointVelocity = vPointVelocity * Maths::Constants<Maths::HVec3>::Half();
		vDir = -vPointVelocity;
		Maths::HMat33 rotation(p_pEvent->m_Matrix);
		vDir = vDir * rotation.InverseRotation();
		vDir.Normalise();
		deferredEvent = E_DefferedEvent_ScrapeDynamic;
	}
	else
	{
		// otherwise use the local friction vector
		vDir = -p_pEvent->m_LocalFriction;
		deferredEvent = E_DefferedEvent_ScrapeStatic;
	}

	bool bUseImpactPositionForSparks = false;

	if (pLeftMaxNode && impactPosition.X() <= pLeftMaxNode->m_OriginalPosition.X())
	{
		float zPos = Maths::Clamp((float)(impactPosition.Z() - pLeftMinNode->m_OriginalPosition.Z()) / (float)(pLeftMaxNode->m_OriginalPosition.Z() - pLeftMinNode->m_OriginalPosition.Z()), 0.0f, 1.0f);
		iRow = (S32)( zPos * ((float)numL - 1.0f) );
		if (iRow>0)
			iRowMin = Maths::Clamp(iRow - iBleedAmount, 0, iRow);
		else
			iRowMin = Maths::Clamp(iRow - iBleedAmount, iRow, 0);

		iRowMax = Maths::Clamp(iRow + iBleedAmount, iRow, numL-1);
		iCol = 0;
		iCol2 = 1;
		if(deferredEvent == E_DefferedEvent_ScrapeStatic)
			vDir.X(vDir.X() - g_DebugVelocityScrapeScaleDirX * fScale);
	}
	else if (pRightMaxNode && impactPosition.X() >= pRightMaxNode->m_OriginalPosition.X())
	{
		float zPos = Maths::Clamp((float)(impactPosition.Z() - pRightMinNode->m_OriginalPosition.Z()) / (float)(pRightMaxNode->m_OriginalPosition.Z() - pRightMinNode->m_OriginalPosition.Z()), 0.0f, 1.0f);
		iRow = (S32)(zPos * ((float)numL - 1.0f));

		if (iRow>0)
			iRowMin = Maths::Clamp(iRow - iBleedAmount, 0, iRow);
		else
			iRowMin = Maths::Clamp(iRow - iBleedAmount, iRow ,0);

		iRowMax = Maths::Clamp(iRow + iBleedAmount, iRow, numL-1);
		iCol = numW-1;
		iCol2 = numW-2;
		if(deferredEvent == E_DefferedEvent_ScrapeStatic)
			vDir.X(vDir.X() + g_DebugVelocityScrapeScaleDirX * fScale);
	}
	else if (pFrontNode && pBackNode && impactPosition.Z() <= pFrontNode->m_OriginalPosition.Z() && impactPosition.Z() >= pBackNode->m_OriginalPosition.Z())
	{
		float xPos = Maths::Clamp((float)(impactPosition.X() - pLeftMaxNode->m_OriginalPosition.X()) / (float)(pRightMaxNode->m_OriginalPosition.X() - pLeftMaxNode->m_OriginalPosition.X()), 0.0f, 1.0f);		
		float zPos = Maths::Clamp((float)(impactPosition.Z() - pBackNode->m_OriginalPosition.Z()) / (float)(pFrontNode->m_OriginalPosition.Z() - pBackNode->m_OriginalPosition.Z()), 0.0f, 1.0f);
				
		AddDefferedDamageEffect(deferredEvent, impactPosition, vDir, vPointVelocity, fScale * Maths::Clamp(F32(m_pVehiclePhysics->GetVelocity().Length() * g_DebugVelocityScrapeFactor), 0.f, 1.f), E_PartCode_None, E_PartSubCode_None);

		iRow = (S32)Maths::RoundToIntZero(zPos * (float)(numL-1));
		iCol = (S32)Maths::RoundToIntZero(xPos * (float)(numW-1));

		C_Node* pScratchNode	= (*m_pDamageVolume)[2].GetNode(iRow, iCol);
		if (pScratchNode)
			pScratchNode->m_ScratchLevel = Maths::Clamp(pScratchNode->m_ScratchLevel + fScale, 0.0f, 1.0f);		

		return;
	}
	
	if (iRow == -1 || iCol == -1 || iCol2 == -1)
		return;

	vDir.Normalise();

	float fScratchAmount = fScale * (float)g_debugScrapeScratchAmount;
	for (S32 iNode = iRowMin; iNode <= iRowMax; iNode++)
	{
		C_Node* pScratchNode	= (*m_pDamageVolume)[iLayer].GetNode(iNode, iCol);
		C_Node* pScratchNode2	= (*m_pDamageVolume)[iLayer].GetNode(iNode, iCol2);

		if (!pScratchNode || !pScratchNode2)
			continue;

		pScratchNode->m_ScratchLevel = Maths::Clamp(pScratchNode->m_ScratchLevel + fScale, 0.0f, 1.0f);
		pScratchNode2->m_ScratchLevel = Maths::Clamp(pScratchNode2->m_ScratchLevel + fScale, 0.0f, 1.0f);		

		AddDefferedDamageEffect(deferredEvent, impactPosition, vDir, vPointVelocity, fScale * Maths::Clamp(F32(m_pVehiclePhysics->GetVelocity().Length() * g_DebugVelocityScrapeFactor), 0.f, 1.f), E_PartCode_None, E_PartSubCode_None);
	}
}

Debug::Bool g_debugDisableWheelDamage("Vehicle.Damage.Code.Wheels.DisableDamage", false);
Debug::Float g_debugRatioForTyreBurst("Vehicle.Damage.Code.Wheels.SeparationRatioForTyreBurst", 0.8f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugRatioForWheelKnockOff("Vehicle.Damage.Code.Wheels.SeparationRatioForWheelKnockOff", 0.6f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Bool g_debugPretendTyreBurst("Vehicle.Damage.Code.Wheels.PretendBurstTyre", false);
Debug::Int g_debugTyreToBurst("Vehicle.Damage.Code.Wheels.TyreToBurst", 0, Debug::Range(0, 3), Debug::Step(1));

Debug::Bool g_debugRepairAlwaysRepairWheels("Vehicle.Damage.Repair.AlwaysRepairWheels", true);

/*
===============
NAME: UpdateWheelDamage

NOTE: Updates damaged state of wheels
===============
*/
void C_DamageLayer::UpdateWheelDamage(C_ImpactEvent* p_pEvent)
{
	if (!m_pVehiclePhysics || g_debugDisableWheelDamage || m_WheelNodes.Size() != 4)
		return;

	PERF_EVENT("C_DamageLayer.UpdateWheelDamage");

	bool forceWheelRepair = false;
	if (!p_pEvent && g_debugRepairAlwaysRepairWheels)
		forceWheelRepair = true;

	bool burstTyreState[2] = {false, false};
	bool repairTyreState[2] = {false, false};

	//for (int iWheel = 0; iWheel < m_WheelNodes.Size(); iWheel++)
	for (int iWheel = 0; iWheel < 2; iWheel++)
	{
		C_WheelDamage* pWheel = &m_WheelNodes[iWheel];
		if (!pWheel || !pWheel->m_pNode)
			continue;

		pWheel->m_OffsetVector = pWheel->m_pNode->m_Position - pWheel->m_pNode->m_OriginalPosition;			
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[iWheel].offset = RacingPhysics::Vector3R(	pWheel->m_OffsetVector.X(),
																											0.0f,//pWheel->m_OffsetVector.Y(),
																											pWheel->m_OffsetVector.Z());
		
		pWheel->m_OffsetLength = pWheel->m_OffsetVector.Length();
		float maxSep = (pWheel->m_pMaxNode->m_OriginalPosition - pWheel->m_pMinNode->m_OriginalPosition).Length();
		float sep = (pWheel->m_pMaxNode->m_Position - pWheel->m_pMinNode->m_Position).Length();
		float fRange = Maths::Clamp(sep / maxSep, 0.0f, 1.0f);

#ifndef CFG_GOLD	
		if (g_debugPretendTyreBurst && iWheel == g_debugTyreToBurst)
		{
			C_Part* pNode = &m_pPartList->m_Parts[E_PartCode_Tyre].m_Nodes[(E_PartSubCode)(pWheel->m_Index+1)];
			AddDefferedDamageEffect(E_DefferedEvent_BurstTyre, pNode->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), 1.0f, pNode->m_PartCode, pNode->m_PartSubCode);
		}
		else if (g_debugActuallyBurstTyre && iWheel == g_debugTyreToBurst)
		{
			pWheel->m_HackTyreBurst = true;
			if (BurstTyre(pWheel))
				burstTyreState[iWheel] = true;
			g_debugActuallyBurstTyre = false;
		}
#endif
		if (m_CanBurstTyres && !pWheel->m_TyreBurst &&	!forceWheelRepair && (pWheel->m_OffsetLength >= (float)g_debugWheelMovementToBurstTyre || fRange < (float)g_debugRatioForTyreBurst))
		{
			if (!p_pEvent || (p_pEvent && p_pEvent->m_Filter.m_BurstTyres))
			{
				if (BurstTyre(pWheel))
					burstTyreState[iWheel] = true;
			}
		}
		else if (m_Transitioning && pWheel->m_TyreBurst && ((pWheel->m_OffsetLength < (float)g_debugWheelMovementToBurstTyre && fRange >= (float)g_debugRatioForTyreBurst) || forceWheelRepair))
		{
			if (RepairTyre(pWheel))
				repairTyreState[iWheel] = true;
		}

#ifndef CFG_GOLD
		if (g_debugActuallyLoseWheel && iWheel == g_debugTyreToBurst)
		{
			pWheel->m_HackWheelLoss = true;
			DetachWheel(pWheel);
			g_debugActuallyLoseWheel = false;
		}
#endif
		if (m_CanLoseWheels && !pWheel->m_Detached && !forceWheelRepair && (pWheel->m_OffsetLength >= (float)g_debugWheelMovementToKnockOff || fRange < (float)g_debugRatioForWheelKnockOff))
		{
			if (p_pEvent && p_pEvent->m_Filter.m_DetachWheels)
				DetachWheel(pWheel);
			else if (!p_pEvent)
				DetachWheel(pWheel);
		}
		else if (pWheel->m_Detached && ((pWheel->m_OffsetLength < (float)g_debugWheelMovementToKnockOff && fRange >= (float)g_debugRatioForWheelKnockOff) || forceWheelRepair))
			ReattachWheel(pWheel);
	}

	if (burstTyreState[0])
		BurstTyre(&m_WheelNodes[1]);
	if (burstTyreState[1])
		BurstTyre(&m_WheelNodes[0]);
	
	if (repairTyreState[0])
		RepairTyre(&m_WheelNodes[1]);
	if (repairTyreState[1])
		RepairTyre(&m_WheelNodes[0]);
}

/*
===============
NAME: BurstTyre

NOTE: Bursts a tyre
===============
*/
bool C_DamageLayer::BurstTyre(C_WheelDamage* p_pWheel)
{
	if (p_pWheel->m_Detached || p_pWheel->m_TyreBurst)
		return false;

	p_pWheel->m_TyreBurst = true;
	C_Part* pNode = &m_pPartList->m_Parts[E_PartCode_Tyre].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)];
	pNode->SetPermanentVisibility(0);
	if (p_pWheel->m_Index == 0 || p_pWheel->m_Index == 1)
	{
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].radiusScale = m_WheelSetup.m_FrontTyreLossRadiusScale;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].gripScale = p_pWheel->m_Grip * (float)g_debugTyreLossGripScale;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].rollingFriction = p_pWheel->m_Friction + (float)g_debugTyreLossFrictionScale;
	}
	else
	{
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].radiusScale = m_WheelSetup.m_RearTyreLossRadiusScale;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].gripScale = p_pWheel->m_Grip * (float)g_debugTyreLossGripScale;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].rollingFriction = p_pWheel->m_Friction + (float)g_debugTyreLossFrictionScale;
	}

	AddDefferedDamageEffect(E_DefferedEvent_BurstTyre, pNode->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), 1.0f, pNode->m_PartCode, pNode->m_PartSubCode);

	return true;
}

/*
===============
NAME: BurstTyre

NOTE: Bursts a tyre
===============
*/
bool C_DamageLayer::RepairTyre(C_WheelDamage* p_pWheel)
{
	if (p_pWheel->m_Detached || !p_pWheel->m_TyreBurst
#ifndef CFG_GOLD
		|| p_pWheel->m_HackTyreBurst
#endif
		)
		return false;

	p_pWheel->m_TyreBurst = false;
	m_pPartList->m_Parts[E_PartCode_Tyre].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(1);
	m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].radiusScale = 1.0f;
	m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].gripScale = p_pWheel->m_Grip;
	m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].rollingFriction = p_pWheel->m_Friction;

	return true;
}

/*
===============
NAME: DetachWheel

NOTE: Knocks wheel off
===============
*/
void C_DamageLayer::DetachWheel(C_WheelDamage* p_pWheel)
{
	if (p_pWheel->m_Detached)
		return;

	if (!p_pWheel->m_TyreBurst)
		BurstTyre(p_pWheel);
	p_pWheel->m_Detached = true;
	m_pPartList->m_Parts[E_PartCode_Wheel].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(0);
	m_pPartList->m_Parts[E_PartCode_Tyre].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(0);
	m_pPartList->m_Parts[E_PartCode_Alloy].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(0);
	m_pPartList->m_Parts[E_PartCode_AlloyRim].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(0);
	m_pPartList->m_Parts[E_PartCode_AlloyBlur].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(0);
	if (p_pWheel->m_Index == 0 || p_pWheel->m_Index == 1)
	{
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].radiusScale = m_WheelSetup.m_FrontWheelLossRadiusScale;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].gripScale = p_pWheel->m_Grip * (float)g_debugWheelLossGripScale;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].rollingFriction = p_pWheel->m_Friction + (float)g_debugWheelLossFrictionScale;
	}
	else
	{
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].radiusScale = m_WheelSetup.m_RearWheelLossRadiusScale;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].gripScale = p_pWheel->m_Grip * (float)g_debugWheelLossGripScale;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].rollingFriction = p_pWheel->m_Friction + (float)g_debugWheelLossFrictionScale;
	}
	
}

/*
===============
NAME: ReattachWheel

NOTE: Puts wheel back on
===============
*/
void C_DamageLayer::ReattachWheel(C_WheelDamage* p_pWheel)
{
	if (!p_pWheel->m_Detached
#ifndef CFG_GOLD
		|| p_pWheel->m_HackWheelLoss
#endif
		)		
		return;

	p_pWheel->m_Detached = false;
	m_pPartList->m_Parts[E_PartCode_Wheel].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(1);
	m_pPartList->m_Parts[E_PartCode_Tyre].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(1);
	m_pPartList->m_Parts[E_PartCode_Alloy].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(-1);
	m_pPartList->m_Parts[E_PartCode_AlloyRim].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(-1);
	m_pPartList->m_Parts[E_PartCode_AlloyBlur].m_Nodes[(E_PartSubCode)(p_pWheel->m_Index+1)].SetPermanentVisibility(-1);
	if (p_pWheel->m_TyreBurst)
		RepairTyre(p_pWheel);
	else
	{
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].radiusScale = 1.0f;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].gripScale = p_pWheel->m_Grip;
		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[p_pWheel->m_Index].rollingFriction = p_pWheel->m_Friction;
	}
}

/*
===============
NAME: AddDefferedDamageEffect

NOTE: Adds a damage effect for later processing
===============
*/
void C_DamageLayer::AddDefferedDamageEffect(E_DefferedEvent p_Effect, Maths::HVec3 p_Position, Maths::HVec3 p_Direction, float p_Scale, E_PartCode p_PartCode, E_PartSubCode p_PartSubCode)
{
	C_DefferedEvent effect(p_Effect, p_Position, p_Direction, Maths::HVec3(), p_Scale, p_PartCode, p_PartSubCode);
	m_DefferedEvents.PushBack(effect);
}

void C_DamageLayer::AddDefferedDamageEffect(E_DefferedEvent p_Effect, Maths::HVec3 p_Position, Maths::HVec3 p_Direction, Maths::HVec3 p_PointVelocity, float p_Scale, E_PartCode p_PartCode, E_PartSubCode p_PartSubCode)
{
	C_DefferedEvent effect(p_Effect, p_Position, p_Direction, p_PointVelocity, p_Scale, p_PartCode, p_PartSubCode);
	m_DefferedEvents.PushBack(effect);
}

Debug::Bool g_debugRepairDeformationOnly("Vehicle.Damage.Repair.RepairDeformationOnly", false);
Debug::Bool g_debugRepairScratches("Vehicle.Damage.Repair.RepairScratches", true);
Debug::Bool g_debugRepairMorphTargets("Vehicle.Damage.Repair.RepairMorphTargets", true);
Debug::Bool g_debugRepairRepairsParts("Vehicle.Damage.Repair.RepairParts", true);
Debug::Float g_debugOverrideRepairAmount("Vehicle.Damage.Repair.OverrideAmount", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: UpdateTransition

NOTE: Update transition
===============
*/
void C_DamageLayer::UpdateTransition(float p_TimeStep)
{
	if (!m_Transitioning || m_TransitionPeriod <= 0.0f)
		return;

	m_TransitionTime += p_TimeStep;

	if (m_TransitionTime >= m_TransitionPeriod)
	{
		m_TransitionTime	= m_TransitionPeriod;
//		m_Transitioning		= false;	
//		m_SlowMotionDamage	= false;
	}

	float fScale = Maths::Clamp(m_TransitionTime / m_TransitionPeriod, 0.0f, 1.0f);

	for (int iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
	{
		for (int iColumn = 0; iColumn < (*m_pDamageVolume)[iLayer].GetWidth(); iColumn++)
		{
			for (int iRow = 0; iRow < (*m_pDamageVolume)[iLayer].GetLength(); iRow++)
			{
				C_Node* pNode = (*m_pDamageVolume)[iLayer].GetNode(iRow, iColumn);

				pNode->m_Position		= Maths::Lerp(pNode->m_CachedPosition, pNode->m_TargetPosition, fScale);
				ClampNodePosition(pNode, pNode->m_Position);
				pNode->m_DeformLevel	= Maths::Lerp(pNode->m_CachedDeformLevel, pNode->m_TargetDeformLevel, fScale);
				pNode->m_OffsetVector	= pNode->m_Position - pNode->m_OriginalPosition;
				pNode->m_OffsetLength	= pNode->m_OffsetVector.Length();
				pNode->m_ScratchLevel	= Maths::Lerp(pNode->m_CachedScratchLevel, pNode->m_TargetScratchLevel, fScale);
			}
		}
	}

	//Now, sort out the wheels, as needed
	UpdateWheelDamage(m_SlowMotionDamage ? &m_CachedDamageEvent : NULL);
	//if (m_pVehiclePhysics)
	//{
	//	for (int iWheel = 0; iWheel < m_WheelNodes.Size(); iWheel++)
	//	{
	//		if (!m_WheelNodes[iWheel].m_pNode)
	//			continue;
	//		m_WheelNodes[iWheel].m_OffsetVector = m_WheelNodes[iWheel].m_pNode->m_Position - m_WheelNodes[iWheel].m_pNode->m_OriginalPosition;
	//		m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[iWheel].offset = RacingPhysics::Vector3R(	m_WheelNodes[iWheel].m_OffsetVector.X(),
	//																											m_WheelNodes[iWheel].m_OffsetVector.Y(),
	//																											m_WheelNodes[iWheel].m_OffsetVector.Z());
	//	}
	//}

	if (!g_debugRepairDeformationOnly)
	{
		for (int iPart = 0; iPart < m_pPartList->m_ActiveParts.Size(); iPart++)
		{
			C_Part* pNode = m_pPartList->m_ActiveParts[iPart].m_pNode;

			if (!pNode)// || pNode->m_Damage.m_DamageLevel == pNode->m_Damage.m_TargetDamageLevel)
				continue;

			float newDamageLevel = Maths::Lerp(pNode->m_Damage.m_CachedDamageLevel, pNode->m_Damage.m_TargetDamageLevel, fScale);
			
			E_DefferedEvent dEvent = pNode->HandleDamage(newDamageLevel);
			if (dEvent != E_DefferedEvent_Unknown)
				AddDefferedDamageEffect(dEvent, pNode->GetCentre(), Maths::HVec3(0.0f, 1.0f, 0.0f), 0.0f, pNode->m_PartCode, pNode->m_PartSubCode);
					
			UpdateDamageBehaviour(NULL, pNode, true);
		}
	}

	SetUpdateVolumeTexture(true);//m_UpdateVolumeTexture = true;
	SetUpdateInfoTexture(true);//m_UpdateInfoTexture = true;

}

/*
===============
NAME: RepairPart

NOTE: Repairs a part
===============
*/
void C_DamageLayer::RepairPart(E_PartCode p_PartCode, E_PartSubCode p_PartSubCode, bool repairAll)
{
	C_PartGroup* pPart = &m_pPartList->m_Parts[p_PartCode];
	if (pPart)
	{
		if (repairAll)
		{
			for (S32 iNode = 0; iNode < pPart->m_Nodes.Size(); iNode++)
			{
				C_Part* pNode = &pPart->m_Nodes[iNode];

				pNode->m_Damage.m_ImpactSize = 0.0f;
				
				pNode->HandleDamage(0.0f);

				if (pNode->m_pDamageBehaviour)
					pNode->m_pDamageBehaviour->Repair(1.0f);
			}
		}
		else
		{
			C_Part* pNode = &pPart->m_Nodes[p_PartSubCode];

			pNode->m_Damage.m_ImpactSize = 0.0f;
			
			pNode->HandleDamage(0.0f);

			if (pNode->m_pDamageBehaviour)
				pNode->m_pDamageBehaviour->Repair(1.0f);
		}

		m_UpdateVolumeTexture = true;
		m_UpdateInfoTexture = true;
	}
}

Debug::Float g_debugRepairAmountScale("Vehicle.Damage.Repair.AmountScale", 2.0f, Debug::Range(0.0f, 4.0f), Debug::Step(0.001f));

Debug::Bool g_debugRepairAlwaysRepairGlass("Vehicle.Damage.Repair.AlwaysRepairGlass", true);
Debug::Bool g_debugRepairAlwaysRepairLights("Vehicle.Damage.Repair.AlwaysRepairLights", true);

/*
===============
NAME: Repair

NOTE: Repair damage
===============
*/
void C_DamageLayer::Repair(float p_Amount, float p_CurrentDamagePercentage, float p_Duration)
{
	if (m_Transitioning)
		return;

	if (p_Duration > 0.0f)
	{
		m_Transitioning = true;
		m_TransitionTime = 0.0f;
		m_TransitionPeriod = p_Duration;
	}

	if (g_debugOverrideRepairAmount > 0.0f)
		p_Amount = g_debugOverrideRepairAmount;

	float fTargetAmount = p_CurrentDamagePercentage;
	float fLerp = fTargetAmount == 0.0f ? 1.0f : fTargetAmount;

	for (int iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
	{
		for (int iColumn = 0; iColumn < (*m_pDamageVolume)[iLayer].GetWidth(); iColumn++)
		{
			for (int iRow = 0; iRow < (*m_pDamageVolume)[iLayer].GetLength(); iRow++)
			{
				C_Node* pNode = (*m_pDamageVolume)[iLayer].GetNode(iRow, iColumn);

				if (m_Transitioning)
				{
					pNode->m_CachedPosition		= pNode->m_Position;
					pNode->m_TargetPosition		= Maths::Lerp(pNode->m_Position, pNode->m_OriginalPosition, fLerp);
					
					if (g_debugRepairScratches)
					{
						pNode->m_CachedScratchLevel	= pNode->m_ScratchLevel;
						pNode->m_TargetScratchLevel	= Maths::Lerp(pNode->m_ScratchLevel, pNode->m_OriginalScratchLevel, fLerp);
					}
					else
					{
						pNode->m_CachedScratchLevel	= pNode->m_ScratchLevel;
						pNode->m_TargetScratchLevel	= pNode->m_ScratchLevel;
					}

					if (g_debugRepairMorphTargets)
					{
						pNode->m_CachedDeformLevel	= pNode->m_DeformLevel;
						pNode->m_TargetDeformLevel	= Maths::Lerp(pNode->m_DeformLevel, pNode->m_OriginalDeformLevel, fLerp);
					}
					else
					{
						pNode->m_CachedDeformLevel	= pNode->m_DeformLevel;
						pNode->m_TargetDeformLevel	= pNode->m_DeformLevel;
					}
				}
				else
				{
					pNode->m_Position		= Maths::Lerp(pNode->m_Position, pNode->m_OriginalPosition, fLerp);
					if (g_debugRepairScratches)
						pNode->m_ScratchLevel	= Maths::Lerp(pNode->m_ScratchLevel, pNode->m_OriginalScratchLevel, fLerp);
					if (g_debugRepairMorphTargets)
						pNode->m_DeformLevel	= Maths::Lerp(pNode->m_DeformLevel, pNode->m_OriginalDeformLevel, fLerp);
				}
			}
		}
	}

	//TODO: Work out system for repairing lights - based on node movement maybe?
	if (g_debugRepairRepairsParts)
	{
		for (int iPart = 0; iPart < m_pPartList->m_ActiveParts.Size(); iPart++)
		{
			C_Part* pNode = m_pPartList->m_ActiveParts[iPart].m_pNode;

			if (!pNode)
				continue;

			if (g_debugRepairDeformationOnly)
			{
				pNode->m_Damage.m_CachedDamageLevel = pNode->m_Damage.m_DamageLevel;
				pNode->m_Damage.m_TargetDamageLevel = pNode->m_Damage.m_DamageLevel;
			}
			else
			{
				pNode->m_Damage.m_CachedDamageLevel = pNode->m_Damage.m_DamageLevel;
				pNode->m_Damage.m_TargetDamageLevel = Maths::Lerp(pNode->m_Damage.m_DamageLevel, 0.0f, fLerp);
			}
			if (!m_Transitioning)
			{
				pNode->HandleDamage(pNode->m_Damage.m_TargetDamageLevel);
				UpdateDamageBehaviour(NULL, pNode, true);
			}
		}

		RepairPart(E_PartCode_LicensePlate, E_PartSubCode_Front);
		m_pPartList->m_Parts[E_PartCode_LicensePlate].m_Nodes[E_PartSubCode_Front].SetPermanentVisibility(1);
		m_pPartList->m_Parts[E_PartCode_LicensePlate].m_Nodes[E_PartSubCode_Front].m_Damage.m_Detached = false;
		RepairPart(E_PartCode_LicensePlate, E_PartSubCode_Back);
		m_pPartList->m_Parts[E_PartCode_LicensePlate].m_Nodes[E_PartSubCode_Back].SetPermanentVisibility(1);
		m_pPartList->m_Parts[E_PartCode_LicensePlate].m_Nodes[E_PartSubCode_Back].m_Damage.m_Detached = false;
	}

	if (g_debugRepairAlwaysRepairGlass)
	{
		C_PartGroup* pPart = &m_pPartList->m_Parts[E_PartCode_Window];
		if (pPart)
		{
			for (S32 iNode = 0; iNode < pPart->m_Nodes.Size(); iNode++)
			{
				C_Part* pNode = &pPart->m_Nodes[iNode];

				if (pNode->m_pMaterialParameters)
					pNode->HandleDamage(0.0f);
			}
		}
		pPart = &m_pPartList->m_Parts[E_PartCode_HeadlightCover];
		if (pPart)
		{
			for (S32 iNode = 0; iNode < pPart->m_Nodes.Size(); iNode++)
			{
				C_Part* pNode = &pPart->m_Nodes[iNode];

				if (pNode->m_pDamageBehaviour)
					pNode->m_pDamageBehaviour->Repair(1.0f);
			}
		}
	}
	//Verbose and hacky, don't leave this laying around
	if (g_debugRepairAlwaysRepairLights)
	{
		for (S32 iLight = E_PartCode_HeadLight; iLight <= E_PartCode_RearAndBrakeLight; iLight++)
		{
			C_PartGroup* pPart = &m_pPartList->m_Parts[iLight];
			if (pPart)
			{
				for (S32 iNode = 0; iNode < pPart->m_Nodes.Size(); iNode++)
				{
					C_Part* pNode = &pPart->m_Nodes[iNode];

					if (pNode->m_pDamageBehaviour)
						pNode->m_pDamageBehaviour->Repair(1.0f);
				}
			}
		}
	}

#ifndef CFG_GOLD
	for (S32 iWheel = 0; iWheel < m_WheelNodes.Size(); iWheel++)
	{
		m_WheelNodes[iWheel].m_HackTyreBurst = false;
		m_WheelNodes[iWheel].m_HackWheelLoss = false;
	}
#endif
	m_UpdateVolumeTexture = true;
	m_UpdateInfoTexture = true;

	for (S32 iVertex = 0; iVertex < m_CollisionShapeVertices.Size(); iVertex++)
		m_CollisionShapeVertices[iVertex].ResetVertex();

#ifdef PLATFORM_PS3
	m_pSpuDamageContext->forceRun	= true;
#endif //#ifdef PLATFORM_PS3
	return;
}

/*
===============
NAME: GetClosestNodeIndex

NOTE: Return the index of the closest node to a given position
===============
*/
void C_DamageLayer::GetClosestNodeIndex(Maths::HVec3 p_Position, 
										S32& p_LayerIndex, S32& p_RowIndex, S32& p_ColIndex, 
										bool clampToEdge)
{
	if ((*m_pDamageVolume).Size() == 0)
		return;

	float fNumNodesW = (float)(*m_pDamageVolume)[0].GetWidth() - 1.0f;
	float fNumNodesL = (float)(*m_pDamageVolume)[0].GetLength() - 1.0f;
	float fNumNodesH = (float)(*m_pDamageVolume).Size() - 1.0f;

	float fScaleX = (p_Position.X() + (m_VehicleInfo.m_Extents.X())) / m_VehicleInfo.m_Extents.X();
	float fScaleY = (p_Position.Y() + (m_VehicleInfo.m_Extents.Y())) / m_VehicleInfo.m_Extents.Y();
	float fScaleZ = (p_Position.Z() + (m_VehicleInfo.m_Extents.Z())) / m_VehicleInfo.m_Extents.Z();

	float fNodeX = fScaleX * fNumNodesW;
	float fNodeY = fScaleY * fNumNodesH;
	float fNodeZ = fScaleZ * fNumNodesL;

	float fColIndex = (float)p_ColIndex;
	float fRemX = modf(fNodeX, &fColIndex);
	p_ColIndex = fRemX >= 0.5f ? fColIndex++ : fColIndex;
	Maths::Clamp(p_ColIndex, 0, (S32)fNumNodesW);

	float fLayerIndex = (float)p_LayerIndex;
	float fRemY = modf(fNodeY, &fLayerIndex);
	p_LayerIndex = fRemY >= 0.5f ? fLayerIndex++ : fLayerIndex;
	Maths::Clamp(p_LayerIndex, 0, (S32)fNumNodesH);

	float fRowIndex = (float)p_RowIndex;
	float fRemZ = modf(fNodeZ, &fRowIndex);
	p_RowIndex = fRemZ >= 0.5f ? fRowIndex++ : fRowIndex;
	Maths::Clamp(p_RowIndex, 0, (S32)fNumNodesL);

	if (clampToEdge)
	{
		if (p_RowIndex < p_ColIndex)
		{
			if (p_RowIndex < (*m_pDamageVolume)[0].GetWidth() / 2)
				p_RowIndex = 0;
			else
				p_RowIndex = (*m_pDamageVolume)[0].GetWidth() - 1;
		}
		else
		{
			if (p_ColIndex < (*m_pDamageVolume)[0].GetLength() / 2)
				p_ColIndex = 0;
			else
				p_ColIndex = (*m_pDamageVolume)[0].GetLength() - 1;
		}
	}
}

/*
===============
NAME: GetClosestNode

NOTE: Return the closest node to a given position
===============
*/
C_Node* C_DamageLayer::GetClosestNode(Maths::HVec3 p_Position)
{
	float dist = 999999999.0f;
	
	C_Node* pClosest = NULL;

	for (S32 iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
	{
		for (int iLength = 0; iLength < (*m_pDamageVolume)[iLayer].GetLength(); iLength++)
		{	
			for (int iWidth = 0; iWidth < (*m_pDamageVolume)[iLayer].GetWidth(); iWidth++)
			{
				int index = (iLength * (*m_pDamageVolume)[iLayer].GetWidth()) + iWidth;

				C_Node* pNode = &(*m_pDamageVolume)[iLayer].m_Nodes[index];

				float length2 = (p_Position - pNode->m_OriginalPosition).Length2();

				if (length2 < dist)
				{
					pClosest = pNode;
					dist = length2;
				}
			}
		}
	}

	return pClosest;
}

Debug::Int g_debugDamageTextureWidth("Vehicle.Damage.Texture.Width", 8, Debug::Range(1, 10), Debug::Step(1));
Debug::Int g_debugDamageTextureHeight("Vehicle.Damage.Texture.Height", 8, Debug::Range(1, 10), Debug::Step(1));
Debug::Int g_debugDamageTextureDepth("Vehicle.Damage.Texture.Depth", 3, Debug::Range(1, 10), Debug::Step(1));

/*
===============
NAME: CreateVolume

NOTE: Creates damage volume
===============
*/
bool C_DamageLayer::CreateVolume()
{
MEMORY_TRACKER_PUSH("graphics.vehicle.damage");
	for (int iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
	{
		(*m_pDamageVolume)[iLayer].m_Length		= 0;
		(*m_pDamageVolume)[iLayer].m_Width		= 0;
	}

	(*m_pDamageVolume).Clear();

#ifdef _DAMAGE_USE_3D_TEX
	int DEBUG_numLayers = g_debugDamageTextureDepth;
#else
	int DEBUG_numLayers = 1;
#endif
	int DEBUG_numWidth = g_debugDamageTextureWidth;
	int DEBUG_numLength = g_debugDamageTextureHeight;
		
	(*m_pDamageVolume).Resize(DEBUG_numLayers);
	
	Btl::Vector<float> wheelDistances;
	Btl::Vector<float> wheelDistancesMin;
	Btl::Vector<float> wheelDistancesMax;
	m_WheelNodes.Clear();

	if (m_pVehiclePhysics)
	{
		for (int iWheel = 0; iWheel < m_pVehiclePhysics->GetNumWheels(); iWheel++)
		{			
			wheelDistances.PushBack(99999.99999f);
			wheelDistancesMin.PushBack(99999.99999f);
			wheelDistancesMax.PushBack(99999.99999f);
			m_WheelNodes.PushBack();
			m_WheelNodes.Back().m_pNode = NULL;
			m_WheelNodes.Back().m_pMinNode = NULL;
			m_WheelNodes.Back().m_pMaxNode = NULL;			
			m_WheelNodes.Back().m_OffsetVector = Maths::HVec3(0.0f, 0.0f, 0.0f);
			m_WheelNodes.Back().m_Index = iWheel;
			m_WheelNodes.Back().m_Detached = false;
			m_WheelNodes.Back().m_RadiusScale = m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[iWheel].radiusScale;
			m_WheelNodes.Back().m_Radius = m_pVehiclePhysics->Wheel(iWheel).Radius();
			m_WheelNodes.Back().m_Grip = m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[iWheel].gripScale;
			m_WheelNodes.Back().m_Friction = m_pVehiclePhysics->PerformanceModifiers().wheelModifiers[iWheel].rollingFriction;
			m_WheelNodes.Back().m_OffsetLength = 0.0f;
			m_WheelNodes.Back().m_TyreBurst = false;
						
			RacingPhysics::Point3R rpWheelPos = m_pVehiclePhysics->GetAxis().Local(m_pVehiclePhysics->Wheel(iWheel).GetWheelPosition());
			Maths::HVec3 wheelPos(rpWheelPos.X(), rpWheelPos.Y(), rpWheelPos.Z());
			m_WheelNodes.Back().m_OriginalPosition = wheelPos;
		}
	}
	
	//Traverse layers
	for (int iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
	{
		(*m_pDamageVolume)[iLayer].m_Length = DEBUG_numLength;
		(*m_pDamageVolume)[iLayer].m_Width	= DEBUG_numWidth;

		//Distance between nodes
		float wStep = (m_VehicleInfo.m_Extents.X() * 2.0f) / (float)((*m_pDamageVolume)[iLayer].m_Width - 1);
		float lStep = (m_VehicleInfo.m_Extents.Z() * 2.0f) / (float)((*m_pDamageVolume)[iLayer].m_Length - 1);
		float hStep = (m_VehicleInfo.m_Extents.Y() * 2.0f) / (float)((*m_pDamageVolume).Size() - 1);
		
		//Traverse length
		for (int iLength = 0; iLength < (*m_pDamageVolume)[iLayer].GetLength(); iLength++)
		{	
			//Traverse width
			for (int iWidth = 0; iWidth < (*m_pDamageVolume)[iLayer].GetWidth(); iWidth++)
			{
				float wPos = (float)iWidth / (float)((*m_pDamageVolume)[iLayer].GetWidth() - 1);
				float lPos = (float)iLength / (float)((*m_pDamageVolume)[iLayer].GetLength() - 1);
		
				float fWidth = Maths::Clamp(m_WidthCurve.Evaluate(lPos), 0.0f, 1.0f);
				fWidth = Maths::Lerp(m_VehicleInfo.m_Extents.X() * -fWidth, m_VehicleInfo.m_Extents.X() * fWidth, wPos);

				float fLength = Maths::Clamp(m_LengthCurve.Evaluate(wPos), 0.0f, 1.0f);
				fLength = Maths::Lerp(m_VehicleInfo.m_Extents.Z() * -fLength, m_VehicleInfo.m_Extents.Z() * fLength, lPos);

				float fHeight = Maths::Clamp(m_HeightCurve.Evaluate(lPos), 0.0f, 1.0f);		
				float hStep = 0.0f;
				if ((*m_pDamageVolume).Size() == 1)
					hStep = 0.0f;
				else
					hStep = fHeight / (float)((*m_pDamageVolume).Size() - 1);

				//Get node index
				int index = (iLength * (*m_pDamageVolume)[iLayer].GetWidth()) + iWidth;

				C_Node* pNode = &(*m_pDamageVolume)[iLayer].m_Nodes[index];
				
				pNode->Reset(false);

				pNode->m_OriginalPosition		= Maths::HVec3(fWidth, hStep * (float)iLayer * m_HeightRange, fLength + m_ZOffset);
				pNode->m_OriginalScratchLevel	= m_InitialScratchLevel;
				pNode->m_OriginalDeformLevel	= m_InitialDamageLevel;				
				pNode->Reset(true);

				//Find if a wheel is closest to this node
				for (int iWheel = 0; iWheel < m_WheelNodes.Size(); iWheel++)
				{
					float fDist = (pNode->m_OriginalPosition - m_WheelNodes[iWheel].m_OriginalPosition).Length();
					if (fDist < wheelDistances[iWheel])
					{
						wheelDistances[iWheel]			= fDist;
						m_WheelNodes[iWheel].m_pNode	= pNode;
					}

					Maths::HVec3 minPos = m_WheelNodes[iWheel].m_OriginalPosition - Maths::HVec3(0.0f, 0.0f, -m_WheelNodes[iWheel].m_Radius);
					float fDistMin = (pNode->m_OriginalPosition - minPos).Length();
					if (fDistMin < wheelDistancesMin[iWheel])
					{
						wheelDistancesMin[iWheel]		= fDist;
						m_WheelNodes[iWheel].m_pMinNode	= pNode;
					}

					Maths::HVec3 maxPos = m_WheelNodes[iWheel].m_OriginalPosition - Maths::HVec3(0.0f, 0.0f, m_WheelNodes[iWheel].m_Radius);
					float fDistMax = (pNode->m_OriginalPosition - maxPos).Length();
					if (fDistMax < wheelDistancesMax[iWheel])
					{
						wheelDistancesMax[iWheel]		= fDist;
						m_WheelNodes[iWheel].m_pMaxNode	= pNode;
					}
				}
			}
		}
	}

	for (S32 iWheel = 0; iWheel < m_WheelNodes.Size(); iWheel++)
	{
		//BIZ_ASSERT(m_WheelNodes[iWheel].m_pNode != NULL);
		//BIZ_ASSERT(m_WheelNodes[iWheel].m_pMinNode != NULL);
		//BIZ_ASSERT(m_WheelNodes[iWheel].m_pMaxNode != NULL);
		if (m_WheelNodes[iWheel].m_pNode == NULL)
			bool breakHere = true;
		if (m_WheelNodes[iWheel].m_pMinNode == NULL)
			bool breakHere = true;
		if (m_WheelNodes[iWheel].m_pMaxNode == NULL)
			bool breakHere = true;
	}

	//Setup height profile for shader damage
	m_HeightProfile.Clear();
	for (int iLength = 0; iLength < (*m_pDamageVolume)[0].GetLength(); iLength++)
	{
		float fLength = (float)iLength / (float)((*m_pDamageVolume)[0].GetLength()-1);
		float fHeight = Maths::Clamp(m_HeightCurve.Evaluate(fLength), 0.0f, 1.0f);
		//fHeight *= m_VehicleInfo.m_Extents.Y();
		m_HeightProfile.PushBack(fHeight);
	}

	m_DamageState.Clear();

MEMORY_TRACKER_POP("graphics.vehicle.damage");

	return true;
}

/*
===============
NAME: MapCollisionVertices

NOTE: Maps collision shape vertices to lattice
===============
*/
void C_DamageLayer::MapCollisionVertices(Gravity::C_Body* p_pCollisionBody)
{
	if (!p_pCollisionBody)
		return;

	m_CollisionShapeVertices.Clear();

	Collision::C_CompositeShape* pComposite = (Collision::C_CompositeShape*)p_pCollisionBody->GetShape();
	Collision::C_CompositeShapeIterator iter(pComposite);

	//for (S32 iShape = 0; iShape < pComposite->m_NumShapes; iShape++)
	while(iter.HasNextShape())
	{		
		iter.NextShape();
		Collision::C_Shape* pShape = iter.GetCurrentShape<Collision::C_Shape>();
		
		if (pShape->m_ShapeType == Collision::E_Composite_Shape)
		{
			Collision::C_CompositeShape* pCompositeShape	= (Collision::C_CompositeShape*)pShape;

			Collision::C_CompositeShapeIterator iter2(pCompositeShape);
			//for (S32 iShape2 = 0; iShape2 < pCompositeShape->m_NumShapes; iShape2++)
			while(iter2.HasNextShape())
			{	
				iter2.NextShape();
				Collision::C_Shape* pShape2 = iter2.GetCurrentShape<Collision::C_Shape>();
				if (pShape2->m_ShapeType == Collision::E_ConvexHull_Shape)
				{
					Collision::C_ConvexHullShape* pConvexHullShape	= (Collision::C_ConvexHullShape*)pShape2;
					Collision::C_ConvexHull* pConvexHull			= &pConvexHullShape->m_Hull;
					
					MathsStorage::Fixed3_S16<14>* pVerts = pConvexHull->GetVertices();
					if (!pVerts)
						continue;
					
					for (S32 iVertex = 0; iVertex < pConvexHull->GetNumVertices(); iVertex++)
					{								
						float fDist = 99999999.99999999f;
						C_Node* pClosestNode = &(*m_pDamageVolume)[1].m_Nodes[0];
						/*Maths::HVec3 position(pConvexHull->m_pVertices + iVertex);
						position = position * pConvexHull->m_Range + pConvexHull->m_Low;*/
						Maths::HVec3 position(pConvexHull->GetVertex(iVertex));

						for (S32 iNode = 0; iNode < (*m_pDamageVolume)[1].Size(); iNode++)
						{
							C_Node* pNode = &(*m_pDamageVolume)[1].m_Nodes[iNode];

							float fLength = (pNode->m_Position - position).Length2();

							if (fLength < fDist)
							{
								pClosestNode = pNode;
								fDist = fLength;
							}
						}

						m_CollisionShapeVertices.PushBack();
						m_CollisionShapeVertices.Back().m_Range = pConvexHull->Range();
						m_CollisionShapeVertices.Back().m_Low = pConvexHull->Low();
						m_CollisionShapeVertices.Back().SetVertex(pClosestNode, pVerts + iVertex);						
					}
				}
			}
		}
	}
	
}

/*
===============
NAME: CreateVolumeTexture

NOTE: Creates volume texture
===============
*/
bool C_DamageLayer::CreateVolumeTexture(U32 vehicleIndex)
{
#if !defined(ENABLE_VEHICLE_SHADER_DAMAGE)
	return true;
#endif

	CreateVolume();

	ResetVolume();

#if !defined(PLATFORM_PS3)

	BIZ_ASSERT(vehicleIndex < ms_NumTextureResourcesVehicles);

	//Get textures from static
	for (int iTex = 0; iTex < m_DamageVolumeTextures.Size(); iTex++)
		m_DamageVolumeTextures[iTex] = &ms_TextureVolumeResources[iTex][vehicleIndex];
	for (int iTex = 0; iTex < m_DamageInfoTextures.Size(); iTex++)
		m_DamageInfoTextures[iTex] = &ms_TextureInfoResources[iTex][vehicleIndex];


	for (int iTex = 0; iTex < m_DamageVolumeTextures.Size(); iTex++)
		UpdateVolumeTexture(true);
	for (int iTex = 0; iTex < m_DamageInfoTextures.Size(); iTex++)
		UpdateInfoTexture(true);
#else
	//Get textures from static
	for (int iTex = 0; iTex < m_DamageVolumeTextures.Size(); iTex++)
		m_DamageVolumeTextures[iTex] = NULL;
	for (int iTex = 0; iTex < m_DamageInfoTextures.Size(); iTex++)
		m_DamageInfoTextures[iTex] = NULL;

#endif

	return true;
}

/*
===============
NAME: UpdateVolumeTexture

NOTE: Updates volume texture
===============
*/
void C_DamageLayer::UpdateVolumeTexture(bool reset)
{	
#if !defined(ENABLE_VEHICLE_SHADER_DAMAGE) || defined(PLATFORM_PS3)
	return;
#endif

	PERF_EVENT("C_DamageLayer.UpdateVolumeTexture");

	SwapTextures();

	//Display::Device()->SetVertexTexture(0, NULL);

#ifdef _DAMAGE_USE_3D_TEX
	Display::C_Texture3D* pTexture = GetDamageVolumeTexture();
	Display::C_LockedBox locked = pTexture->LockBox3D(0);
#else
	Display::C_Texture2D* pTexture = GetDamageVolumeTexture();
	Display::C_LockedRect locked = pTexture->LockRect2D(0);
#endif
	
#if defined(_DAMAGE_USE_F32) || defined(BLUR2PC_DAMAGE_VOLUME_F32)
	F32* pData	= (F32*)locked.pBits;
#else
	DWORD* pData	= (DWORD*)locked.pBits;
#endif

	U32 height		= pTexture->GetHeight();
	U32 width		= pTexture->GetWidth();
#ifdef _DAMAGE_USE_3D_TEX
	U32 depth		= pTexture->GetDepth();
#else
	U32 depth		= 1;
#endif

	BIZ_ASSERT(reset || (depth == (*m_pDamageVolume).Size()));

	for (unsigned int iLayer = 0; iLayer < depth; iLayer++)
	{
		for (U32 v = 0; v < height; v++)
		{	
			for (U32 u = 0; u < width; u++)
			{
				if (reset)
				{					
				#if defined(_DAMAGE_USE_F32) || defined(BLUR2PC_DAMAGE_VOLUME_F32)
					*(pData + 0) = 0.0f;
					*(pData + 1) = 0.0f;
					*(pData + 2) = 0.0f;
					*(pData + 3) = 0.0f;
					pData += 4;
				#else
					U8 a = 0;
					U8 r = 127;
					U8 g = 127;
					U8 b = 127;
					(*pData++) = (a << 24) + (r << 16) + (g << 8) + (b << 0);
				#endif				
					continue;
				}

				int index = (v * width) + u;

				C_Node* pNode = &(*m_pDamageVolume)[iLayer].m_Nodes[index];
				
#if defined(_DAMAGE_USE_F32) || defined(BLUR2PC_DAMAGE_VOLUME_F32)
				Maths::HVec3 vector = pNode->m_Position - pNode->m_OriginalPosition;
				pData[0] = vector.X();
				pData[1] = vector.Y();
				pData[2] = vector.Z();
				pData[3] = pNode->m_DeformLevel;
				pData += 4;
#else
				Maths::HVec3 vector = pNode->m_Position - pNode->m_OriginalPosition;
				U8 r = (U8)(Maths::Clamp((2.0f + (float)vector.X()) / 4.0f, 0.0f, 1.0f) * 254.0f);
				U8 g = (U8)(Maths::Clamp((2.0f + (float)vector.Y()) / 4.0f, 0.0f, 1.0f) * 254.0f);
				U8 b = (U8)(Maths::Clamp((2.0f + (float)vector.Z()) / 4.0f, 0.0f, 1.0f) * 254.0f);
				U8 a = (U8)(pNode->m_ScratchLevel * 255.0f);
				(*pData++) = (a << 24) + (r << 16) + (g << 8) + (b << 0);
#endif		
			}
#ifdef _DAMAGE_USE_3D_TEX			
		#if defined(_DAMAGE_USE_F32) || defined(BLUR2PC_DAMAGE_VOLUME_F32)
			pData += (locked.rowPitch - (width * sizeof(F32) * 4)) / 4;
		#else
			pData += (locked.rowPitch - (width * sizeof(DWORD))) / 4;
		#endif
#else
		#if defined(_DAMAGE_USE_F32) || defined(BLUR2PC_DAMAGE_VOLUME_F32)
			pData += (locked.pitch - (width * sizeof(F32) * 4)) / 4;
		#else
			pData += (locked.pitch - (width * sizeof(DWORD))) / 4;
		#endif			
#endif
		}

#ifdef _DAMAGE_USE_3D_TEX	
	#if defined(_DAMAGE_USE_F32) || defined(BLUR2PC_DAMAGE_VOLUME_F32)
		// [Blur2PC] was a TODO; same byte arithmetic as the 8-bit case (pData steps 4 bytes)
		DWORD sliceSizeUsedInBytes = height * locked.rowPitch;
		pData += (locked.slicePitch - sliceSizeUsedInBytes) / 4;
	#else
		DWORD sliceSizeUsedInBytes = height * locked.rowPitch;
		pData += (locked.slicePitch - sliceSizeUsedInBytes) / 4;
	#endif
#endif
	}

#ifdef _DAMAGE_USE_3D_TEX
	pTexture->UnlockBox3D(0);	
#else
	pTexture->UnlockRect2D(0);	
#endif

	m_NextVolumeTextureUpdate = (int)g_debugUpdatesBetweenVolumeTextureUpdate;
}	

/*
===============
NAME: UpdateInfoTexture

NOTE: Updates info texture
===============
*/
void C_DamageLayer::UpdateInfoTexture(bool reset)
{	
#if !defined(ENABLE_VEHICLE_SHADER_DAMAGE) || defined(PLATFORM_PS3)

	return;
#endif

	SwapInfoTextures();

	//Display::Device()->SetVertexTexture(1, NULL);

	Display::C_Texture2D* pTexture = GetDamageInfoTexture();
	Display::C_LockedRect locked = pTexture->Lock(0);
	
#ifdef _DAMAGE_USE_F32
	F32* pData	= (F32*)locked.pBits;
#else
	DWORD* pData	= (DWORD*)locked.pBits;
#endif

	U32 height		= pTexture->GetHeight();
	U32 width		= pTexture->GetWidth();

	for (U32 v = 0; v < height; v++)
	{	
		for (U32 u = 0; u < width; u++)
		{
			int index = (v * width) + u;

			if (reset)
			{
				U8 r = 0;
				U8 g = 0;
				U8 b = 0;
				U8 a = 0;
				(*pData++) = (a << 24) + (r << 16) + (g << 8) + (b << 0);
				continue;
			}
			float fHeight = m_HeightProfile[v];

			float maxDeform = 0.0f;
			for (int iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
			{
				float deformLevel = (*m_pDamageVolume)[iLayer].m_Nodes[index].m_DeformLevel;
				if (deformLevel > maxDeform)
					maxDeform = deformLevel;
			}

#ifdef _DAMAGE_USE_F32
			pData = {fHeight, 0.0f, 0.0f, 0.0f);
			pData += 4;
#else
			U8 r = (U8)(fHeight*255.0f);
			U8 g = (U8)(maxDeform*255.0f);
			U8 b = 0;
			U8 a = 0;
			(*pData++) = (a << 24) + (r << 16) + (g << 8) + (b << 0);
#endif		
		}
	#ifdef _DAMAGE_USE_F32
		pData += (locked.pitch - (width * sizeof(F32) * 4)) / 4;
	#else
		pData += (locked.pitch - (width * sizeof(DWORD))) / 4;
	#endif			
	}

	pTexture->Unlock(0);

	m_NextInfoTextureUpdate = (int)g_debugUpdatesBetweenInfoTextureUpdate;
}

Debug::Enum<E_PartCode> g_debugConstraintPartCode("Vehicle.Damage.Parts.Constraints.PartCode", E_PartCode_None);
Debug::Enum<E_PartSubCode> g_debugConstraintSubCode("Vehicle.Damage.Parts.Constraints.SubCode", E_PartSubCode_None);

/*
===============
NAME: EditConstraints

NOTE: Edit/place constraints
===============
*/
void C_DamageLayer::EditConstraints(XtVehicle* p_pVehicle, Gravity::C_Body* p_pCollisionBody, Gravity::C_Scene* p_pPhysicsScene)
{
	if (!p_pVehicle || !p_pVehicle->m_DamagedParts || !p_pPhysicsScene)
		return;

	Maths::HVec4 ax(m_VehicleInfo.m_Matrix.AX().X(), m_VehicleInfo.m_Matrix.AX().Y(), m_VehicleInfo.m_Matrix.AX().Z(), 1.0f);
	Maths::HVec4 ay(m_VehicleInfo.m_Matrix.AY().X(), m_VehicleInfo.m_Matrix.AY().Y(), m_VehicleInfo.m_Matrix.AY().Z(), 1.0f);
	Maths::HVec4 az(m_VehicleInfo.m_Matrix.AZ().X(), m_VehicleInfo.m_Matrix.AZ().Y(), m_VehicleInfo.m_Matrix.AZ().Z(), 1.0f);
	Maths::HVec4 aw(m_VehicleInfo.m_Matrix.AW().X(), m_VehicleInfo.m_Matrix.AW().Y(), m_VehicleInfo.m_Matrix.AW().Z(), 1.0f);
	Maths::HMat44 mat(ax, ay, az, aw);

	for (int iConstraint = 0; iConstraint < p_pVehicle->m_Constraints->m_Constraints.count; iConstraint++)
	{
		if (!p_pVehicle->m_Constraints->m_Constraints[iConstraint] || !p_pVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part)
			continue;
		{
			XtPart* pPart = p_pVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part;
			//if (!pPart || (pPart->m_PartCode != g_debugConstraintPartCode && pPart->m_PartSubCode != g_debugConstraintSubCode))
			//	continue;

			Vehicle::XtConstraintBase* pBase = p_pVehicle->m_Constraints->m_Constraints[iConstraint];

			pBase->DebugDraw((Maths::HMat43)mat);

		}
	}
}

Debug::Bool g_debugSetupConstraints("Vehicle.Damage.Constraints.Setup", true);

/*
===============
NAME: SetupDamagedParts

NOTE: Sets up damage parts from Sorcery
===============
*/
void C_DamageLayer::SetupDamagedParts(XtVehicle* p_pVehicle, Gravity::C_Body* p_pCollisionBody, Gravity::C_Scene* p_pPhysicsScene, Maths::HMat43 p_VehicleMatrix)
{
	if (!m_pPartList || !p_pVehicle || !p_pVehicle->m_DamagedParts || !p_pPhysicsScene)
		return;

	m_pPartList->m_DamageParts.Clear();
	m_pPartList->m_ConstrainedParts.Clear();

MEMORY_TRACKER_PUSH("graphics.vehicle.damage");
	//Go through all the components that exist in this vehicle
	for (int iComponent = 0; iComponent < m_pPartList->m_ActiveParts.Size(); iComponent++)
	{
		C_PartList::ActivePart* pComponent = &m_pPartList->m_ActiveParts[iComponent];

		if (!pComponent->m_pNode || !pComponent->m_pNode->m_pPhysicsBody)
			continue;
		
		//Traverse all damaged parts in Sorcery and look for match
		int partCount = p_pVehicle->m_DamagedParts->m_Parts.count;
		bool bFound = false;
		for (int iPart = 0; iPart < p_pVehicle->m_DamagedParts->m_Parts.count && !bFound; iPart++)
		{
			XtDamagedPart* pPart = &p_pVehicle->m_DamagedParts->m_Parts[iPart];

			if (!pPart->m_Part)
				continue;
	
			E_PartCode pPartCode = pPart->m_Part->m_PartCode;
			E_PartSubCode pPartSubCode = pPart->m_Part->m_PartSubCode;

			if (pPart->m_Part->m_PartCode == pComponent->m_PartCode && pPart->m_Part->m_PartSubCode == pComponent->m_PartSubCode)
			{
				m_pPartList->m_DamageParts.PushBack(pComponent->m_pNode);

				bFound = true;

				pComponent->m_pNode->m_pPhysicsBody->SetMass(pPart->m_Mass);
				pComponent->m_pNode->m_Damage.Clear();
				pComponent->m_pNode->m_Damage.m_Behaviour = pPart->m_Behaviour;
				for (S32 iTransfer = 0; iTransfer < pPart->m_DamageTransfer.count; iTransfer++)
				{
					XtDamageTransfer* pTrans = &pPart->m_DamageTransfer[iTransfer];
					if (!pTrans || !pTrans->m_Part)
						continue;

					C_Part* pTransferNode = &m_pPartList->m_Parts[pTrans->m_Part->m_PartCode].m_Nodes[pTrans->m_Part->m_PartSubCode];
					if (pTransferNode && pTransferNode->m_pModel && pTransferNode->m_pModelInstance && pTransferNode->m_pElement && pTransferNode->m_PartCode != E_PartCode_None)
					{
						for (int iTPart = 0; iTPart < p_pVehicle->m_DamagedParts->m_Parts.count; iTPart++)
						{
							XtDamagedPart* pTPart = &p_pVehicle->m_DamagedParts->m_Parts[iTPart];
							if (pTPart && pTPart->m_Part && pTPart->m_Part->m_PartCode == pTrans->m_Part->m_PartCode && pTPart->m_Part->m_PartSubCode == pTrans->m_Part->m_PartSubCode)
							{
								pTransferNode->m_Damage.m_Behaviour = pTPart->m_Behaviour;
								break;
							}
						}
						
						C_Part::C_Damage::Transfer trans;
						trans.m_pNode = pTransferNode;
						trans.m_Percentage = pTrans->m_Percentage;
						pComponent->m_pNode->m_Damage.m_Transfer.PushBack(trans);
					}
				}

				if (!p_pVehicle->m_Constraints || !g_debugSetupConstraints)
					continue;

				int constraintCount = p_pVehicle->m_Constraints->m_Constraints.count;
				for (int iConstraint = 0; iConstraint < p_pVehicle->m_Constraints->m_Constraints.count; iConstraint++)
				{
					if (!p_pVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part)
						continue;

					E_PartCode cPartCode = p_pVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part->m_PartCode;
					E_PartSubCode cPartSubCode = p_pVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part->m_PartSubCode;					

					if (p_pVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part->m_PartCode == pPart->m_Part->m_PartCode &&
						p_pVehicle->m_Constraints->m_Constraints[iConstraint]->m_Part->m_PartSubCode == pPart->m_Part->m_PartSubCode)
					{
						Vehicle::XtConstraintBase* pBase = p_pVehicle->m_Constraints->m_Constraints[iConstraint];
#ifdef WIP_PART_CONSTRAINTS
						if (pPart->m_Part->m_PartCode == E_PartCode_Bumper)
						{
							for (S32 iHack = 0; iHack < 3; iHack++)
							{
								bool xFlip = false;
								float xAxis = 0.0f;
								if (iHack == 1)
									xFlip = true;
								else if (iHack == 2)
									xAxis = -1.0f;

								SharedPhysics::C_Constraint* pConstraint = m_pPartList->CreateConstraint(	pComponent->m_pNode,
																												pBase,
																												p_pPhysicsScene->m_pGraphManager,
																												p_pCollisionBody, 
																												pComponent->m_pNode->m_pPhysicsBody,
																												p_VehicleMatrix, xFlip, xAxis);
								if (pConstraint)
								{
									pComponent->m_pNode->AddConstraint(pConstraint);
									pConstraint->SetScale(0.0f);
									//pConstraint->SetBodyAMassScale(0.f);
									p_pPhysicsScene->AddConstraint(pConstraint);
									m_pPartList->m_ConstrainedParts.PushBack(pComponent->m_pNode);
								}
							}
						}
						else
#endif
						{
							SharedPhysics::C_Constraint* pConstraint = m_pPartList->CreateConstraint(	pComponent->m_pNode,
																											pBase,
																											p_pPhysicsScene->m_pGraphManager,
																											p_pCollisionBody, 
																											pComponent->m_pNode->m_pPhysicsBody,
																											p_VehicleMatrix);
							if (pConstraint)
							{
								pComponent->m_pNode->AddConstraint(pConstraint);
								pConstraint->SetScale(0.0f);
								//pConstraint->SetBodyAMassScale(0.f);
								p_pPhysicsScene->AddDebugConstraint(pConstraint);
								m_pPartList->m_ConstrainedParts.PushBack(pComponent->m_pNode);
							}
						}
					}
				}
			}
		}		
	}

	C_Part* pFrontTyre = &m_pPartList->m_Parts[E_PartCode_Tyre].m_Nodes[E_PartSubCode_FrontLeft];
	C_Part* pFrontRim = &m_pPartList->m_Parts[E_PartCode_AlloyRim].m_Nodes[E_PartSubCode_FrontLeft];
	Maths::HVec3 frontTyreExtents = pFrontTyre->GetExtents();
	Maths::HVec3 frontRimExtents = pFrontRim->GetExtents();
	float fFrontTyreSize = (frontTyreExtents.Y() - frontRimExtents.Y()) * 0.5f;

	C_Part* pBackTyre = &m_pPartList->m_Parts[E_PartCode_Tyre].m_Nodes[E_PartSubCode_BackLeft];
	C_Part* pBackRim = &m_pPartList->m_Parts[E_PartCode_AlloyRim].m_Nodes[E_PartSubCode_BackLeft];
	Maths::HVec3 backTyreExtents = pBackTyre->GetExtents();
	Maths::HVec3 backRimExtents = pBackRim->GetExtents();
	float fBackTyreSize = (backTyreExtents.Y() - backRimExtents.Y()) * 0.5f;

	for (S32 iWheel = 0; iWheel < m_WheelNodes.Size(); iWheel++)
	{	
		if (iWheel == 0 || iWheel == 1)
			m_WheelNodes[iWheel].m_TyreSize = fFrontTyreSize;
		else
			m_WheelNodes[iWheel].m_TyreSize = fBackTyreSize;
	}
MEMORY_TRACKER_POP("graphics.vehicle.damage");
}

/*
===============
NAME: SaveDamageState

NOTE: Save damage state
===============
*/
void C_DamageLayer::SaveDamageState(C_DamageState* p_pState)
{
	if (!p_pState || (*m_pDamageVolume).Size() < 2)
		return;

	S32 partOffset = 8;
	S32 numParts = m_pPartList->m_ActiveParts.Size();// - 8;
	S32 numNodes = (*m_pDamageVolume)[1].GetWidth() * (*m_pDamageVolume)[1].GetLength();
	//First, fill nodes
	for (int iColumn = 0; iColumn < (*m_pDamageVolume)[1].GetWidth(); iColumn++)
	{
		for (int iRow = 0; iRow < (*m_pDamageVolume)[1].GetLength(); iRow++)
		{
			S32 index = iRow * (*m_pDamageVolume)[1].GetWidth() + iColumn;
			C_Node* pNode = (*m_pDamageVolume)[1].GetNode(iRow, iColumn);
		
			p_pState->m_State[index].m_DX = (DWORD)(Maths::Clamp((2.0f + (float)pNode->m_OffsetVector.X()) / 4.0f, 0.0f, 1.0f) * 32.0f);
			p_pState->m_State[index].m_DZ = (DWORD)(Maths::Clamp((2.0f + (float)pNode->m_OffsetVector.Z()) / 4.0f, 0.0f, 1.0f) * 64.0f);
			p_pState->m_State[index].m_Scratch	= (DWORD)(pNode->m_ScratchLevel * 7.0f);
			p_pState->m_State[index].m_Deform	= (DWORD)(pNode->m_DeformLevel * 7.0f);
	
			S32 partIndex = index + partOffset;

			if (partIndex >= numParts)
				continue;
			
			C_Part* pPartNode				= m_pPartList->m_ActiveParts[partIndex].m_pNode;
			p_pState->m_State[index].m_PartCode		= m_pPartList->m_ActiveParts[partIndex].m_PartCode;
			p_pState->m_State[index].m_PartSubCode	= m_pPartList->m_ActiveParts[partIndex].m_PartSubCode;
			p_pState->m_State[index].m_Hinged		= (DWORD)(pPartNode->m_Damage.m_HingeActive ? 1 : 0);
			p_pState->m_State[index].m_Detached		= (DWORD)(pPartNode->m_Damage.m_Detached ? 1 : 0);
			p_pState->m_State[index].m_Level		= (DWORD)(pPartNode->m_Damage.m_DamageLevel * 15.0f);
		}
	}
}

/*
===============
NAME: RestoreDamageState

NOTE: Restore damage state
===============
*/
void C_DamageLayer::RestoreDamageState(const C_DamageState* p_pState)
{
	if (!p_pState || (*m_pDamageVolume).Size() < 2)
		return;

	S32 partOffset = 8;
	S32 numParts = m_pPartList->m_ActiveParts.Size() - 8;
	//First, fill nodes
	for (int iColumn = 0; iColumn < (*m_pDamageVolume)[1].GetWidth(); iColumn++)
	{
		for (int iRow = 0; iRow < (*m_pDamageVolume)[1].GetLength(); iRow++)
		{
			S32 index = iRow * (*m_pDamageVolume)[1].GetWidth() + iColumn;
			C_Node* pNode0 = (*m_pDamageVolume)[0].GetNode(iRow, iColumn);
			C_Node* pNode1 = (*m_pDamageVolume)[1].GetNode(iRow, iColumn);
			C_Node* pNode2 = (*m_pDamageVolume)[2].GetNode(iRow, iColumn);
		
			float fx = (float)(p_pState->m_State[index].m_DX / 32.0f) * 4.0f - 2.0f;
			float fy = 0.0f;
			float fz = (float)(p_pState->m_State[index].m_DZ / 64.0f) * 4.0f - 2.0f;
			pNode0->m_OffsetVector = Maths::HVec3(fx, fy, fz);
			pNode0->m_Position = pNode0->m_OriginalPosition + pNode0->m_OffsetVector;
			pNode0->m_OffsetLength = pNode0->m_OffsetVector.Length();
			pNode1->m_OffsetVector = Maths::HVec3(fx, fy, fz);
			pNode1->m_Position = pNode1->m_OriginalPosition + pNode1->m_OffsetVector;
			pNode1->m_OffsetLength = pNode1->m_OffsetVector.Length();
			pNode2->m_OffsetVector = Maths::HVec3(fx, fy, fz);
			pNode2->m_Position = pNode2->m_OriginalPosition + pNode2->m_OffsetVector;
			pNode2->m_OffsetLength = pNode2->m_OffsetVector.Length();
			float fs = (float)p_pState->m_State[index].m_Scratch / 7.0f;
			pNode0->m_ScratchLevel = fs;
			pNode1->m_ScratchLevel = fs;
			pNode2->m_ScratchLevel = fs;
			float fd = (float)p_pState->m_State[index].m_Deform / 7.0f;
			pNode0->m_DeformLevel = fd;
			pNode1->m_DeformLevel = fd;
			pNode2->m_DeformLevel = fd;
			
			if (index - partOffset >= numParts)
				continue;

			C_Part* pPartNode = m_pPartList->m_ActiveParts[index].m_pNode;

			if (p_pState->m_State[index].m_PartCode != m_pPartList->m_ActiveParts[index].m_PartCode)
				continue;
			if (p_pState->m_State[index].m_PartSubCode != m_pPartList->m_ActiveParts[index].m_PartSubCode)
				continue;

			pPartNode->m_Damage.m_DamageLevel = (float)p_pState->m_State[index].m_Level / 15.0f;
			pPartNode->HandleDamage(pPartNode->m_Damage.m_DamageLevel);				
			UpdateDamageBehaviour(NULL, pPartNode, true);

			if (p_pState->m_State[index].m_Hinged == 1)
				pPartNode->ActivateConstraints();
			else
				pPartNode->DeactivateConstraints();

			if (p_pState->m_State[index].m_Detached == 1)
				pPartNode->Detach(Maths::HVec3(0.0f, 0.0f, 0.0f));
			else 
				pPartNode->Reattach();
		}
	}

	SetUpdateVolumeTexture(true);//m_UpdateVolumeTexture = true;
	SetUpdateInfoTexture(true);//m_UpdateInfoTexture = true;

#ifdef PLATFORM_PS3
	m_pSpuDamageContext->forceRun = true;
#endif

	UpdateWheelDamage(NULL);
}

Debug::Bool g_debugDisplayDebugColour("Vehicle.Damage.Debug.RenderNodesAsDebugColour", true);
Debug::Bool g_debugDamageDrawCachedEvents("Vehicle.Damage.Debug.RenderCachedImpactEvents", true);
Debug::Bool g_debugDamageDrawLattice("Vehicle.Damage.Debug.RenderNodeLattice", true);
Debug::Int g_debugLastImportantImpactEvents("Vehicle.Damage.Debug.NumOfLastImportantEvents", 5, Debug::Range(0, 20), Debug::Step(1));
Debug::Int g_debugLastImportantPlanes("Vehicle.Damage.Debug.NumOfLastImportantPlanes", 3, Debug::Range(0, 20), Debug::Step(1));
Debug::Bool g_debugDrawCollisionVertices("Vehicle.Damage.Debug.DrawCollisionVertices", false);

/*
===============
NAME: DebugDraw

NOTE: Draws debug information
===============
*/
void C_DamageLayer::DebugDraw()
{
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();

	Maths::HVec4 ax(m_VehicleInfo.m_Matrix.AX().X(), m_VehicleInfo.m_Matrix.AX().Y(), m_VehicleInfo.m_Matrix.AX().Z(), 1.0f);
	Maths::HVec4 ay(m_VehicleInfo.m_Matrix.AY().X(), m_VehicleInfo.m_Matrix.AY().Y(), m_VehicleInfo.m_Matrix.AY().Z(), 1.0f);
	Maths::HVec4 az(m_VehicleInfo.m_Matrix.AZ().X(), m_VehicleInfo.m_Matrix.AZ().Y(), m_VehicleInfo.m_Matrix.AZ().Z(), 1.0f);
	Maths::HVec4 aw(m_VehicleInfo.m_Matrix.AW().X(), m_VehicleInfo.m_Matrix.AW().Y(), m_VehicleInfo.m_Matrix.AW().Z(), 1.0f);
	Maths::HMat44 mat(ax, ay, az, aw);

	if (g_debugDrawCollisionVertices)
	{
		for (S32 iV = 0; iV < m_CollisionShapeVertices.Size(); iV++)
		{
			Maths::HVec3 nodePos = m_CollisionShapeVertices[iV].m_Original * mat;
			pDebugRenderer->Draw3dCircle(nodePos, Maths::HVec3(0.0f, 1.0f, 0.0f), 0.05f, Maths::RGBA(0, 255, 0));
			nodePos = (m_CollisionShapeVertices[iV].m_pNode->m_OffsetVector + m_CollisionShapeVertices[iV].m_Original) * mat;
			pDebugRenderer->Draw3dCircle(nodePos, Maths::HVec3(0.0f, 1.0f, 0.0f), 0.05f, Maths::RGBA(255, 0, 0));
		}
	}
		
	if (g_debugDamageDrawLattice)
	{
		//Render nodes
		for (int iLayer = 0; iLayer < (*m_pDamageVolume).Size(); iLayer++)
		{	
			for (int iLength = 0; iLength < (*m_pDamageVolume)[iLayer].GetLength(); iLength++)
			{
				for (int iWidth = 0; iWidth < (*m_pDamageVolume)[iLayer].GetWidth(); iWidth++)
				{
					int index = (iLength * (*m_pDamageVolume)[iLayer].GetWidth()) + iWidth;

					C_Node* pNode = &(*m_pDamageVolume)[iLayer].m_Nodes[index];

					Maths::RGBA colour(pNode->m_DeformLevel, 1.0f - pNode->m_DeformLevel, 0.0f);
					
					if (pNode->m_Active && g_debugModifyVolume)
						colour = Maths::RGBA(0.0f, 0.0f, 1.0f);
					else if (g_debugDisplayDebugColour)
						colour = pNode->m_DebugColour;

					float col = 0.0f;
					if (pNode->m_MaxDisplacement.Length() > 0.0f)
						col = pNode->m_OffsetVector.Length() / pNode->m_MaxDisplacement.Length();
					colour = Maths::RGBA(col, 1.0f - col, 0.0f);
					float radius = 0.06f;

					Maths::HVec3 nodePos = pNode->m_Position * mat;
					pDebugRenderer->Draw3dCircle(nodePos, Maths::HVec3(0.0f, 1.0f, 0.0f), radius, colour);
				}
			}
		}
	}

	if (g_debugDamageDrawCachedEvents)
	{
		for (int iEvent = 0; iEvent < m_CachedImpactEvents.Size(); iEvent++)
		{
			Maths::RGBA colour(255, 255, 0);

			S32 lag = m_CachedImpactEvents.Size() - (S32)iEvent;
			if (lag <= g_debugLastImportantImpactEvents)
				colour = Maths::RGBA(0, 255, 0);

			for (int iPoint = 0; iPoint < m_CachedImpactEvents[iEvent].m_CollisionInfo.m_ContactPoints.Size(); iPoint++)
			{
				Maths::HVec3 point = m_CachedImpactEvents[iEvent].m_CollisionInfo.m_ContactPoints[iPoint].m_LocalContactPoint;
				Maths::HVec3 pos = point * mat;
				
				pDebugRenderer->Draw3dCross(pos, 0.1f, colour);				
			}

			if (lag <= g_debugLastImportantPlanes)
			{
				Maths::HVec3 pos = m_CachedImpactEvents[iEvent].m_LocalPosition * mat;
				Maths::HVec3 pos2 = m_CachedImpactEvents[iEvent].m_GlobalPosition + m_CachedImpactEvents[iEvent].m_GlobalNormal;
				pos2 = pos2 * m_CachedImpactEvents[iEvent].m_Matrix.InverseRotTrans();
				pos2 = pos2 * mat;
				pDebugRenderer->Draw3dLine(pos, pos2, Maths::RGBA(0, 0, 255));
			}
		}
	}

}

void XtFixedJoint::DebugDraw(Maths::HMat43 p_Matrix)
{
	//DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	//
	//Maths::HVec3 pos1(m_AnchorPoint1.vx, m_AnchorPoint1.vy, m_AnchorPoint1.vz);
	//pos1 = pos1 * p_Matrix;
	//Maths::HVec3 pos2(m_AnchorPoint2.vx, m_AnchorPoint2.vy, m_AnchorPoint2.vz);
	//pos2 = pos2 * p_Matrix;

	//pDebugRenderer->Draw3dStar(pos1, 0.1f, Maths::RGBA(0, 0, 255, 255));
	//pDebugRenderer->Draw3dStar(pos2, 0.1f, Maths::RGBA(0, 0, 255, 255));
	//pDebugRenderer->Draw3dLine(pos1, pos2, Maths::RGBA(0, 255, 0, 255));
}

void XtSphericalJoint::DebugDraw(Maths::HMat43 p_Matrix)
{
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	
	Maths::HVec3 pos(m_Position.vx, m_Position.vy, m_Position.vz);
	pos = pos * p_Matrix;

	pDebugRenderer->Draw3dStar(pos, 0.1f, Maths::RGBA(0, 0, 255, 255));
}

void XtRevoluteJoint::DebugDraw(Maths::HMat43 p_Matrix)
{
	//DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	//
	//Maths::HVec3 pos1(m_AnchorPoint1.vx, m_AnchorPoint1.vy, m_AnchorPoint1.vz);
	//pos1 = pos1 * p_Matrix;
	//Maths::HVec3 pos2(m_AnchorPoint2.vx, m_AnchorPoint2.vy, m_AnchorPoint2.vz);
	//pos2 = pos2 * p_Matrix;

	//Maths::HVec3 vec1(m_JointAxis1.vx, m_JointAxis1.vy, m_JointAxis1.vz);
	//pos1 = vec1 * p_Matrix;
	//Maths::HVec3 vec2(m_JointAxis2.vx, m_JointAxis2.vy, m_JointAxis2.vz);
	//pos2 = vec2 * p_Matrix;

	//pDebugRenderer->Draw3dStar(pos1, 0.1f, Maths::RGBA(0, 0, 255, 255));
	//pDebugRenderer->Draw3dStar(pos2, 0.1f, Maths::RGBA(0, 0, 255, 255));
	//pDebugRenderer->Draw3dLine(pos1, pos2, Maths::RGBA(0, 255, 0, 255));
	//pDebugRenderer->Draw3dArrow(pos1, vec1, vec1.Length(), 0.1f, Maths::RGBA(255, 255, 0, 255));
	//pDebugRenderer->Draw3dArrow(pos2, vec2, vec2.Length(), 0.1f, Maths::RGBA(255, 255, 0, 255));
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	
	Maths::HVec3 pos(m_Position.vx, m_Position.vy, m_Position.vz);
	pos = pos * p_Matrix;

	pDebugRenderer->Draw3dStar(pos, 0.1f, Maths::RGBA(0, 0, 255, 255));
}

void XtRevoluteJointLimit::DebugDraw(Maths::HMat43 p_Matrix)
{
	//DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	//	
	//Maths::HVec3 pos1(m_AnchorPoint1.vx, m_AnchorPoint1.vy, m_AnchorPoint1.vz);
	//pos1 = pos1 * p_Matrix;
	//Maths::HVec3 pos2(m_AnchorPoint2.vx, m_AnchorPoint2.vy, m_AnchorPoint2.vz);
	//pos2 = pos2 * p_Matrix;

	//Maths::HVec3 vec1(m_JointAxis1.vx, m_JointAxis1.vy, m_JointAxis1.vz);
	//pos1 = vec1 * p_Matrix;
	//Maths::HVec3 vec2(m_JointAxis2.vx, m_JointAxis2.vy, m_JointAxis2.vz);
	//pos2 = vec2 * p_Matrix;

	//Util::C_String str1;
	//str1.Format("Low = %.2f", (float)m_LowJointLimit);
	//Util::C_String str2;
	//str2.Format("High = %.2f", (float)m_HighJointLimit);
	//pDebugRenderer->Draw3dStar(pos1, 0.1f, Maths::RGBA(0, 0, 255, 255));
	//pDebugRenderer->Draw3dStar(pos2, 0.1f, Maths::RGBA(0, 0, 255, 255));
	//pDebugRenderer->Draw3dLine(pos1, pos2, Maths::RGBA(0, 255, 0, 255));
	//pDebugRenderer->Draw3dArrow(pos1, vec1, vec1.Length(), 0.1f, Maths::RGBA(255, 255, 0, 255));
	//pDebugRenderer->Draw3dArrow(pos2, vec2, vec2.Length(), 0.1f, Maths::RGBA(255, 255, 0, 255));
	//pDebugRenderer->Draw3dText(pos1, str1.Chars());
	//pDebugRenderer->Draw3dText(pos2, str2.Chars());
	DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	
	Maths::HVec3 pos(m_Position.vx, m_Position.vy, m_Position.vz);
	pos = pos * p_Matrix;
	
	Maths::HVec3 axis(m_Axis.vx, m_Axis.vy, m_Axis.vz);
	axis = axis * p_Matrix.Rotation();
	pDebugRenderer->Draw3dStar(pos, 0.1f, Maths::RGBA(0, 0, 255, 255));
	pDebugRenderer->Draw3dArrow(pos, axis, 0.5f, 0.1f, Maths::RGBA(0, 255, 0));
}

void XtPrismaticJoint::DebugDraw(Maths::HMat43 p_Matrix)
{
	//DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	//
	//Maths::HVec3 vec1(m_JointAxis1.vx, m_JointAxis1.vy, m_JointAxis1.vz);
	//vec1 = vec1 * p_Matrix;
	//Maths::HVec3 vec2(m_JointAxis2.vx, m_JointAxis2.vy, m_JointAxis2.vz);
	//vec2 = vec2 * p_Matrix;

	//pDebugRenderer->Draw3dArrow(pos1, vec1, vec1.Length(), 0.1f, Maths::RGBA(255, 255, 0, 255));
	//pDebugRenderer->Draw3dArrow(pos2, vec2, vec2.Length(), 0.1f, Maths::RGBA(255, 255, 0, 255));
}

void XtPrismaticJointLimit::DebugDraw(Maths::HMat43 p_Matrix)
{
	//DbUtil::I_DebugRender *pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Deferred();
	//	
	//Maths::HVec3 pos1(m_AnchorPoint1.vx, m_AnchorPoint1.vy, m_AnchorPoint1.vz);
	//pos1 = pos1 * p_Matrix;
	//Maths::HVec3 pos2(m_AnchorPoint2.vx, m_AnchorPoint2.vy, m_AnchorPoint2.vz);
	//pos2 = pos2 * p_Matrix;

	//Maths::HVec3 vec1(m_JointAxis1.vx, m_JointAxis1.vy, m_JointAxis1.vz);
	//pos1 = vec1 * p_Matrix;
	//Maths::HVec3 vec2(m_JointAxis2.vx, m_JointAxis2.vy, m_JointAxis2.vz);
	//pos2 = vec2 * p_Matrix;

	//Util::C_String str1;
	//str1.Format(L"Low = %.2f", (float)m_LowJointLimit);
	//Util::C_String str2;
	//str2.Format(L"High = %.2f", (float)m_HighJointLimit);
	//pDebugRenderer->Draw3dArrow(pos1, vec1, vec1.Length(), 0.1f, Maths::RGBA(255, 255, 0, 255));
	//pDebugRenderer->Draw3dArrow(pos2, vec2, vec2.Length(), 0.1f, Maths::RGBA(255, 255, 0, 255));
	//pDebugRenderer->Draw3dText(pos1, str1.Chars());
	//pDebugRenderer->Draw3dText(pos2, str2.Chars());
}


//Texture Resources

U32 C_DamageLayer::ms_NumTextureResourcesVehicles = 0;
Display::C_Texture3D C_DamageLayer::ms_TextureVolumeResources[NUM_DAMAGE_TEXTURES][20];
Display::C_Texture2D C_DamageLayer::ms_TextureInfoResources[NUM_DAMAGE_TEXTURES][20];

bool C_DamageLayer::CreateTextureResources( U32 numVehicles )
{
	ms_NumTextureResourcesVehicles = numVehicles;

	MEMORY_TRACKER_PUSH("graphics.vehicle.damage");
	#if !defined(PLATFORM_PS3)

	for (U32 j = 0; j < ms_NumTextureResourcesVehicles; j++)
	{
		for (int iTex = 0; iTex < NUM_DAMAGE_TEXTURES; iTex++)
		{
#ifdef BLUR2PC_DAMAGE_VOLUME_F32
			if (!Display::Device()->CreateTexture3D(g_debugDamageTextureWidth, g_debugDamageTextureHeight, g_debugDamageTextureDepth, 1, Display::E_Format_Linear, Display::E_Format_A32B32G32R32F, &ms_TextureVolumeResources[iTex][j]))
#else
			if (!Display::Device()->CreateTexture3D(g_debugDamageTextureWidth, g_debugDamageTextureHeight, g_debugDamageTextureDepth, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &ms_TextureVolumeResources[iTex][j]))
#endif
			{
				MEMORY_TRACKER_POP("graphics.vehicle.damage");
				return false;
			}
		}

		for (int iTex = 0; iTex < NUM_DAMAGE_TEXTURES; iTex++)
		{
			#ifdef _DAMAGE_USE_F32
				if (!Display::Device()->CreateTexture2D(g_debugDamageTextureWidth, g_debugDamageTextureHeight, 1, Display::E_Format_Linear, Display::E_Format_A32B32G32R32F, &ms_TextureInfoResources[iTex][j]))
			#else
				if (!Display::Device()->CreateTexture2D(g_debugDamageTextureWidth, g_debugDamageTextureHeight, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &ms_TextureInfoResources[iTex][j]))
			#endif
			{
				MEMORY_TRACKER_POP("graphics.vehicle.damage");
				return false;
			}
		}
	}

	#endif

	MEMORY_TRACKER_POP("graphics.vehicle.damage");
	return true;
}

void C_DamageLayer::DestroyTextureResources()
{
	for (U32 j = 0; j < ms_NumTextureResourcesVehicles; j++)
	{
		for (int iTex = 0; iTex < NUM_DAMAGE_TEXTURES; iTex++)
		{
			Display::Device()->DestroyTexture3D(&ms_TextureVolumeResources[iTex][j]);
		}
		for (int iTex = 0; iTex < NUM_DAMAGE_TEXTURES; iTex++)
		{
			Display::Device()->DestroyTexture3D(&ms_TextureInfoResources[iTex][j]);
		}
	}

	ms_NumTextureResourcesVehicles = 0;
}

} //namespace Vehicle

