#include <stdafx.h>
#include <misc_shared/types/Float16.h> // [Blur2PC][HDR] Native floating light-cube packing.
#include <Game/GameWorld/VehicleLightRGBM.h>

#ifndef PLATFORM_SPU
#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/View.h>
#include <world_render/LightList.h>
#include <scenegraph/light/Manager.h>
#include <scenegraph/view.h>

Debug::Float g_DynamicLightsRimFactor("Amax.DynamicLightsRimFactor", 10.f , Debug::Range(0.0f, 50.0f));
Debug::Float g_DynamicLightsShadowFactor("Amax.DynamicLightsShadowFactor", 3.f , Debug::Range(0.0f, 10.0f));
Debug::Float g_StaticRimLightRadiusFactor("Amax.StaticRimLightRadiusFactor", 1.f , Debug::Range(0.0f, 10.0f));
Debug::Float g_PointRimLightRadiusFactor("Amax.PointRimLightRadiusFactor", 4.f , Debug::Range(0.0f, 10.0f));
Debug::Float g_SpotRimLightRadiusFactor("Amax.SpotRimLightRadiusFactor", 1.f , Debug::Range(0.0f, 10.0f));
Debug::Float g_HeadLightFadeFactor("Amax.HeadLightFadeFactor", 0.6f , Debug::Range(0.0f, 1.0f));
Debug::Float g_CarPointLightFactor("Amax.CarPointLightFactor", 1.0f , Debug::Range(0.0f, 10.0f));
Debug::Float g_CarPointLightRadiusFactor("Amax.CarPointLightRadiusFactor", 1.5f , Debug::Range(0.0f, 10.0f));
Debug::Float g_CarSpotLightFactor("Amax.CarSpotLightFactor", 1.f , Debug::Range(0.0f, 10.0f));
Debug::Float g_CarSpotLightRadiusFactor("Amax.CarSpotLightRadiusFactor", 1.f , Debug::Range(0.0f, 10.0f));
Debug::Float g_CarFakeSunLightCubemapFactor("Amax.CarFakeSunLightCubemapFactor", 10.f , Debug::Range(0.0f, 20.0f));
Debug::Float g_CarSunFakeOccusionFactor("Amax.CarSunFakeOccusionFactor", 2.f, Debug::Range(0.f, 5.f));

Debug::Float g_StaticPointLightsAttenuateFactor("Amax.StaticPointLightsAttenuateFactor", 0.015f , Debug::Range(0.0f, 1.0f));
Debug::Float g_MinIntensityStaticLight("Amax.MinIntensityStaticLight", 15.f , Debug::Range(0.0f, 30.0f));

#else
#include <system/perf.h>
#include <../source/maths/hvec/VectorConst.cpp>
#include <display/device/DeviceTypesEnums.h>
#include "GameWorldTasks.h"
#endif


namespace Game
{
	#ifndef PLATFORM_SPU

	const U32 nearestMaxCount = GameWorld_MaxShadowCasters * 4;
	const U32 carBoundBoxRadius = 30;

	void AddSHLight(const Maths::HVec3& direction, const MathsStorage::Float3& colour, F32 shData[16])
	{
		float x = direction.X();
		float y = direction.Y();
		float z = direction.Z();

		static const float a = 0.5f*sqrtf(1.0f/(float)M_PI);
		static const float b = 0.5f*sqrtf(3.0f/(float)M_PI);

		Maths::HVec4 shPreVector;

		shPreVector.X( a );
		shPreVector.Y( -b*y);
		shPreVector.Z( b*z);
		shPreVector.W( -b*x);
		
		//Transform the shVector for normal dot product
		Maths::HVec4 shVector;

		const float fC0 = 0.282094777; //1.0f / (2.0f * sqrtf(PI));
		const float fC1 = 0.325734973; //sqrt(3.0f) / (3.0f * sqrtf(PI));

		shVector.W(fC0*shPreVector.X());
		shVector.Y(-fC1*shPreVector.Y());
		shVector.Z(fC1*shPreVector.Z());
		shVector.X(-fC1*shPreVector.W());

		//Add to shData
		
		//Red
		shData[0] += shVector.X() * colour.x ;
		shData[1] += shVector.Y() * colour.x ;
		shData[2] += shVector.Z() * colour.x ;
		shData[3] += shVector.W() * colour.x ;

		//Green
		shData[4] += shVector.X() * colour.y ;
		shData[5] += shVector.Y() * colour.y ;
		shData[6] += shVector.Z() * colour.y ;
		shData[7] += shVector.W() * colour.y ;

		//Blue
		shData[8]  += shVector.X() * colour.z ;
		shData[9]  += shVector.Y() * colour.z ;
		shData[10] += shVector.Z() * colour.z ;
		shData[11] += shVector.W() * colour.z ;
	}


