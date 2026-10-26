#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/helpers/AftkProgress.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <data/LightListData.h>
#include <process/LightList.h>


bool C_ProcessLightList::Process( Progress::I_ProgressBar* pProgress )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pSceneDescription = (C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pSceneDescription);
	C_DataLightList* pLightList = (C_DataLightList*)GetOutput( E_GraphData_LightList );
	assert(pLightList);
	
	//Add default lights

	// add hemisphere light
	if(pInputConversionOptions->GetLightingOptions().GetAddSkyLight())
	{
		C_Light newLight;

		newLight.m_LightType = E_LightType_Hemisphere;

		newLight.m_Colour = Maths::C_RGBA_F32( 1.0f, 1.0f, 1.0f, pInputConversionOptions->GetLightingOptions().GetSkyIntensity());

		Maths::AxisY ay( Maths::C_Normal(0.0f, 0.0f, 1.0f) );
		Maths::AxisZ az( Maths::C_Normal(0.0f, 1.0f, 0.0f) );
		newLight.m_Matrix = Maths::HMat43( az, ay );

		pLightList->SetSkyLight(newLight);
	}


	// add sun light
	if(pInputConversionOptions->GetLightingOptions().GetAddSunLight())
	{
		F32 sinHeading = sinf(pInputConversionOptions->GetLightingOptions().GetSunHeading() * Maths::Pi / 180.0f);
		F32 cosHeading = cosf(pInputConversionOptions->GetLightingOptions().GetSunHeading() * Maths::Pi / 180.0f);

		F32 sinElevation = sinf(pInputConversionOptions->GetLightingOptions().GetSunElevation() * Maths::Pi / 180.0f);
		F32 cosElevation = cosf(pInputConversionOptions->GetLightingOptions().GetSunElevation() * Maths::Pi / 180.0f);

		F32 x = sinHeading	* cosElevation;
		F32 y = 1.0f		* sinElevation;
		F32 z = cosHeading	* cosElevation;

		Maths::HVec3 sunDirection = AftkHelpers::CalculateSunDirection( pInputConversionOptions->GetLightingOptions().GetSunHeading(), pInputConversionOptions->GetLightingOptions().GetSunElevation() );

		C_Light newLight;

		newLight.m_LightType = E_LightType_Directional;

		newLight.m_Colour = Maths::C_RGBA_F32( 1.0f, 1.0f, 1.0f, 1.0f );

		Maths::AxisY ay( Maths::C_Normal(0.0f, 1.0f, 0.0f) );
		Maths::AxisZ az( Maths::C_Normal(-sunDirection) );
		newLight.m_Matrix = Maths::HMat43( az, ay );

		pLightList->SetSunLight(newLight);
	}


	//Add Lights
	for (S32 i = 0; i < pSceneDescription->GetLightNodes().Size(); i++)
	{
		const Aftk::C_SceneNode* pCurrAftkNode = pSceneDescription->GetLightNodes()[i];
		const Aftk::C_SceneNode* pCurrAftkSourceNode = AftkHelpers::InstanceGetSourceNode(pCurrAftkNode);

		Aftk::E_ObjType sourceNodeType = pCurrAftkSourceNode->GetType();
	    
		//Create light
		
		C_Light light;
		Aftk::Matrix44 aftkMatrix = pCurrAftkNode->GetWorldMatrix();
		light.m_Matrix.AX(Maths::HVec3(aftkMatrix[0][0], aftkMatrix[0][1], aftkMatrix[0][2]) );
		light.m_Matrix.AY(Maths::HVec3(aftkMatrix[1][0], aftkMatrix[1][1], aftkMatrix[1][2]) );
		light.m_Matrix.AZ(Maths::HVec3(aftkMatrix[2][0], aftkMatrix[2][1], aftkMatrix[2][2]) );
		light.m_Matrix.AW(Maths::HVec3(aftkMatrix[3][0], aftkMatrix[3][1], aftkMatrix[3][2]) );
		

		switch(sourceNodeType)
		{
			default:
			break;

			case Aftk::E_ObjType_ScenePointLight:
			{
				
				light.m_LightType = E_LightType_Point;

				Aftk::C_ScenePointLightNode* pLightNode = (Aftk::C_ScenePointLightNode*)pCurrAftkSourceNode;

				const Aftk::Vector4F32 colour = pLightNode->GetColour();
				const F32 intensity = pLightNode->GetIntensity();

				light.m_Colour = Maths::C_RGBA_F32( colour[0], colour[1], colour[2], intensity);
				
				//Approximate radius using intensity
				light.m_PointCutOffRadius = Maths::Sqrt(intensity / 0.01f);

				pLightList->AddLight(light);
			}
			break;
			case Aftk::E_ObjType_SceneDirectionalLight:
			{
				light.m_LightType = E_LightType_Directional;

				Aftk::C_SceneDirectionalLightNode* pLightNode = (Aftk::C_SceneDirectionalLightNode*)pCurrAftkSourceNode;

				const Aftk::Vector4F32 colour = pLightNode->GetColour();
				const F32 intensity = pLightNode->GetIntensity();

				light.m_Colour = Maths::C_RGBA_F32( colour[0], colour[1], colour[2], intensity );

				pLightList->AddLight(light);
			}
			break;
			case Aftk::E_ObjType_SceneSpotLight: 	
			{
				light.m_LightType = E_LightType_Spot;

				Aftk::C_SceneSpotLightNode* pLightNode = (Aftk::C_SceneSpotLightNode*)pCurrAftkSourceNode;

				const Aftk::Vector4F32 colour = pLightNode->GetColour();
				const F32 intensity = pLightNode->GetIntensity();

				light.m_Colour = Maths::C_RGBA_F32( colour[0], colour[1], colour[2], intensity );

				light.m_SpotConeAngle = pLightNode->GetConeAngle();
				light.m_SpotPenumbraAngle = pLightNode->GetPenumbraAngle();
				light.m_SpotDropOff = pLightNode->GetDropOff();

				//Approximate radius using intensity
				light.m_SpotCutOffRadius = Maths::Sqrt(intensity / 0.01f);

				pLightList->AddLight(light);
			}
			break;
			case Aftk::E_ObjType_SceneAreaLight: 	
			{
				light.m_LightType = E_LightType_Area;

				Aftk::C_SceneAreaLightNode* pLightNode = (Aftk::C_SceneAreaLightNode*)pCurrAftkSourceNode;

				const Aftk::Vector4F32 colour = pLightNode->GetColour();
				const F32 intensity = pLightNode->GetIntensity();

				light.m_Colour = Maths::C_RGBA_F32( colour[0], colour[1], colour[2], intensity );

				//Approximate radius using intensity
				light.m_Radius = Maths::Sqrt(intensity / 0.01f);

				pLightList->AddLight(light);
			}
			break;
			case Aftk::E_ObjType_SceneHemisphereLight:
			{
				light.m_LightType = E_LightType_Hemisphere;

				Aftk::C_SceneHemisphereLightNode* pLightNode = (Aftk::C_SceneHemisphereLightNode*)pCurrAftkSourceNode;

				const Aftk::Vector4F32 colour = pLightNode->GetColour();
				const F32 intensity = pLightNode->GetIntensity();

				light.m_Colour = Maths::C_RGBA_F32( colour[0], colour[1], colour[2], intensity );
				
				pLightList->AddLight(light);
			}
			break;
		} 
	}	

	return true;
}

const Util::C_String C_ProcessLightList::GetName() const 
{
	return Util::C_String("C_ProcessLightList");
}
