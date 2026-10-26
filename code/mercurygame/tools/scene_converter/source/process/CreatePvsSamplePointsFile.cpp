#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <process/CreatePvsSamplePointsFile.h>
#include <Polyflags.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

class PvsPolyInfo : public Pvs::Build::PvsPolyInfo
{
public:
	PvsPolyInfo( const Objectflags::C_Config* pObjectFlagConfig, const Polyflags::C_Config* pPolyflagConfig ) : m_pObjectflagConfig( pObjectFlagConfig ), m_pPolyflagConfig( pPolyflagConfig )
	{
		SetNumSamplePointsY(2);
		SetHeightAboveTriangle(0.5f, 3.f);
	}

	virtual bool GeneratePvs(const Aftk::C_Polygon& poly) const
	{		
		Objectflags::C_Objectflags objectFlags( *m_pObjectflagConfig, poly.GetMeshNodePtr() );

		if( objectFlags.GetValue(E_Objectflag_NoPVSSamples))
			return false;



		Polyflags::C_Polyflags polyFlags( *m_pPolyflagConfig, &poly );

		// NoLightSamples has priority
		if (polyFlags.GetValue(E_Polyflag_NoPVSSamples))
			return 0;

		// if collideable or forcelightsamples is set
		if (polyFlags.GetValue(E_Polyflag_CollideFlags) || polyFlags.GetValue(E_Polyflag_ForcePVSSamples))
			return 1;
		else
			return 0;

		//return Polyflags::C_Polyflags( *m_pConfig, &poly ).GetValue( E_Polyflag_CollideFlags ) != 0;
	}

private:
	const Polyflags::C_Config* m_pPolyflagConfig;
	const Objectflags::C_Config* m_pObjectflagConfig;
};

bool C_ProcessCreatePvsSamplePointsFile::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progressSimple(pProgress, GetName());

	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pInputSceneDescription = (const C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pInputSceneDescription);

	bool success = true;

	PvsPolyInfo pvsPolyInfo(&pInputConversionOptions->GetObjectflagConfig(), &pInputConversionOptions->GetPolyflagConfig());
	pvsPolyInfo.SetGridSizeXZ( pInputConversionOptions->GetPvsSampleSpacing() );

	for(S32 i = 0; i < pInputSceneDescription->GetStaticNodes().Size(); ++i)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = pInputSceneDescription->GetStaticNodes()[i];
		
		if (AftkHelpers::GetCollisionType(pAftkSceneNode) != AftkHelpers::E_CollisionType_None)
			continue;

		if(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			m_pvsData.AddMesh((const Aftk::C_SceneMeshNode&)*pAftkSceneNode, pAftkSceneNode->GetWorldMatrix(), pvsPolyInfo);
		}
		else if (pAftkSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		{
			const Aftk::C_SceneMeshNode* pSource = ((Aftk::C_SceneMeshRemappedInstanceNode*) pAftkSceneNode)->GetSourceNodePtr();
			m_pvsData.AddMesh(*pSource, pAftkSceneNode->GetWorldMatrix(), pvsPolyInfo);
		}
		else if (pAftkSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
		{
			const Aftk::C_SceneNode* pSource = ((Aftk::C_SceneInstanceNode*) pAftkSceneNode)->GetSourceNodePtr();

			if (pSource->GetType() == Aftk::E_ObjType_SceneMesh)
				m_pvsData.AddMesh((const Aftk::C_SceneMeshNode&)*pSource, pAftkSceneNode->GetWorldMatrix(), pvsPolyInfo);
		}
		AddPVSZones(pAftkSceneNode);
	}
	
	m_pvsData.SetZoneSize( pInputConversionOptions->GetPvsZoneSize(), pInputConversionOptions->GetPvsZoneHeight() );	
	m_pvsData.Process();

	//Create vis directory
	
	Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(pInputConversionOptions->GetVisInputFilename()));

	//Write file to Vis tool
	m_pvsData.WriteDataForPvsTool(pInputConversionOptions->GetVisInputFilename());

	//Write file to ReadPVSData
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(pInputConversionOptions->GetPVSDataFilename(), Sys::E_FileAccess_Write);
	if (pFile)
	{
		Arch::BinaryOutputArchive ar(*pFile, true);
		ar << m_pvsData;
		delete pFile;
	}


	return success;
}

void C_ProcessCreatePvsSamplePointsFile::AddPVSZones(const Aftk::C_SceneNode* pAftkNode)
{
	AftkUt::C_NodeFlagParser parser( *pAftkNode );
	if (parser.GetFlagGroupCategory() == "System" && parser.GetFlagGroupName() == "PVSZone")
    {
		Maths::HMat44 mtx;
		Aftk::C_Bounds bounds = pAftkNode->GetLocalBounds();
		Aftk::TemplateToNightmareMatrix44(mtx, pAftkNode->GetWorldMatrix());
		Maths::HVec3 vScale = (Aftk::TemplateToNightmare(bounds.GetMax()) - Aftk::TemplateToNightmare(bounds.GetMin()))*0.5f;
		Maths::HVec3 vOffset = Aftk::TemplateToNightmare(bounds.GetCentre());

		Maths::HMat44 boundsmtx(
			Maths::HVec4(vScale.X(), 0.f, 0.f, 0.f),
			Maths::HVec4(0.f, vScale.Y(), 0.f, 0.f),
			Maths::HVec4(0.f, 0.f, vScale.Z(), 0.f),
			Maths::HVec4(vOffset.X(), vOffset.Y(), vOffset.Z(), 1.f));

		mtx = boundsmtx * mtx;

		MathsStorage::Matrix44 zoneMtx = {	mtx[0].X(), mtx[0].Y(), mtx[0].Z(), mtx[0].W(),
											mtx[1].X(), mtx[1].Y(), mtx[1].Z(), mtx[1].W(),
											mtx[2].X(), mtx[2].Y(), mtx[2].Z(), mtx[2].W(),
											mtx[3].X(), mtx[3].Y(), mtx[3].Z(), mtx[3].W()
										  };
		m_pvsData.AddZone(zoneMtx);
	}
}

const Util::C_String C_ProcessCreatePvsSamplePointsFile::GetName() const
{
	return "C_ProcessCreatePvsSamplePointsFile";
}