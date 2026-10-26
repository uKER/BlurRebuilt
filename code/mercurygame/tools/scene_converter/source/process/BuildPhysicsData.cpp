#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <data/PhysicsData.h>
#include "converter_shared/PhysicsDataConverter.h"


#include <process/BuildPhysicsData.h>

#include <Polyflags.h>

C_ProcessBuildPhysicsData::C_ProcessBuildPhysicsData()
{
}

const Util::C_String C_ProcessBuildPhysicsData::GetName() const
{
	return Util::C_String("C_ProcessBuildPhysicsData");
}

class C_MyPolyInform : public Gravity::C_PolyInform
{
public:
	C_MyPolyInform( const Polyflags::C_Config* pPolyflagConfig, const bool forceAllSolid, const U32 forceExtraCollideMeshFlagsValue ) : m_pConfig( pPolyflagConfig ), m_ForceAllSolid( forceAllSolid ), m_ForceExtraCollideMeshFlagsValue(forceExtraCollideMeshFlagsValue) {}
	~C_MyPolyInform() {}

	U32 GetCollisionFlags(const Aftk::C_Polygon& poly) 
	{
		if(m_ForceAllSolid)
			return 0xffffffff;
		
		U32 value = Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_CollideFlags );

		if( value != 0 )
			value |= m_ForceExtraCollideMeshFlagsValue;

		return value; 
	}
	U32 GetPhysicsMaterial(const Aftk::C_Polygon& poly) { return Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_PhysicsMaterial ); }
	U32 GetEffectMaterial(const Aftk::C_Polygon& poly) 
	{ 
		U32 effectMaterial = Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_EffectMaterial );
		return effectMaterial;
	}
	U32 GetUserData(const Aftk::C_Polygon& poly) { return Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_CollideUserData ); }

private:
	const Polyflags::C_Config* m_pConfig;
	const bool m_ForceAllSolid;
	const U32 m_ForceExtraCollideMeshFlagsValue;
};

class C_MyNodeInform : public Gravity::C_NodeInform
{
public:		
	C_MyNodeInform( const Objectflags::C_Config* pObjectflagConfig, const Polyflags::C_Config* pPolyflagConfig )
		: m_pObjectflagConfig( pObjectflagConfig )
		, m_pPolyflagConfig( pPolyflagConfig )
	{	
	}

	bool IsWater(const Aftk::C_SceneNode *pSceneNode) const
	{
		if(Objectflags::C_Objectflags( *m_pObjectflagConfig, pSceneNode ).GetValue( E_Objectflag_WaterMesh ))
		{
			return true;
		}
		return false;
	}

	U32 GetEffectMaterial(const Aftk::C_SceneNode *pSceneNode) const
	{
		U32 effectMaterial = Polyflags::C_Polyflags( *m_pPolyflagConfig, pSceneNode ).GetValue( E_Polyflag_EffectMaterial );
		return effectMaterial;
	}


private:
	const Polyflags::C_Config* m_pPolyflagConfig;
	const Objectflags::C_Config* m_pObjectflagConfig;
};

bool C_ProcessBuildPhysicsData::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progress( pProgress, GetName() );

	

	// make the progress bar do something
	progress.SetProgress( 1.0f );

	const C_DataConversionOptions* pInputConversionOptions = (const C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pInputSceneDescription = (const C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pInputSceneDescription);

	C_DataPhysicsData* pOutputPhysicsData = (C_DataPhysicsData*)GetOutput( E_GraphData_PhysicsData );
	assert(pOutputPhysicsData);

	U32 forceExtraCollideMeshFlagsValue = 0;
	if( pInputConversionOptions->ForceExtraCollideMeshFlags() )
	{
		forceExtraCollideMeshFlagsValue = pInputConversionOptions->GetToolConfig().GetForceExtraCollideMeshFlagsValue();
	}

	U32 fluidCollisionFlag = pInputConversionOptions->GetToolConfig().GetFluidCollisionFlag();

	C_MyPolyInform* pNewPolyInform = new C_MyPolyInform( &pInputConversionOptions->GetPolyflagConfig(), pInputConversionOptions->ForceAllSolid(), forceExtraCollideMeshFlagsValue );
	pOutputPhysicsData->SetPolyInform( pNewPolyInform );

	C_MyNodeInform* pNewNodeInform = new C_MyNodeInform( &pInputConversionOptions->GetObjectflagConfig(), &pInputConversionOptions->GetPolyflagConfig() );
	pOutputPhysicsData->SetNodeInform( pNewNodeInform );

	Gravity::C_Converter* pMainScene = pOutputPhysicsData->GetMainScene();
	pMainScene->SetMaxConvexHullTriangles( pInputConversionOptions->GetToolConfig().GetMaxConvexHullTriangles() );
	pMainScene->SetFluidCollisionFlag(fluidCollisionFlag);

	ModelData::C_AftkNodeList staticNodes = pInputSceneDescription->GetStaticNodes();
	pMainScene->AddStaticNodes( staticNodes );		
	
	// add route nodes to route scene
	const S32 routeCount = pInputSceneDescription->GetRouteCount();
	for(S32 routeNo=0; routeNo<routeCount; routeNo++)
	{
		Gravity::C_Converter* pRouteScene = pOutputPhysicsData->NewRouteScene();
		pRouteScene->SetMaxConvexHullTriangles( pInputConversionOptions->GetToolConfig().GetMaxConvexHullTriangles() );
		pRouteScene->SetFluidCollisionFlag(fluidCollisionFlag);
		ModelData::C_AftkNodeList routeNodes = pInputSceneDescription->GetRouteNodes( routeNo );
		pRouteScene->AddStaticNodes( routeNodes );	
	}

	ModelData::C_ModelData& modelData = const_cast<ModelData::C_ModelData&>(pInputSceneDescription->GetModelData());

	for( ModelData::C_ModelIterator iModel( &modelData ); (const ModelData::C_Model*)iModel; ++iModel )
	{
		ModelData::C_Model* pModel = iModel;
		for( ModelData::C_ModelElementIterator iModelElement( pModel ); (const ModelData::C_ModelElement*)iModelElement; ++iModelElement )
		{
			ModelData::C_ModelElement* pModelElement = iModelElement;
			
			LOGDEBUG( Tools::g_log, " BuildPhysicsData Element %s:%s", pModel->GetName().Chars(), pModelElement->GetName().Chars() );

			const U32 physicsShapeId = pMainScene->AddDynamicShape( pModelElement->GetAftkNodeList(), pModelElement->GetMatrix() );
			pModelElement->SetPhysicsShapeId( physicsShapeId );
		}
	}

	const S32 constraintCount = modelData.GetConstraintCount();
	for(S32 constraintNo=0; constraintNo<constraintCount; constraintNo++)
	{
		ModelData::C_Constraint& constraint = modelData.GetConstraint(constraintNo);
		// TODO: call AddToConstraint
		constraint.m_PhysicsConstraintId = pMainScene->AddToConstraint( constraint.m_pAftkSceneNode );
	}
	bool success = true;
	
	// only 2 percent as it shouldn't have taken long up to this point
	progress.SetProgress( 2.0f );

	success = pMainScene->Process() && success;

	for(S32 routeNo=0; routeNo<routeCount; routeNo++)
	{
		Gravity::C_Converter* pRouteScene = pOutputPhysicsData->GetRouteScene(routeNo);
		success = pRouteScene->Process() && success;
	}

	return success;
}