	void CarDynamicEffectsTask(Scheduler::Task* pTask)
	{
		PERF_EVENT("CarDynamicEffectsTask");

		//Read Input structure
		Game::C_CarDynamicEffectsTaskInit* pInit = (Game::C_CarDynamicEffectsTaskInit*)pTask->Input;
		SceneGraph::C_SceneGraph*		pSceneGraph = pInit->m_pSceneGraph;
		const WorldRender::C_LightList*	pLightList = pInit->m_pLightList;
		const Util::KdTree<WorldRender::C_Light>& lightVector = pLightList->GetLightList();
		Game::C_ViewManager*		pViewManager = pInit->m_pViewManager;
		Game::C_GameWorld* pGameWorld = pInit->m_pGameWorld;
		U32 numViews = pInit->m_NumViews;
		Maths::HVec4 fakeLightColour = pInit->m_FakeLightColour;
		Maths::HVec3 fakeLightPosition = pInit->m_FakeLightPosition;

		//For each view
		for (U32 iView = 0; iView < numViews; iView++)
		{
			PERF_EVENT("CarDynamicEffectsTaskPerView");

			Game::C_View*			pView = pInit->m_pView[iView];

			//Shadow Casters init
			Game::C_ShadowCaster* shadowCasterVector = pView->m_ShadowCaster;
			U32	numShadowCasters = 0;
			U32 maxShadowCasters = pView->m_MaxShadowCasters;
			Maths::HVec3 shadorCasterCentre = pView->GetReferencePosition();

			//Rim Generators init
			Game::C_RimGenerator* rimGeneratorVector = pView->m_RimGeneratorVector;
			U32 numRimGenerators = 0;
			U32 maxRimGenerators = GameWorld_MaxRimGenerators - numViews + 1; //Reduce rim generators if more that one view

			Maths::C_AxisBox vbox(Maths::C_Sphere(shadorCasterCentre, carBoundBoxRadius));

			//Dynamic lights
			Game::C_DynamicPointLight *pDynamicPointLightVector = pInit->m_DynamicPointLightVector[iView];
			U32	dynamicPointLightCount = pInit->m_DynamicPointLightCount[iView];
			Game::C_DynamicSpotLight *pDynamicSpotLightVector = pInit->m_DynamicSpotLightVector[iView];
			U32	dynamicSpotLightCount = pInit->m_DynamicSpotLightCount[iView];

			//Get nearest static lights
			int nearestIndex[nearestMaxCount];
			S32 numNearestIndexs = lightVector.Intersect(vbox, nearestIndex, nearestMaxCount);
			
			{
				PERF_EVENT("Shadow Casters");
				//Look for nearest shadow caster
				while (numShadowCasters < maxShadowCasters)
				{
					Game::C_ShadowCaster nextNearestShadowCaster;
					nextNearestShadowCaster.m_Intensity = 0.f;

					bool lightFound = false;
					F32	minDistant = 100000.f;
					//Find the nearest (static)
					for (S32 i = 0; i < numNearestIndexs; i++)
					//for (S32 i = 0; i < lightVector.Size(); i++)
					{
						bool newLight = true;
						const WorldRender::C_Light& light = lightVector[nearestIndex[i]];
						//const WorldRender::C_Light& light = lightVector[i];
						F32 distant = (light.m_Matrix.AW() - shadorCasterCentre).Length2();
						if (minDistant < distant)
							continue;

						//it is new?
						for (U32 ii = 0; (ii < numShadowCasters) && newLight; ii++)
						{
							if ((shadowCasterVector[ii].m_Position - light.m_Matrix.AW()).Length2() < 1.5f)
							{
								newLight = false;
								//shadowCasterVector[ii].m_Intensity = Maths::Max(shadowCasterVector[ii].m_Intensity, (light.m_Colour.X() + light.m_Colour.Y() + light.m_Colour.Z()) * light.m_Colour.W());
							}
						}
						//Calculate distant
						Maths::HVec3 distantVector = light.m_Matrix.AW()  -  shadorCasterCentre;
						if (((distantVector.Y() * distantVector.Y()) / (distantVector.X() * distantVector.X() + distantVector.Z() * distantVector.Z())) < 0.05f ||
							distantVector.Y() < 1.5f)
							newLight = false;

						if (newLight)
						{
							nextNearestShadowCaster.m_Position = light.m_Matrix.AW();
							nextNearestShadowCaster.m_Intensity = (light.m_Colour.X() + light.m_Colour.Y() + light.m_Colour.Z()) * light.m_Colour.W();
					
							minDistant = distant;
							lightFound = true;
						}
					}
					if (lightFound)
					{
						//Add new shadow caster
						shadowCasterVector[numShadowCasters] = nextNearestShadowCaster;
						numShadowCasters++;
					}
					else
						break; //No more lights

				}

			/*
				//Calculate the fade out shadow casters
				Game::C_ShadowCaster fadeOutShadowCaster[15]; //try 15 as max
				U32 numFadeOutShadowCasters = 0;
				
				for (U32 i = 0; i < numOldShadowCasters && numFadeOutShadowCasters < 15; i++)
				{
					//Add from the old ones
					if (oldShadowCasterVector[i].m_FadeOutEnable)
					{
						fadeOutShadowCaster[numFadeOutShadowCasters] = oldShadowCasterVector[i];
						numFadeOutShadowCasters++;
					}
					else
					{
						bool found = false;
						//Check if it is in the new ones
						for (U32 j = 0; j < numShadowCasters && !found; j++)
						{
							
							//Check if it is the same
							if ((oldShadowCasterVector[i].m_Position - shadowCasterVector[j].m_Position).Length2() < 0.001f && fabs(oldShadowCasterVector[i].m_Intensity - shadowCasterVector[j].m_Intensity) < 0.05f)
							{
								//It not a new fade out shadow
								found = true;
								shadowCasterVector[j].m_FadeOutTime = oldShadowCasterVector[i].m_FadeOutTime; //Pass the fadeouttime
							}
						}
						if (!found)
						{
							//New fade out shadow, add to the vector
							fadeOutShadowCaster[numFadeOutShadowCasters] = oldShadowCasterVector[i];
							fadeOutShadowCaster[numFadeOutShadowCasters].m_FadeOutTime = 1.f;
							numFadeOutShadowCasters++;
						}
					}
				}

				//Fadeout to the old shadow lights, still without fade out
				for (U32 i = 0; i < numShadowCasters; i++)
				{
					shadowCasterVector[i].m_FadeOutTime += 0.05f;
				}
				
				//Find the tree more important fade lights to the next shadow casters
				U32 indexSorted[3] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
				for (U32 i = 0; i < 3; i++)
				{
					U32 index = 0xFFFFFFFF;
					F32 fadeOutTime = 0.f; 
					for (U32 j = 0; j < numFadeOutShadowCasters; j++)
					{
						if (j != indexSorted[0] && j != indexSorted[1] && j != indexSorted[2])
						{
							if (fadeOutTime < fadeOutShadowCaster[j].m_FadeOutTime)
							{
								fadeOutTime = fadeOutShadowCaster[j].m_FadeOutTime;
								index = j;
							}
						}
					}

					if (index != 0xFFFFFFFF)
					{
						indexSorted[i] = index;

						if (numShadowCasters < GameWorld_MaxShadowCasters)
						{
							fadeOutTime -= 0.05f;
							if (fadeOutTime > 0.f)
							{
								shadowCasterVector[numShadowCasters] = fadeOutShadowCaster[index];
								//Apply time
								shadowCasterVector[numShadowCasters].m_FadeOutTime = fadeOutTime;
								shadowCasterVector[numShadowCasters].m_FadeOutEnable = true;
								numShadowCasters++;	
							}
						}
					}
				}*/
				pView->m_NumShadowCasters = numShadowCasters;
			}
			{
				PERF_EVENT("RIM generators");

				//Add sun if needed
				if (fakeLightColour.W() > 0.f)
				{
					//Fake Sun light
					rimGeneratorVector[0].m_Colour = Maths::HVec3(fakeLightColour) * g_CarFakeSunLightCubemapFactor;
					rimGeneratorVector[0].m_Direction = Maths::HVec3(0.f, 0.f, 0.f);
					rimGeneratorVector[0].m_Position = fakeLightPosition; 
					rimGeneratorVector[0].m_Radius = -1.f;
					F32 fakeLight = Maths::Min(fakeLightColour.W() * g_CarSunFakeOccusionFactor, 1.f);
					rimGeneratorVector[0].m_Colour = rimGeneratorVector[0].m_Colour * fakeLight;
					numRimGenerators++;
				}

				//Look for the nearest rim generators
				while (numRimGenerators < GameWorld_MaxRimGenerators)
				{
					Game::C_RimGenerator nextNearestRimGenerator;
					F32	minDistant = 100000.f;
					bool lightFound = false;
					//Find the nearest (static)
					for (S32 i = 0; i < numNearestIndexs; i++)
					//for (S32 i = 0; i < lightVector.Size(); i++)
					{
						const WorldRender::C_Light& light = lightVector[nearestIndex[i]];
						//const WorldRender::C_Light& light = lightVector[i];
						bool newLight = true;

						if (light.m_Colour.W() < g_MinIntensityStaticLight)
							continue;

						F32 distant = (light.m_Matrix.AW() - shadorCasterCentre).Length2();
						if (minDistant < distant)
							continue;

						if (distant > (light.m_Radius * light.m_Radius) )
							continue;

						for (U32 ii = 0; (ii < numRimGenerators) && newLight; ii++)
						{
							if ((rimGeneratorVector[ii].m_Position - light.m_Matrix.AW()).Length2() < 1.5f)
							{
								newLight = false;
								//nextNearestRimGenerator.m_Colour += Maths::HVec3(light.m_Colour.X() * light.m_Colour.W() / distant, light.m_Colour.Y() * light.m_Colour.W() / distant, light.m_Colour.Z() * light.m_Colour.W() / distant);
							}
						}
						if (newLight)
						{
							nextNearestRimGenerator.m_Direction = Maths::HVec3(0.f, 0.f, 0.f);
							nextNearestRimGenerator.m_Position = light.m_Matrix.AW();
							F32 factor = pGameWorld->GetCarLightSettings().m_StaticLightCubemapFactor * light.m_Colour.W();
							if (light.m_LightType == WorldRender::E_LightType_Point) //Point lights need to attenuate
								factor *= g_StaticPointLightsAttenuateFactor;
							F32 max = pGameWorld->GetCarLightSettings().m_LightCubemapMax;
							nextNearestRimGenerator.m_Colour = Maths::HVec3(Maths::Min(light.m_Colour.X() * factor, max), Maths::Min(light.m_Colour.Y() * factor, max), Maths::Min(light.m_Colour.Z() * factor, max));
							nextNearestRimGenerator.m_Radius = light.m_Radius * g_StaticRimLightRadiusFactor;
							minDistant = distant;
							lightFound = true;
						}
					}
					//Find the nearest point light (dynamic)
					for (U32 i = 0; i < dynamicPointLightCount; i++)
					{
						bool newLight = true;
						
						const Game::C_DynamicPointLight& pointLight = pDynamicPointLightVector[i];

						if (pointLight.m_Flags != 0)
							continue;

						if (pointLight.m_Radius < 1.f)
							continue;

						const Maths::HVec3& position = pointLight.m_Position;
						F32 distant = (position - shadorCasterCentre).Length2();
						if (minDistant < distant)
							continue;

						for (U32 ii = 0; (ii < numRimGenerators) && newLight; ii++)
						{
							if ((rimGeneratorVector[ii].m_Position - position).Length2() < 1.5f)
							{
								newLight = false;
							}
						}
						if (newLight)
						{
							nextNearestRimGenerator.m_Position = position;
							nextNearestRimGenerator.m_Direction = Maths::HVec3(0.f, 0.f, 0.f);
							F32 attenuation = pGameWorld->GetCarLightSettings().m_DynamicLightCubemapFactor * g_DynamicLightsRimFactor; 
							F32 max = pGameWorld->GetCarLightSettings().m_LightCubemapMax;
							nextNearestRimGenerator.m_Colour = Maths::HVec3(Maths::Min(pointLight.m_Colour.X() * attenuation, max), Maths::Min(pointLight.m_Colour.Y() * attenuation, max), Maths::Min(pointLight.m_Colour.Z() * attenuation, max));
							nextNearestRimGenerator.m_Radius = pointLight.m_Radius * g_PointRimLightRadiusFactor;
							minDistant = distant;
							lightFound = true;
						}
					}
					//Find the nearest spot light (dynamic)
					for (U32 i = 0; i < dynamicSpotLightCount; i++)
					{
						bool newLight = true;

						const Game::C_DynamicSpotLight& spotLight = pDynamicSpotLightVector[i];

						if (spotLight.m_Flags != 0)
							continue;

						const Maths::HVec3&	position	= spotLight.m_Position;
						const Maths::HVec3&	direction	= spotLight.m_Direction;

						F32 distant = (position - shadorCasterCentre).Length2();
						if (minDistant < distant)
							continue;

						for (U32 ii = 0; (ii < numRimGenerators) && newLight; ii++)
						{
							if ((rimGeneratorVector[ii].m_Position - position).Length2() < 1.5f && (rimGeneratorVector[ii].m_Direction - direction).Length2() < 1.f)
							{
								newLight = false;
							}
						}
						if (newLight)
						{
							nextNearestRimGenerator.m_Direction = direction; //It has direction
							nextNearestRimGenerator.m_Position = position;
							F32 attenuation = pGameWorld->GetCarLightSettings().m_DynamicLightCubemapFactor * g_DynamicLightsRimFactor; 
							F32 max = pGameWorld->GetCarLightSettings().m_LightCubemapMax;
							nextNearestRimGenerator.m_Colour = Maths::HVec3(Maths::Min(spotLight.m_Colour.X() * attenuation, max), Maths::Min(spotLight.m_Colour.Y() * attenuation, max), Maths::Min(spotLight.m_Colour.Z() * attenuation, max));
							nextNearestRimGenerator.m_Radius = spotLight.m_Radius * g_SpotRimLightRadiusFactor;
							minDistant = distant;
							lightFound = true;
						}
					}


					if (lightFound)
					{
						//Add new shadow caster
						rimGeneratorVector[numRimGenerators] = nextNearestRimGenerator;
						numRimGenerators++;
					}
					else
						break; //No more lights
				}

				for (U32 i = numRimGenerators; i < GameWorld_MaxRimGenerators; i++)
				{
					rimGeneratorVector[i].m_Colour = Maths::HVec3(0.f,0.f,0.f);
				}

				pView->m_NumRimGenerators = numRimGenerators;
			}
		}
		{
			PERF_EVENT("Car per View index");

			//Associate all the cars to view
			C_VehicleTaskInfo* vehicleInfo = pInit->m_VehicleInfo;
			U32	numVehicles = pInit->m_NumVehicles;
			U32 numViews = pInit->m_NumViews;

			//View positions
			Maths::HVec3 viewPosition[4];
			for (U32 i = 0; i < numViews; i++)
			{
				viewPosition[i] = pInit->m_pView[i]->GetMainView().GetViewSettings3D().GetCameraPosition();
			}
			
		
			switch (numViews)
			{
			case 1:
				for (U32 i = 0; i < numVehicles; i++)
				{
					vehicleInfo[i].m_ViewIndex = 0;
				}
				break;
			case 2:
				{
					for (U32 i = 0; i < numVehicles; i++)
					{
						if (vehicleInfo[i].m_ViewIndex == -1) //Needs to associate a viewport
						{
							F32 distant1 = (vehicleInfo[i].m_Position - viewPosition[0]).Length2();
							F32 distant2 = (vehicleInfo[i].m_Position - viewPosition[1]).Length2();
							if (distant1 < distant2)
								vehicleInfo[i].m_ViewIndex = 0;
							else
								vehicleInfo[i].m_ViewIndex = 1;
						}
					}
				}
				break;
			case 3:
			case 4:
				{
					for (U32 i = 0; i < numVehicles; i++)
					{
						if (vehicleInfo[i].m_ViewIndex == -1) //Needs to associate a viewport
						{
							F32 carDistant[4];
							for (U32 j = 0; j < numViews; j++)
							{
								carDistant[j] = (vehicleInfo[i].m_Position - viewPosition[j]).Length2(); 
							}
							F32 distantMaxim = carDistant[0];
							U32 viewIndex = 0;

							for (U32 j = 1; j < numViews; j++)
							{
								if (carDistant[j] < distantMaxim)
								{
									distantMaxim = carDistant[j];
									viewIndex = j;
								}
							}
							BIZ_ASSERT(viewIndex != 0xFFFFFFFF);
							
							vehicleInfo[i].m_ViewIndex = viewIndex;
						}
					}
				}
				break;
			}
		}
		{
			PERF_EVENT("Dynamic SH light");

			//Calculate dynamic SH for ligting in each car
			C_VehicleTaskInfo* vehicleInfo = pInit->m_VehicleInfo;
			U32	numVehicles = pInit->m_NumVehicles;

			//For each car
			for (U32 i = 0; i < numVehicles; i++)
			{
				if (vehicleInfo[i].m_NeedUpdateDynamicSH)
				{
					//Reset 0
					for (U32 j = 0; j < 16; j++)
						vehicleInfo[i].m_DynamicSphericalHarmonicLight[j] = 0.f;

					//Get View dependent
					U32 carView = vehicleInfo[i].m_ViewIndex;

					//Dynamic lights
					Game::C_DynamicPointLight *pDynamicPointLightVector = pInit->m_DynamicPointLightVector[carView];
					U32	dynamicPointLightCount = pInit->m_DynamicPointLightCount[carView];
					Game::C_DynamicSpotLight *pDynamicSpotLightVector = pInit->m_DynamicSpotLightVector[carView];
					U32	dynamicSpotLightCount = pInit->m_DynamicSpotLightCount[carView];

					//Point lights
					for (U32 j = 0; j < dynamicPointLightCount; j++)
					{
						const Game::C_DynamicPointLight& pointLight = pDynamicPointLightVector[j];

						if ((pointLight.m_Flags&vehicleInfo[i].m_LightFlags) != 0)  //Only lights from other cars
							continue;

						Maths::HVec3 dir = pointLight.m_Position  - vehicleInfo[i].m_Position;
						F32 distant = dir.Length();
						dir = dir * 1.f / distant; //Normalize direction
						//Calculate attenuation
						F32 attenuation = 1.f - (distant * distant)/(pointLight.m_Radius * pointLight.m_Radius * g_CarPointLightRadiusFactor * g_CarPointLightRadiusFactor);
						attenuation*= g_CarPointLightFactor;

						if (attenuation > 0.f)
						{
							F32 max = pGameWorld->GetCarLightSettings().m_SHLightMax;
							F32 factor = pGameWorld->GetCarLightSettings().m_SHLightFactor * attenuation;
							MathsStorage::Float3 colour = MathsStorage::Float3(Maths::Min(pointLight.m_Colour.X() * factor, max), Maths::Min(pointLight.m_Colour.Y() * factor, max), Maths::Min(pointLight.m_Colour.Z() * factor, max));

							//Add to sh Data
							AddSHLight(dir, colour, vehicleInfo[i].m_DynamicSphericalHarmonicLight);
						}
					}

					//Spot Lights
					for (U32 j = 0; j < dynamicSpotLightCount; j++)
					{
						const Game::C_DynamicSpotLight& spotLight = pDynamicSpotLightVector[j];

						if ((spotLight.m_Flags&vehicleInfo[i].m_LightFlags) != 0)  //Only lights from other cars
							continue;

						const Maths::Mat44& mWorldLight = spotLight.m_WorldLight;

						Maths::HVec3 dir = spotLight.m_Position - vehicleInfo[i].m_Position;
						F32 distant = dir.Length();
						dir = dir * 1.f / distant; //Normalize direction
						//Calculate attenuation
						
						//Fade the light at the begining, radius cone greater than 2, attenuate last 2 meter
						F32 coneSize = distant * tanf(Maths::Max(spotLight.m_HorizontalFoV, spotLight.m_VerticalFoV));
						F32 attenuation = Maths::Clamp(((spotLight.m_Radius * g_CarSpotLightRadiusFactor) - distant) * 0.5f, 0.f, 1.f) * Maths::Clamp(coneSize - 1.0f, 0.f, 1.f);
						attenuation *= g_CarSpotLightFactor;

						if (attenuation > 0.f)
						{
							Maths::Vec4F32 spotPosition = Maths::MakeVec4F32(vehicleInfo[i].m_Position.X(), vehicleInfo[i].m_Position.Y(), vehicleInfo[i].m_Position.Z(), 1.f) * mWorldLight;

							F32 projectionX;
							F32 projectionY;
							if (Maths::Z(spotPosition) > Maths::Const<Maths::VF32>::Zero())
							{
								Maths::Store(Maths::X(spotPosition) / Maths::Z(spotPosition), &projectionX);
								Maths::Store(Maths::Y(spotPosition) / Maths::Z(spotPosition), &projectionY);

								projectionX *= 1.f / tanf(spotLight.m_HorizontalFoV);
								projectionY *= 1.f / tanf(spotLight.m_VerticalFoV);

								F32 distantCenter = sqrtf(projectionX * projectionX + projectionY * projectionY);


								attenuation *= 1.f - Maths::Clamp(distantCenter, 0.f, 1.f);
								attenuation = Maths::Min(attenuation, 1.f);

								if (attenuation > 0.f)
								{
									F32 max = pGameWorld->GetCarLightSettings().m_SHLightMax;
									F32 factor = pGameWorld->GetCarLightSettings().m_SHLightFactor * attenuation;
									MathsStorage::Float3 colour = MathsStorage::Float3(Maths::Min(spotLight.m_Colour.X() * factor, max), Maths::Min(spotLight.m_Colour.Y() * factor, max), Maths::Min(spotLight.m_Colour.Z() * factor, max));

									//Add to sh Data
									AddSHLight(dir, colour, vehicleInfo[i].m_DynamicSphericalHarmonicLight);
								}
							}
						}
					}
				}
			}
		}
		{
			PERF_EVENT("HeadLights Fade");

			//Calculate if diferents headlights affects the same area
			C_VehicleTaskInfo* vehicleInfo = pInit->m_VehicleInfo;
			U32	numVehicles = pInit->m_NumVehicles;

			//Reset
			for (U32 i = 0; i < numVehicles; i++)
				vehicleInfo[i].m_HeadLightFadeIntensity = 1.f;

			//For each car
			for (U32 i = 0; i < numVehicles; i++)
			{
				C_VehicleTaskInfo& currentVehicleInfo = vehicleInfo[i];

				//Check with the rest of cars
				for (U32 j = i + 1 ; j < numVehicles; j++)
				{
					//Check distant
					F32 distant = (currentVehicleInfo.m_HeadLightPosition - vehicleInfo[j].m_HeadLightPosition).Length();
					F32 maxRadius = currentVehicleInfo.m_HeadLightRadius + vehicleInfo[j].m_HeadLightRadius;
					F32 fade = 1.f - Maths::Clamp((maxRadius - distant)/distant, 0.f, 1.f);
								
					//Fade it
					F32 totalFade = g_HeadLightFadeFactor + fade * (1.f - g_HeadLightFadeFactor);
					currentVehicleInfo.m_HeadLightFadeIntensity *= totalFade;
					vehicleInfo[j].m_HeadLightFadeIntensity *= totalFade;
				}
			}
		}
	}

#endif

// 	switch (face)
// 	{
// 	case Display::E_CubeTextureFace_PosX:
// 		dirVector = Maths::HVec3( 1.0f,  0.0f,  0.0f);
// 		upVector = Maths::HVec3( 0.0f,  1.0f,  0.0f);
// 		break;
// 	case Display::E_CubeTextureFace_PosY:
// 		dirVector = Maths::HVec3( 0.0f,  1.0f,  0.0f);
// 		upVector = Maths::HVec3( 0.0f,  0.0f, -1.0f);
// 		break;
// 	case Display::E_CubeTextureFace_PosZ:
// 		dirVector = Maths::HVec3( 0.0f,  0.0f,  1.0f);
// 		upVector = Maths::HVec3( 0.0f,  1.0f,  0.0f);
// 		break;
// 	case Display::E_CubeTextureFace_NegX:
// 		dirVector = Maths::HVec3(-1.0f,  0.0f,  0.0f);
// 		upVector = Maths::HVec3( 0.0f,  1.0f,  0.0f);
// 		break;
// 	case Display::E_CubeTextureFace_NegY:
// 		dirVector = Maths::HVec3( 0.0f, -1.0f,  0.0f);
// 		upVector = Maths::HVec3( 0.0f,  0.0f,  1.0f);
// 		break;
// 	case Display::E_CubeTextureFace_NegZ:
// 		dirVector = Maths::HVec3( 0.0f,  0.0f, -1.0f);
// 		upVector = Maths::HVec3( 0.0f,  1.0f,  0.0f);
// 		break;
// 	}
// 	Maths::HVec3 xVector = dirVector ^ upVector;
// 	xVector.Normalise();

