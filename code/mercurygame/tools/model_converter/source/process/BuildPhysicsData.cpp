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
	C_MyPolyInform( const Polyflags::C_Config* pPolyflagConfig ) : m_pConfig( pPolyflagConfig ) {}
	~C_MyPolyInform() {}

	U32 GetCollisionFlags(const Aftk::C_Polygon& poly) { return Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_CollideFlags ); }
	U32 GetPhysicsMaterial(const Aftk::C_Polygon& poly) { return Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_PhysicsMaterial ); }
	U32 GetEffectMaterial(const Aftk::C_Polygon& poly) { return Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_EffectMaterial ); }
	U32 GetUserData(const Aftk::C_Polygon& poly) { return Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_CollideUserData ); }

private:
	const Polyflags::C_Config* m_pConfig;
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
		//if(Objectflags::C_Objectflags( *m_pObjectflagConfig, pSceneNode ).GetValue( E_Objectflag_WaterMesh ))
		//{
		//	return true;
		//}
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

	const C_DataConversionOptions* pInputConversionOptions = (const C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pInputSceneDescription = (const C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pInputSceneDescription);

	C_DataPhysicsData* pOutputPhysicsData = (C_DataPhysicsData*)GetOutput( E_GraphData_PhysicsData );
	assert(pOutputPhysicsData);

	C_MyPolyInform* pNewPolyInform = new C_MyPolyInform( &pInputConversionOptions->GetPolyflagConfig() );
	pOutputPhysicsData->SetPolyInform( pNewPolyInform );

	C_MyNodeInform* pNewNodeInform = new C_MyNodeInform( &pInputConversionOptions->GetObjectflagConfig(), &pInputConversionOptions->GetPolyflagConfig() );
	pOutputPhysicsData->SetNodeInform( pNewNodeInform );

	Gravity::C_Converter* pConverter = pOutputPhysicsData->NewExportScene( pInputConversionOptions->RotateY180() );
	pConverter->SetMaxConvexHullTriangles( pInputConversionOptions->GetToolConfig().GetMaxConvexHullTriangles() );

	ModelData::C_ModelData& modelData = const_cast<ModelData::C_ModelData&>(pInputSceneDescription->GetModelData());

	for( ModelData::C_ModelIterator iModel( &modelData ); iModel; ++iModel )
	{
		ModelData::C_Model* pModel = iModel;
		for( ModelData::C_ModelElementIterator iModelElement( pModel ); iModelElement; ++iModelElement )
		{
			ModelData::C_ModelElement* pModelElement = iModelElement;
			
			LOGDEBUG( Tools::g_log, "Element %s:%s", pModel->GetName().Chars(), pModelElement->GetName().Chars() );
			ModelData::DumpAfktNodeList( pModelElement->GetAftkNodeList(), ":" );

			const U32 physicsShapeId = pConverter->AddDynamicShape( pModelElement->GetAftkNodeList(), pModelElement->GetMatrix() );
			pModelElement->SetPhysicsShapeId( physicsShapeId );
		}
	}

	const S32 constraintCount = modelData.GetConstraintCount();
	for(S32 constraintNo=0; constraintNo<constraintCount; constraintNo++)
	{
		ModelData::C_Constraint& constraint = modelData.GetConstraint(constraintNo);
		// TODO: call AddToConstraint
		constraint.m_PhysicsConstraintId = pConverter->AddToConstraint( constraint.m_pAftkSceneNode );
	}

	bool success = true;
	
	success = pConverter->Process() && success;

	return success;
}

