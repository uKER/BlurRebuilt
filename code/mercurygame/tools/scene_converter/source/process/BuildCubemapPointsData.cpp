#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <data/CubemapPointsData.h>
#include <data/AftkSceneData.h>
#include <process/BuildCubemapPointsData.h>
#include <aftkut\AftkUtNodeFlagParser.h>
#include <data\AftkSceneData.h>

#include <Polyflags.h>

C_ProcessBuildCubemapPoints::C_ProcessBuildCubemapPoints()
{
}

const Util::C_String C_ProcessBuildCubemapPoints::GetName() const
{
	return Util::C_String("C_ProcessBuildCubemapPoints");
}

/// for a given cubemap location, find all the zones associated with it
/// (association is done by making the zone a child of the cubemap location in maya)
void C_ProcessBuildCubemapPoints::ProcessCubemapZones(C_DataCubemapPoints *pData, Aftk::C_SceneNode *pNode, int iCubemap)
{
	for (U32 i=0; i<pNode->GetNumChildren(); i++)
	{
		Aftk::C_SceneNode *pChild = pNode->GetChildPtr(i);
		AftkUt::C_NodeFlagParser parser( *pChild );
		if (parser.GetFlagGroupCategory() == "System" && parser.GetFlagGroupName() == "CubemapZone")
		{
			// add a cubemap zone
			Maths::HMat44 mtx;
			Aftk::C_Bounds bounds = pChild->GetLocalBounds();
			Aftk::TemplateToNightmareMatrix44(mtx, pChild->GetWorldMatrix());
			Maths::HVec3 vScale = (Aftk::TemplateToNightmare(bounds.GetMax()) - Aftk::TemplateToNightmare(bounds.GetMin()))*0.5f;
			Maths::HVec3 vOffset = Aftk::TemplateToNightmare(bounds.GetCentre());
			Maths::HMat44 boundsmtx(
				Maths::HVec4(vScale.X(), 0.f, 0.f, 0.f),
				Maths::HVec4(0.f, vScale.Y(), 0.f, 0.f),
				Maths::HVec4(0.f, 0.f, vScale.Z(), 0.f),
				Maths::HVec4(vOffset.X(), vOffset.Y(), vOffset.Z(), 1.f));

			mtx = boundsmtx * mtx;

			pData->AddCubemapZone(mtx, iCubemap);
		}
	}
}

/// find localised cubemap render locations and cubemap zones and add them into the localised cubemap data
bool C_ProcessBuildCubemapPoints::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progress( pProgress, GetName() );

	const C_DataConversionOptions* pInputConversionOptions = (const C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataAftkScene* pLitAftkFile = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );

	C_DataCubemapPoints* pOutputCubemapPoints = (C_DataCubemapPoints*)GetOutput( E_GraphData_CubemapPoints );
	assert(pOutputCubemapPoints);

	const Aftk::C_Scene &aftk = pLitAftkFile->GetAftkScene();

	// generate a list of all cubemaps and their names
	for (Aftk::C_SceneNodeConstIterator sceneNodeIterator( &aftk ); sceneNodeIterator; ++sceneNodeIterator)
	{
		AftkUt::C_NodeFlagParser parser( *sceneNodeIterator );
		if (parser.GetFlagGroupCategory() == "System" && parser.GetFlagGroupName() == "CubemapSample")
		{
			// add a cubemap sample
			Aftk::Matrix44 mtx = sceneNodeIterator->GetWorldMatrix();
			Aftk::Position3 vPos(0,0,0);
			vPos = Aftk::ApplyMatrix(mtx, vPos);
			int iCubemap = pOutputCubemapPoints->AddCubemapLocation(Aftk::TemplateToNightmare(vPos), sceneNodeIterator->GetName().c_str());

			ProcessCubemapZones(pOutputCubemapPoints, sceneNodeIterator, iCubemap);
		}
	}

	// copy cubemap paths from conversion options into cubemap data class
	pOutputCubemapPoints->SetLocalisedCubemapPath( pInputConversionOptions->GetToolConfig().GetLocalisedCubemapPath() );
	pOutputCubemapPoints->SetDefaultCubemapFilename( pInputConversionOptions->GetDefaultCubemapFilename() );

	return true;
}