	Maths::HVec3 g_CubeMapVectorTable[18] = 
	{
		Maths::HVec3( 1.0f,  0.0f,  0.0f),	Maths::HVec3( 0.0f,  1.0f,  0.0f), Maths::HVec3( 0.0f,  0.0f,  1.0f),
		Maths::HVec3( 0.0f,  1.0f,  0.0f),	Maths::HVec3( 0.0f,  0.0f, -1.0f), Maths::HVec3( -1.0f,  0.0f,  0.0f),
		Maths::HVec3( 0.0f,  0.0f,  1.0f),	Maths::HVec3( 0.0f,  1.0f,  0.0f), Maths::HVec3( -1.0f,  0.0f,  0.0f),
		Maths::HVec3(-1.0f,  0.0f,  0.0f),	Maths::HVec3( 0.0f,  1.0f,  0.0f), Maths::HVec3( 0.0f,  0.0f,  -1.0f),
		Maths::HVec3( 0.0f, -1.0f,  0.0f),	Maths::HVec3( 0.0f,  0.0f,  1.0f), Maths::HVec3( -1.0f,  0.0f,  0.0f),
		Maths::HVec3( 0.0f,  0.0f, -1.0f),	Maths::HVec3( 0.0f,  1.0f,  0.0f), Maths::HVec3( 1.0f,  0.0f,  0.0f)
	};

//	void CarUpdateLightCubeMapTask(Scheduler::Task* pTask)
//	{
//		PERF_EVENT("CarUpdateLightCubeMapTask");
//
//		//Get data
//#ifdef PLATFORM_SPU
//		ALIGN16 U8 m_Buffer[sizeof(Game::C_CarUpdateLightCubeMapTaskInit)+15 & ~0xf];
//		//new(&m_Buffer[0]) Game::C_CarUpdateLightCubeMapTaskInit();
//		Game::C_CarUpdateLightCubeMapTaskInit* init = (Game::C_CarUpdateLightCubeMapTaskInit*)&m_Buffer[0];
//		Spu::Fetch(init, pTask->Input, sizeof(Game::C_CarUpdateLightCubeMapTaskInit));
//		const Game::C_CarUpdateLightCubeMapTaskInit* pInit = init;
//		Spu::Wait(0);
//#else
//		const Game::C_CarUpdateLightCubeMapTaskInit* pInit = (Game::C_CarUpdateLightCubeMapTaskInit*)pTask->Input;
//#endif
//		const U32 numLightCubeMaps = pInit->m_NumLightCubeMaps;
//		const C_LightCubeMapInfo* lightCubeInfo = pInit->m_LightCubeMapInfo;
//
//#ifdef PLATFORM_SPU
//		BIZ_STATIC_ASSERT((sizeof(C_RimGenerator) * GameWorld_MaxRimGenerators) % 16 == 0);
//		ALIGN16 C_RimGenerator rimGeneratorVector[GameWorld_MaxRimGenerators];
//		Spu::Fetch(rimGeneratorVector, pInit->m_RimGeneratorVector, sizeof(rimGeneratorVector));
//#else
//		const C_RimGenerator*	rimGeneratorVector = pInit->m_RimGeneratorVector;
//#endif
//		//Get range
//		const U32 firstLightCubeMap = pTask->ParamData[0];
//		const U32 lastLightCubeMap = pTask->ParamData[1];
//
//		//Data
//		Maths::HVec3 rimGeneratorDirVector[GameWorld_MaxRimGenerators];
//		Maths::HF32	 rimGeneratorAttenuationVector[GameWorld_MaxRimGenerators];
//
//		//for each cubemap
//		for (U32 i = firstLightCubeMap; i < lastLightCubeMap; i++)
//		{
//			PERF_EVENT("UpdateLightCubeMap");
//			//Calculate rim generator vectors
//			for (U32 jj = 0; jj < GameWorld_MaxRimGenerators; jj++)
//			{
//				rimGeneratorDirVector[jj] = (rimGeneratorVector[jj].m_Position - lightCubeInfo[i].m_Position);
//				F32 distant = rimGeneratorDirVector[jj].Length();
//				rimGeneratorDirVector[jj] /= distant;
//				//Check if it has direction
//				if (rimGeneratorVector[jj].m_Direction.Length2() > 0.1f)
//				{
//					//Apply direction
//					F32 factor = Maths::Clamp(2.f * F32(rimGeneratorDirVector[jj] | -rimGeneratorVector[jj].m_Direction) - 1.f, 0.f, 1.f);
//					rimGeneratorDirVector[jj] *= factor;
//				}
//
//				//Attenuation
//				//F32 distantAttenuation = Maths::Clamp((1.f - ((distant * distant) / (rimGeneratorVector[jj].m_Radius * rimGeneratorVector[jj].m_Radius))), 0.f, 1.f);
//				F32 distantAttenuation = Maths::Clamp((1.f - ((distant) / ((F32)rimGeneratorVector[jj].m_Radius))), 0.f, 1.f);
//				F32 closeAttenuation = Maths::Clamp(distant, 0.f, 1.f);
//				F32 downAttenuation = Maths::Clamp(1.f + F32(rimGeneratorDirVector[jj].Y()), 0.f, 1.f);
//
//				rimGeneratorAttenuationVector[jj] =  distantAttenuation * closeAttenuation * downAttenuation;
//			}
//
//			//for each face
//			for (Display::E_CubeTextureFace face = Display::E_CubeTextureFace_PosX; face < Display::E_CubeTextureFace_Count; face = Display::E_CubeTextureFace(face + 1) )
//			{
//				const U32 pitch = lightCubeInfo[i].m_Pitch / 4;
//
//				#ifdef PLATFORM_SPU
//				U32* pInitLockedMemory = (U32*)Spu::Alloc(pitch * GameWorld_LightCubeMapSize * 4);
//				U32* pLockedMemory = pInitLockedMemory;
//				#else
//				U32* pLockedMemory = (U32*)lightCubeInfo[i].m_MemoryLocked[face];
//				#endif
//
//				const Maths::HVec3 dirVector = g_CubeMapVectorTable[0 + U32(face) * 3];
//				const Maths::HVec3 upVector = g_CubeMapVectorTable[1 + U32(face) * 3];
//				const Maths::HVec3 xVector =g_CubeMapVectorTable[2 + U32(face) * 3];
//
//				const F32 halfLightCubeMapSize = F32(GameWorld_LightCubeMapSize - 1)/2.f;
//				//for each pixel
//				for (U32 y = 0; y < GameWorld_LightCubeMapSize; y++)
//				{
//					for (U32 x = 0; x < GameWorld_LightCubeMapSize; x++)
//					{
//						Maths::HVec3 pixelVector = dirVector - xVector * (F32(x) - halfLightCubeMapSize) / halfLightCubeMapSize - upVector * (F32(y) - halfLightCubeMapSize) / halfLightCubeMapSize;
//						pixelVector.Normalise();
//
//						//Calculate rim generator values
//						Maths::HVec3 pixelValue(0.f, 0.f, 0.f);
//
//						for (U32 j = 0; j < GameWorld_MaxRimGenerators; j++)
//						{
//							//Dot product
//							Maths::HF32 dotProduct = pixelVector | rimGeneratorDirVector[j];
//							Maths::HF32 factor = ((dotProduct > 0.f)?dotProduct * dotProduct * dotProduct:Maths::HF32(0.f)) * rimGeneratorAttenuationVector[j];
//
//							pixelValue += factor * rimGeneratorVector[j].m_Colour;
//						}
//
//						//Convert to RGBM (Max 8.f)
//						Maths::HF32 maxValue = Maths::Max(pixelValue.X(), Maths::Max(pixelValue.Y(), pixelValue.Z()));
//						maxValue = Maths::Max(0.25f, Maths::Min(8.f, maxValue));
//
//						Maths::C_RGBA_F32 pixelValueColour;
//						//Normalize
//						pixelValueColour.R(Maths::Min(1.f,pixelValue.X() / (maxValue)));
//						pixelValueColour.G(Maths::Min(1.f,pixelValue.Y() / (maxValue)));
//						pixelValueColour.B(Maths::Min(1.f,pixelValue.Z() / (maxValue)));
//						pixelValueColour.A(maxValue / 8.f);
//
//						*pLockedMemory = Maths::RGBA(pixelValueColour);
//						//*pLockedMemory = Maths::RGBA(x,y,U32(face),1);
//						pLockedMemory++;
//					}
//
//					pLockedMemory += (pitch - GameWorld_LightCubeMapSize);
//				}
//#ifdef PLATFORM_SPU
//				//DMA out to the texture
//				Spu::StoreAsync(lightCubeInfo[i].m_MemoryLocked[face], pInitLockedMemory, pitch * GameWorld_LightCubeMapSize * 4, 1);
//#endif
//			}
//		}
//		Spu::Wait(1);
//	}
//} // namespace Game


	void CarUpdateLightCubeMapTask(Scheduler::Task* pTask)
	{
		PERF_EVENT("CarUpdateLightCubeMapTask");

		//Get data
#ifdef PLATFORM_SPU
		ALIGN16 U8 m_Buffer[sizeof(Game::C_CarUpdateLightCubeMapTaskInit)+15 & ~0xf];
		//new(&m_Buffer[0]) Game::C_CarUpdateLightCubeMapTaskInit();
		Game::C_CarUpdateLightCubeMapTaskInit* init = (Game::C_CarUpdateLightCubeMapTaskInit*)&m_Buffer[0];
		Spu::Fetch(init, pTask->Input, sizeof(Game::C_CarUpdateLightCubeMapTaskInit));
		const Game::C_CarUpdateLightCubeMapTaskInit* pInit = init;
		Spu::Wait(0);
#else
		const Game::C_CarUpdateLightCubeMapTaskInit* pInit = (Game::C_CarUpdateLightCubeMapTaskInit*)pTask->Input;
#endif
		const U32 numLightCubeMaps = pInit->m_NumLightCubeMaps;
		const C_LightCubeMapInfo* lightCubeInfo = pInit->m_LightCubeMapInfo;

#ifdef PLATFORM_SPU
		BIZ_STATIC_ASSERT((sizeof(C_RimGenerator) * GameWorld_MaxRimGenerators) % 16 == 0);
		ALIGN16 C_RimGenerator rimGeneratorVector[GameWorld_MaxRimGenerators];
		Spu::Fetch(rimGeneratorVector, pInit->m_RimGeneratorVector, sizeof(rimGeneratorVector));
#else
		const C_RimGenerator*	rimGeneratorVector = pInit->m_RimGeneratorVector;
#endif
		//Get range
		const U32 firstLightCubeMap = pTask->ParamData[0];
		const U32 lastLightCubeMap = pTask->ParamData[1];

		//Data
		Maths::HVec3 rimGeneratorDirVector[GameWorld_MaxRimGenerators];
		Maths::HF32	 rimGeneratorAttenuationVector[GameWorld_MaxRimGenerators];
		const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 quarter(Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HVec3 vOne(Maths::Constants<Maths::HVec3>::One());
		const Maths::HF32 tenth(0.1f);
		const Maths::HF32 eight(8.f);
		const Maths::HF32 eighth(1.f/8.f);

		const Maths::HF32 halfLightCubeMapSize = F32(GameWorld_LightCubeMapSize - 1)/2.f;
		const Maths::HF32 reciprocalHalfLightCubeMapSize(Maths::Reciprocal(halfLightCubeMapSize));


		//for each cubemap
		for (U32 i = firstLightCubeMap; i < lastLightCubeMap; i++)
		{
			PERF_EVENT("UpdateLightCubeMap");
			//Calculate rim generator vectors
			for (U32 jj = 0; jj < GameWorld_MaxRimGenerators; jj++)
			{
				rimGeneratorDirVector[jj] = (rimGeneratorVector[jj].m_Position - lightCubeInfo[i].m_Position);

				const Maths::HF32 recipDistant(rimGeneratorDirVector[jj].ReciprocalLength());
				rimGeneratorDirVector[jj] *= recipDistant;
				//Check if it has direction
				const Maths::HF32 scaleValue(two * (rimGeneratorDirVector[jj] | -rimGeneratorVector[jj].m_Direction) - one);
				const Maths::HF32 factor(Maths::Select(rimGeneratorVector[jj].m_Direction.Length2().GT(tenth), 
					scaleValue.Clamp(zero, one),
					one));

				rimGeneratorDirVector[jj] *= factor;

				//Attenuation
				//F32 distantAttenuation = Maths::Clamp((1.f - ((distant * distant) / (rimGeneratorVector[jj].m_Radius * rimGeneratorVector[jj].m_Radius))), 0.f, 1.f);
				const Maths::HF32 clampValue(one - Maths::Reciprocal(recipDistant * rimGeneratorVector[jj].m_Radius));
				const Maths::HF32 distantAttenuation = clampValue.Clamp(zero, one);
				const Maths::HF32 closeAttenuation = Maths::Reciprocal(recipDistant).Clamp(zero, one);
				const Maths::HF32 downAttenuation = (one + rimGeneratorDirVector[jj].Y()).Clamp(zero, one);

				rimGeneratorAttenuationVector[jj] =  distantAttenuation * closeAttenuation * downAttenuation;
			}

			//for each face
			for (U32 face = 0; face < 6; face++ )
			{
				const U32 pitch = lightCubeInfo[i].m_Pitch / 4;

				#ifdef PLATFORM_SPU
				U32* pInitLockedMemory = (U32*)Spu::Alloc(pitch * GameWorld_LightCubeMapSize * 4);
				U32* pLockedMemory = pInitLockedMemory;
				#else
				U32* pLockedMemory = (U32*)lightCubeInfo[i].m_MemoryLocked[face];
				#endif

				const Maths::HVec3 dirVector = g_CubeMapVectorTable[0 + U32(face) * 3];
				const Maths::HVec3 upVector = g_CubeMapVectorTable[1 + U32(face) * 3];
				const Maths::HVec3 xVector =g_CubeMapVectorTable[2 + U32(face) * 3];

				//for each pixel
				Maths::HF32 fY(zero);
				for (U32 y = 0; y < GameWorld_LightCubeMapSize; y++, fY += one)
				{
					Maths::HF32 fX(zero);
					for (U32 x = 0; x < GameWorld_LightCubeMapSize; x++, fX += one)
					{
						Maths::HVec3 pixelVector = dirVector - xVector * (fX - halfLightCubeMapSize) * reciprocalHalfLightCubeMapSize - upVector * (fY - halfLightCubeMapSize) * reciprocalHalfLightCubeMapSize;
						pixelVector.Normalise();

						//Calculate rim generator values
						Maths::HVec3 pixelValue(zero);

						for (U32 j = 0; j < GameWorld_MaxRimGenerators; j++)
						{
							//Dot product
							Maths::HF32 dotProduct = pixelVector | rimGeneratorDirVector[j];
							Maths::HF32 factor = Maths::Select(dotProduct.GT(zero), dotProduct * dotProduct * dotProduct, zero) * rimGeneratorAttenuationVector[j];

							pixelValue += factor * rimGeneratorVector[j].m_Colour;
						}

						if (lightCubeInfo[i].m_FloatRGBM)
						{
							const Maths::C_RGBA_F32 radiance((Maths::HVec4(pixelValue)));
							float encoded[4];
							EncodeExtendedVehicleLightRGBM(radiance.R(), radiance.G(), radiance.B(), encoded);
							// [Blur2PC][HDR] RGBA16F has RGBA order and two U32 words per texel.
							U16* pixel = (U16*)pLockedMemory;
							pixel[0] = Half::F16(encoded[0]).m_U16;
							pixel[1] = Half::F16(encoded[1]).m_U16;
							pixel[2] = Half::F16(encoded[2]).m_U16;
							pixel[3] = Half::F16(encoded[3]).m_U16;
							pLockedMemory += 2;
						}
						else
						{
							//Convert to RGBM (Max 8.f)
							Maths::HF32 maxValue = Maths::Max(pixelValue.X(), Maths::Max(pixelValue.Y(), pixelValue.Z()));
							maxValue = Maths::Max(quarter, Maths::Min(eight, maxValue));

							Maths::HVec4 clampedPixelValueColour(Maths::Min(vOne, pixelValue/maxValue));
							clampedPixelValueColour.W(maxValue * eighth);

							const Maths::C_RGBA_F32 pixelValueColour(clampedPixelValueColour);

							U8 r = (U8)(pixelValueColour.R() * 255.0f);
							U8 g = (U8)(pixelValueColour.G() * 255.0f);
							U8 b = (U8)(pixelValueColour.B() * 255.0f);
							U8 a = (U8)(pixelValueColour.A() * 255.0f);

							*pLockedMemory = (a << 24) + (r << 16) + (g << 8) + (b << 0);
							//*pLockedMemory = Maths::RGBA(x,y,U32(face),1);
							pLockedMemory++;
						}
					}

					// [Blur2PC][HDR] Row pitch is bytes; float RGBM consumes two words per pixel.
					pLockedMemory += pitch - GameWorld_LightCubeMapSize * (lightCubeInfo[i].m_FloatRGBM ? 2 : 1);
				}
#ifdef PLATFORM_SPU
				//DMA out to the texture
				Spu::StoreAsync(lightCubeInfo[i].m_MemoryLocked[face], pInitLockedMemory, pitch * GameWorld_LightCubeMapSize * 4, 1);
#endif
			}
		}
		Spu::Wait(1);
	}
} // namespace Game

JQ_DECLARE_PPU_MODULE(CarDynamicEffectsTaskModule, Game::CarDynamicEffectsTask);
JQ_DECLARE_SPU_MODULE(CarUpdateLightCubeMapTaskModule, Game::CarUpdateLightCubeMapTask, carupdatelightcubemap, "UpdateLightCube" );

