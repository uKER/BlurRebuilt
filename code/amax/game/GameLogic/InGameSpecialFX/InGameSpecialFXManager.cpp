#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

#include <Game\GameWorld\GameWorld.h>
#include <Game\Messaging\GameflowMessages.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <Game\Messaging\GameMessages.h>
#include <Game\Messaging\PickupActorMessages.h>

#include <Game\AmaxCam\CameraSystem.h>
#include <Game\AmaxCam\VehicleFeed.h>

#include <Game\GameWorld\ViewType.h>
#include <algorithm>

#include "Automation\GamePerfLogging.h"
#include <SpecialFX/SpecialFX.h>

#include <vehicle/input.h>

Debug::Logger &g_InGameSFXLogger( Debug::GetLogger("SpecialFX") );
Debug::Int g_debugSFXCullCount("SpecialFXManager.CullCount", 4);

namespace Game
{
	C_InGameSpecialFXManager C_InGameSpecialFXManager::s_instance;

	// -----------------------------------------------------------------------

	C_InGameSpecialFXManager::C_InGameSpecialFXManager() : mInPhotoMode(false)
	{
		
	}

	C_InGameSpecialFXManager::~C_InGameSpecialFXManager()
	{
		ClearFXStore();
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::ClearFXStore()
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
			Delete(mCurrentSFXStore[i]);

		mCurrentSFXStore.Destroy();
	}

	void C_InGameSpecialFXManager::Init()
	{
		// Most of these we just directly call the messagehandler. Dirty, yes, but faster.
		SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
		AddSubscriber( GameMessaging::E_GameMsg_FinalUpdate );
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::Insert(C_InGameSpecialFXBase* fxToInsert, Maths::HMat43 matrix, Maths::HVec3 velocity, Maths::HVec3 planePos, Maths::C_Normal planeNormal)
	{
		PERF_EVENT("InGameSFX.InsertSFX");
		if(fxToInsert)
		{
			Game::C_LevelState& levelState		= C_Game::GetInstance()->GetLevelState();

			if(fxToInsert->mViewport != -1 && fxToInsert->mViewport == fxToInsert->mFirerViewport && levelState.mNumViewports > 1)
			{
				if(fxToInsert->mBumperSpecialFX)
					fxToInsert->mBumperSpecialFX->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void*)fxToInsert->mViewport);

				if(fxToInsert->mChaseSpecialFX)
					fxToInsert->mChaseSpecialFX->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void*)fxToInsert->mViewport);
			}

			if(fxToInsert->mGlobalSpecialFX)
			{
				fxToInsert->mGlobalSpecialFX->Start();
				fxToInsert->mGlobalSpecialFX->Update(0.f);
				fxToInsert->mGlobalSpecialFX->UpdateWorldMatrix(matrix, velocity, planePos, planeNormal);
			}

			if(fxToInsert->mBumperSpecialFX)
			{
				fxToInsert->mBumperSpecialFX->Start();
				fxToInsert->mBumperSpecialFX->Update(0.f);
				fxToInsert->mBumperSpecialFX->UpdateWorldMatrix(matrix, velocity, planePos, planeNormal);
				
			}

			if(fxToInsert->mChaseSpecialFX)
			{
				fxToInsert->mChaseSpecialFX->Start();
				fxToInsert->mChaseSpecialFX->Update(0.f);
				fxToInsert->mChaseSpecialFX->UpdateWorldMatrix(matrix, velocity, planePos, planeNormal);
			}	

			if(fxToInsert->mViewport != -1 && levelState.mNumViewports > 1)
			{
				U32 params[2];
				params[0] = 1 << (Game::E_ViewType_Main1 + fxToInsert->mViewport);
				params[1] = Game::E_ViewTypeFlag_Main;

				if(fxToInsert->mBumperSpecialFX)
					fxToInsert->mBumperSpecialFX->SetParameter(SpecialFX::E_ParamID_SetMeshViewFlags, (void*)params);

				if(fxToInsert->mChaseSpecialFX)
					fxToInsert->mChaseSpecialFX->SetParameter(SpecialFX::E_ParamID_SetMeshViewFlags, (void*)params);
			}

			mCurrentSFXStore.PushBack(fxToInsert);
		}
	}

	// -----------------------------------------------------------------------

	bool C_InGameSpecialFXManager::UsingLocalViewport(int vehIdx)
	{
		Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Game::C_LevelState& levelState		= C_Game::GetInstance()->GetLevelState();
		int viewportIdx						= pRLC->GetViewportIndex(vehIdx);

		return (viewportIdx < (int)levelState.mNumViewports);
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::CullVehicleSFX(C_InGameSpecialFXVehicle* fxToInsert)
	{
		Btl::Vector<C_InGameSpecialFXVehicle*> vehFxList;
		vehFxList.PushBack(fxToInsert);

		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* currentFx = mCurrentSFXStore[i];

			if(currentFx->GetType() == E_InGameSpecialFXBase_Vehicle && fxToInsert->GetType() == E_InGameSpecialFXBase_Vehicle && 
				currentFx->mVehicleIndex == fxToInsert->mVehicleIndex )
			{
				C_InGameSpecialFXVehicle* currentVehFx = (C_InGameSpecialFXVehicle*)currentFx;
				vehFxList.PushBack(currentVehFx);
			}
		}

		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		int numToCull = Maths::Floor(g_debugSFXCullCount * levelState.mNumViewports);
		if(numToCull <= vehFxList.Size())
		{
			std::sort(vehFxList.Begin(), vehFxList.End(), SpecialFxTriggerSort());

			for( int i = numToCull; i < vehFxList.Size(); i++ )
			{
				C_InGameSpecialFXVehicle* currentCull = vehFxList[i];

				if(vehFxList[i]->mBumperSpecialFX)
					vehFxList[i]->mBumperSpecialFX->SetParameter(SpecialFX::E_ParamID_KillMeshEffect, NULL);

				if(vehFxList[i]->mChaseSpecialFX)
					vehFxList[i]->mChaseSpecialFX->SetParameter(SpecialFX::E_ParamID_KillMeshEffect, NULL);

				if(vehFxList[i]->mGlobalSpecialFX)
					vehFxList[i]->mGlobalSpecialFX->SetParameter(SpecialFX::E_ParamID_KillMeshEffect, NULL);
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::SetUpVehicleSFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, int vehIdx, 
		U32 id, E_InGameSpecialFX_Trigger trigger, float liveTime, Maths::HVec3 scale, Maths::HVec3 offset)
	{
		PERF_EVENT("InGameSFX.SetUpVehicleSFX");
		C_InGameSpecialFXVehicle* fxToInsert	= NULL;
		Game::C_LevelState& levelState			= C_Game::GetInstance()->GetLevelState();
		Game::Actors::RaceLogicCore* pRLC		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		int firerViewport						= pRLC->GetViewportIndex(vehIdx);
		Maths::HVec3 planePos(Maths::UNDEFINED);
		Maths::C_Normal planeNormal(Maths::UNDEFINED);

		float lifetime = 0.f;
		if(liveTime >= 0.f)
			lifetime = liveTime;

		for( unsigned int i = 0; i < levelState.mNumViewports; i++ )
		{
			if(firerViewport != i) // Show the external FX only on other vehicles
			{
				if(sfx.mExternalSpecialFX || sfx.mGlobalSpecialFX)
				{
					fxToInsert = mVehiclePool.AllocateNoConstructor();
					new (fxToInsert) C_InGameSpecialFXVehicle(sfx, vehIdx, id, lifetime, i, firerViewport, trigger, scale, offset);
					fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
					Insert(fxToInsert, matrix, velocity, planePos, planeNormal);	
					CullVehicleSFX(fxToInsert);
				}
			}
			else
			{
				if(IsCamInternal(vehIdx, firerViewport))
				{
					if(sfx.mInternalSpecialFX || sfx.mGlobalSpecialFX)
					{
						// Show the cam FX just to the firing vehicle, and only if we are in bumpercam.
						fxToInsert = mVehiclePool.AllocateNoConstructor();
						new (fxToInsert) C_InGameSpecialFXVehicle(sfx, vehIdx, id, lifetime, firerViewport, firerViewport, trigger, scale, offset);
						fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
						Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
						CullVehicleSFX(fxToInsert);
					}
				}
				else if(sfx.mLocalSpecialFX || sfx.mGlobalSpecialFX)
				{
					// Show the local FX only to the firing vehicle.
					fxToInsert = mVehiclePool.AllocateNoConstructor();
					new (fxToInsert) C_InGameSpecialFXVehicle(sfx, vehIdx, id, lifetime, firerViewport, firerViewport, trigger, scale, offset);
					fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
					Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
					CullVehicleSFX(fxToInsert);
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::SetUpWorldSFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, Maths::HVec3 planePos, Maths::C_Normal planeNormal, int vehIdx, U32 id, float liveTime, Maths::HVec3 scale)
	{
		PERF_EVENT("InGameSFX.SetUpWorldSFX");
		C_InGameSpecialFXBase* fxToInsert	= NULL;
		Game::C_LevelState& levelState		= C_Game::GetInstance()->GetLevelState();
		Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		int firerViewport					= pRLC->GetViewportIndex(vehIdx);

		float lifetime = 0.f;
		if(liveTime >= 0.f)
			lifetime = liveTime;
		else
		{
			if(sfx.mDurationLoop.mEnd > 0.f)
				lifetime = sfx.mDurationLoop.mEnd;
			else
				lifetime = sfx.mDurationIntro.mEnd;
		}

		for( unsigned int i = 0; i < levelState.mNumViewports; i++ )
		{
			if(firerViewport != i) // Show the external FX only on other vehicles
			{
				if(sfx.mExternalSpecialFX || sfx.mGlobalSpecialFX)
				{
					fxToInsert = mWorldPosPool.AllocateNoConstructor();
					new (fxToInsert) C_InGameSpecialFXWorldPos(sfx, matrix, vehIdx, id, lifetime, i, firerViewport, scale);
					Insert(fxToInsert, matrix, velocity, planePos, planeNormal);	
				}
			}
			else
			{
				if(IsCamInternal(vehIdx, firerViewport))
				{
					if(sfx.mInternalSpecialFX || sfx.mGlobalSpecialFX)
					{
						// Show the cam FX just to the firing vehicle, and only if we are in bumpercam.
						fxToInsert = mWorldPosPool.AllocateNoConstructor();
						new (fxToInsert) C_InGameSpecialFXWorldPos(sfx, matrix, vehIdx, id, lifetime, -1, firerViewport);
						Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
					}
				}
				else if(sfx.mLocalSpecialFX || sfx.mGlobalSpecialFX)
				{
					// Show the local FX only to the firing vehicle.
					fxToInsert = mWorldPosPool.AllocateNoConstructor();
					new (fxToInsert) C_InGameSpecialFXWorldPos(sfx, matrix, vehIdx, id, lifetime, firerViewport, firerViewport, scale);
					Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::SetUpRearWheelFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, int vehIdx, 
		U32 id, E_InGameSpecialFX_Trigger trigger, float liveTime, Maths::HVec3 scale, Maths::HVec3 offset, bool leftWheel)
	{
		PERF_EVENT("InGameSFX.SetUpRearWheelFX");
		C_InGameSpecialFXRearWheel* fxToInsert	= NULL;
		Game::C_LevelState& levelState			= C_Game::GetInstance()->GetLevelState();
		Game::Actors::RaceLogicCore* pRLC		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		int firerViewport						= pRLC->GetViewportIndex(vehIdx);
		Maths::HVec3 planePos(Maths::UNDEFINED);
		Maths::C_Normal planeNormal(Maths::UNDEFINED);

		float lifetime = 0.f;
		if(liveTime >= 0.f)
			lifetime = liveTime;

		for( unsigned int i = 0; i < levelState.mNumViewports; i++ )
		{
			if(firerViewport != i) // Show the external FX only on other vehicles
			{
				if(sfx.mExternalSpecialFX || sfx.mGlobalSpecialFX)
				{
					fxToInsert = mRearWheelPool.AllocateNoConstructor();
					new (fxToInsert) C_InGameSpecialFXRearWheel(sfx, matrix, vehIdx, id, lifetime, i, firerViewport, scale, leftWheel);
					fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
					Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
				}
			}
			else
			{
				if(IsCamInternal(vehIdx, firerViewport))
				{
					if(sfx.mInternalSpecialFX || sfx.mGlobalSpecialFX)
					{
						// Show the cam FX just to the firing vehicle, and only if we are in bumpercam.
						fxToInsert = mRearWheelPool.AllocateNoConstructor();
						new (fxToInsert) C_InGameSpecialFXRearWheel(sfx, matrix, vehIdx, id, lifetime, -1, firerViewport, scale, leftWheel);
						fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
						Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
					}
				}
				else if(sfx.mLocalSpecialFX || sfx.mGlobalSpecialFX)
				{
					// Show the local FX only to the firing vehicle.
					fxToInsert = mRearWheelPool.AllocateNoConstructor();
					new (fxToInsert) C_InGameSpecialFXRearWheel(sfx, matrix, vehIdx, id, lifetime, firerViewport, firerViewport, scale, leftWheel);
					fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
					Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::SetUpVehicleToMatrixFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, int vehIdx, 
		U32 id, E_InGameSpecialFX_Trigger trigger, float liveTime, Maths::HVec3 scale, Maths::HVec3 offset)
	{
		PERF_EVENT("InGameSFX.SetUpVehicleToMatrixFX");
		C_InGameSpecialFXVehicleToMatrix* fxToInsert	= NULL;
		Game::C_LevelState& levelState			= C_Game::GetInstance()->GetLevelState();
		Game::Actors::RaceLogicCore* pRLC		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		int firerViewport						= pRLC->GetViewportIndex(vehIdx);
		Maths::HVec3 planePos(Maths::UNDEFINED);
		Maths::C_Normal planeNormal(Maths::UNDEFINED);

		float lifetime = 0.f;
		if(liveTime >= 0.f)
			lifetime = liveTime;

		for( unsigned int i = 0; i < levelState.mNumViewports; i++ )
		{
			if(firerViewport != i) // Show the external FX only on other vehicles
			{
				if(sfx.mExternalSpecialFX || sfx.mGlobalSpecialFX)
				{
					fxToInsert = mVehicleToMatrixPool.AllocateNoConstructor();
					new (fxToInsert) C_InGameSpecialFXVehicleToMatrix(sfx, matrix, vehIdx, id, lifetime, i, firerViewport, scale);
					fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
					Insert(fxToInsert, matrix, velocity, planePos, planeNormal);	
				}
			}
			else
			{
				if(IsCamInternal(vehIdx, firerViewport))
				{
					if(sfx.mInternalSpecialFX || sfx.mGlobalSpecialFX)
					{
						// Show the cam FX just to the firing vehicle, and only if we are in bumpercam.
						fxToInsert = mVehicleToMatrixPool.AllocateNoConstructor();
						new (fxToInsert) C_InGameSpecialFXVehicleToMatrix(sfx, matrix, vehIdx, id, lifetime, -1, firerViewport, scale);
						fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
						Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
					}
				}
				else if(sfx.mLocalSpecialFX || sfx.mGlobalSpecialFX)
				{
					// Show the local FX only to the firing vehicle.
					fxToInsert = mVehicleToMatrixPool.AllocateNoConstructor();
					new (fxToInsert) C_InGameSpecialFXVehicleToMatrix(sfx, matrix, vehIdx, id, lifetime, -1, firerViewport, scale);
					fxToInsert->GetMatrixVelocityCollision(matrix, velocity, planePos, planeNormal);
					Insert(fxToInsert, matrix, velocity, planePos, planeNormal);
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::Update()
	{
		PERF_EVENT("InGameSFX.Update");
		float currentTime = C_GameApp::GetInstance()->GetTimeStep();

		// -----------------
		// Delete expired FX
		// -----------------
		for( int a = 0; a < mCurrentSFXStore.Size(); a++ )
		{
			C_InGameSpecialFXBase* pSpecialFX = mCurrentSFXStore[a];

			if(pSpecialFX)
			{
				bool deleteFX = false;
				if(pSpecialFX->mCurrentTime == -1.f)
				{
					deleteFX = true;
				}
				else
				{
					pSpecialFX->mCurrentTime += currentTime;
					pSpecialFX->mTotalElapsedTime += currentTime;

					switch(pSpecialFX->mCurrentMode)
					{
					case E_InGameSpecialFX_Mode_Intro:
						{
							if(pSpecialFX->mCurrentTime >= pSpecialFX->mIntroEnd)
							{
								if(pSpecialFX->mLoopStart != pSpecialFX->mLoopEnd)
									pSpecialFX->StartLooping();
								else if(pSpecialFX->mOutroStart != pSpecialFX->mOutroEnd)
									pSpecialFX->StartOutro();
								else
									deleteFX = true;
							}
						}
						break;

					case E_InGameSpecialFX_Mode_Looping:
						{
							if(pSpecialFX->mLifetime == 0.f)
							{
								if(pSpecialFX->mCurrentTime >= pSpecialFX->mLoopEnd)
									pSpecialFX->StartOutro();		
							}
							else
							{
								float outroDuration = pSpecialFX->mOutroEnd - pSpecialFX->mOutroStart;

								// We loop forever if we don't have an outro, otherwise we loop until our lifetime is up
								if(outroDuration == 0.f)
								{
									if(pSpecialFX->mCurrentTime >= pSpecialFX->mLoopEnd)
										pSpecialFX->mCurrentTime = pSpecialFX->mLoopStart;
								}
								else
								{
									if(pSpecialFX->mTotalElapsedTime >= pSpecialFX->mLifetime - outroDuration)
										pSpecialFX->StartOutro();
									else if(pSpecialFX->mCurrentTime >= pSpecialFX->mLoopEnd)
										pSpecialFX->mCurrentTime = pSpecialFX->mLoopStart;
								}
							}
						}
						break;

					case E_InGameSpecialFX_Mode_Outro:
						{
							if(pSpecialFX->mCurrentTime >= pSpecialFX->mOutroEnd)
								deleteFX = true;
						}
						break;
					}
				}

				if(deleteFX)
				{
					Delete(mCurrentSFXStore[a]);
					// this is much quicker if we don't care about the order
					mCurrentSFXStore.EraseByIndexNonOrdered(a);
					a--;
				}
			}
		}

		// Record how many perk's are running
		AmaxPerfLogging::AccumulateDuration(AmaxPerfLogging::kPerkInfo, mCurrentSFXStore.Size());

		// ---------
		// Update FX
		// ---------
		if(mCurrentSFXStore.Size())
		{
			int size = mCurrentSFXStore.Size();
			int i;
			for( i = 0; i < size - 1; i++ )
			{
				DCBT(0, mCurrentSFXStore[i+1]);
				mCurrentSFXStore[i]->Update();
			}
			mCurrentSFXStore[i]->Update();
		}
	}

	bool C_InGameSpecialFXManager::IsCamInternal(int vehIdx, int viewport)
	{
		Game::Actors::RaceLogicCore* pRLC		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Game::Actors::C_VehicleActor* pActor	= pRLC->GetVehicleActor(vehIdx);

		if(vehIdx != -1 && viewport != -1)
		{
			if(pActor)
			{
				AmaxCam::E_VFCamera camType = pActor->GetCameraType(viewport);
				return camType == AmaxCam::E_VFCamera_Bumper;
			}
			else
			{
				LOGWARN(g_InGameSFXLogger, "IsCamInternal - Trying to access an actor that does not exist.");
				return false;
			}
		}

		return false;
	}

	void C_InGameSpecialFXManager::CreateSpecialFX(const XtArray<RLPickupSpecialFX>& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, Maths::HVec3 planePos, Maths::C_Normal planeNormal, int firingIdx, 
		int targetIdx, U32 id, E_InGameSpecialFX_Trigger priority, float liveTime, Maths::HVec3 scale, Maths::HVec3 offset, bool mExtraOption)
	{
		for( int i = 0; i < sfx.count; i++ )
		{
			switch(sfx[i].mSpecialFXLocation)
			{
			case E_PICKUP_SPECIALFX_LOCATION_ACTINGCAR:
				{		
					SetUpVehicleSFX(sfx[i], matrix, velocity, firingIdx, id, priority, liveTime, scale, offset);
					break;
				}

			case E_PICKUP_SPECIALFX_LOCATION_TARGETCAR:
				{		
					SetUpVehicleSFX(sfx[i], matrix, velocity, targetIdx, id, priority, liveTime, scale, offset);
					break;
				}

			case E_PICKUP_SPECIALFX_LOCATION_TARGETCAR_FIXEDORIENTATION:
				{		
					SetUpVehicleSFX(sfx[i], matrix, velocity, targetIdx, id, priority, liveTime, scale, offset);
					break;
				}

			case E_PICKUP_SPECIALFX_LOCATION_WORLDPOS:
				{		
					SetUpWorldSFX(sfx[i], matrix, velocity, planePos, planeNormal, firingIdx, id, liveTime, scale);
					break;
				}
			case E_PICKUP_SPECIALFX_LOCATION_REARWHEELPOS:
				{
					SetUpRearWheelFX(sfx[i], matrix, velocity, firingIdx, id, priority, liveTime, scale, offset, mExtraOption);
					break;
				}
			case E_PICKUP_SPECIALFX_LOCATION_VEHICLETOMATRIX:
				{
					SetUpVehicleToMatrixFX(sfx[i], matrix, velocity, firingIdx, id, priority, liveTime, scale, offset);
					break;
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::StartPhotoMode()
	{
		mInPhotoMode = true;
		ForceChaseCam(-1);
	}

	void C_InGameSpecialFXManager::UpdatePhotoMode()
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* pSpecialFX = mCurrentSFXStore[i];
			if(pSpecialFX)
			{
				pSpecialFX->Update();
			}
		}	
	}

	void C_InGameSpecialFXManager::EndPhotoMode()
	{
		mInPhotoMode = false;
		CeaseForceChaseCam(-1);
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::ForceChaseCam(int viewport)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* pSpecialFX = mCurrentSFXStore[i];
			if(pSpecialFX && (pSpecialFX->mViewport == viewport || viewport == -1))
				pSpecialFX->ForceChaseCam();
		}
	}

	void C_InGameSpecialFXManager::CeaseForceChaseCam(int viewport)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* pSpecialFX = mCurrentSFXStore[i];
			if(pSpecialFX && (pSpecialFX->mViewport == viewport || viewport == -1))
				pSpecialFX->CeaseForceChaseCam();
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::DestroyPerk(U32 id)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* currentFx = mCurrentSFXStore[i];

			if(currentFx->mUniqueId == id)
			{
				bool remove = false;
				if(currentFx->GetType() == E_InGameSpecialFXBase_Vehicle)
				{
					C_InGameSpecialFXVehicle* currentVehFx = (C_InGameSpecialFXVehicle*)currentFx;
					if( (currentVehFx->mTrigger != E_InGameSpecialFX_Trigger_Charging && currentVehFx->mTrigger != E_InGameSpecialFX_Trigger_Firing))
					{
						remove = true;
					}
				}
				else
					remove = true;

				if(remove)
				{
					if(currentFx->mChaseSpecialFX)
						currentFx->mChaseSpecialFX->Stop();

					if(currentFx->mBumperSpecialFX)
						currentFx->mBumperSpecialFX->Stop();

					if(currentFx->mGlobalSpecialFX)
						currentFx->mGlobalSpecialFX->Stop();

					currentFx->mCurrentTime = -1.f;
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::DestroyPerkForced(U32 id)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* currentFx = mCurrentSFXStore[i];

			if(currentFx->mUniqueId == id)
			{
				
				if(currentFx->mChaseSpecialFX)
					currentFx->mChaseSpecialFX->Stop();

				if(currentFx->mBumperSpecialFX)
					currentFx->mBumperSpecialFX->Stop();

				if(currentFx->mGlobalSpecialFX)
					currentFx->mGlobalSpecialFX->Stop();

				currentFx->mCurrentTime = -1.f;
				
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::CopySFXState(U32 myId, U32 hisId)
	{
		C_InGameSpecialFXBase* mySFX = NULL;
		C_InGameSpecialFXBase* hisSFX = NULL;

		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* currentFx = mCurrentSFXStore[i];

			if(currentFx->mUniqueId == myId)
			{	
				mySFX = currentFx;
			}
			else if(currentFx->mUniqueId == hisId)
			{
				hisSFX = currentFx;
			}
		}

		if(mySFX && hisSFX)
		{
			mySFX->mCurrentMode = hisSFX->mCurrentMode;
			mySFX->mCurrentTime = hisSFX->mCurrentTime;
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::DestroyPerkMesh(U32 id)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* currentFx = mCurrentSFXStore[i];

			if(currentFx->mUniqueId == id)
			{
				if(currentFx->mChaseSpecialFX)
					currentFx->mChaseSpecialFX->SetParameter(SpecialFX::E_ParamID_KillMeshEffect, NULL);

				if(currentFx->mBumperSpecialFX)
					currentFx->mBumperSpecialFX->SetParameter(SpecialFX::E_ParamID_KillMeshEffect, NULL);

				if(currentFx->mGlobalSpecialFX)
					currentFx->mGlobalSpecialFX->SetParameter(SpecialFX::E_ParamID_KillMeshEffect, NULL);
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::UpdatePerkMatrix(U32 id, Maths::HMat43 matrix, bool immediateUpdate)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* currentFx = mCurrentSFXStore[i];

			if(currentFx->GetType() == E_InGameSpecialFXBase_WorldPos)
			{
				C_InGameSpecialFXWorldPos* targetFx = (C_InGameSpecialFXWorldPos*)currentFx;

				if(targetFx->mUniqueId == id)
				{
					targetFx->mMatrix = matrix;

					if(targetFx->mBumperSpecialFX)
						targetFx->mBumperSpecialFX->UpdateWorldMatrix(targetFx->mMatrix);

					if(targetFx->mChaseSpecialFX)
						targetFx->mChaseSpecialFX->UpdateWorldMatrix(targetFx->mMatrix);

					if(targetFx->mGlobalSpecialFX)
						targetFx->mGlobalSpecialFX->UpdateWorldMatrix(targetFx->mMatrix);
					
					if (immediateUpdate)
					{
						if(targetFx->mBumperSpecialFX)
							targetFx->mBumperSpecialFX->SetParameter(SpecialFX::E_ParamID_ImmediateUpdate, 0);

						if(targetFx->mChaseSpecialFX)
							targetFx->mChaseSpecialFX->SetParameter(SpecialFX::E_ParamID_ImmediateUpdate, 0);

						if(targetFx->mGlobalSpecialFX)
							targetFx->mGlobalSpecialFX->SetParameter(SpecialFX::E_ParamID_ImmediateUpdate, 0);
					}
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::SetState(U32 id, E_InGameSpecialFX_Mode state)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* currentFx = mCurrentSFXStore[i];
			C_InGameSpecialFXWorldPos* targetFx = (C_InGameSpecialFXWorldPos*)currentFx;

			if(targetFx->mUniqueId == id)
			{
				switch(state)
				{
				case E_InGameSpecialFX_Mode_Looping:
					targetFx->StartLooping();
					break;

				case E_InGameSpecialFX_Mode_Outro:
					targetFx->StartOutro();
					break;
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::SetCurrentTime(U32 id, float time)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* currentFx = mCurrentSFXStore[i];
			C_InGameSpecialFXWorldPos* targetFx = (C_InGameSpecialFXWorldPos*)currentFx;

			if(targetFx->mUniqueId == id)
			{
				targetFx->mCurrentTime = time;
			}
		}
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::SetIntensity(U32 id, float intensity)
	{
		for( int i = 0; i < mCurrentSFXStore.Size(); i++ )
		{
			C_InGameSpecialFXBase* targetFx = mCurrentSFXStore[i];
			
			if(targetFx->mUniqueId == id)
			{
				if( targetFx->mGlobalSpecialFX )
				{
					targetFx->mGlobalSpecialFX->UpdateIntensity(intensity);
				}
				if( targetFx->mBumperSpecialFX )
				{
					targetFx->mBumperSpecialFX->UpdateIntensity(intensity);
				}
				if( targetFx->mChaseSpecialFX )
				{
					targetFx->mChaseSpecialFX->UpdateIntensity(intensity);
				}
			}
		}
	}

	// -----------------------------------------------------------------------

	bool C_InGameSpecialFXManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
	{
		bool handledMessage = true;

		switch( pMessage->Type() )
		{
		case GameMessaging::E_GameMsg_FinalUpdate:
			{
				if(!C_GameApp::GetInstance()->IsPaused())
					Update();
				else if(mInPhotoMode)
					UpdatePhotoMode();

				break;
			}

		default:
			break;
		}

		return handledMessage;
	}

	// -----------------------------------------------------------------------

	void C_InGameSpecialFXManager::Delete(C_InGameSpecialFXBase* fxDelete)
	{
		switch(fxDelete->GetType())
		{
			case E_InGameSpecialFXBase_Vehicle:
				mVehiclePool.Free((C_InGameSpecialFXVehicle*)fxDelete);
				break;
			case E_InGameSpecialFXBase_WorldPos:
				mWorldPosPool.Free((C_InGameSpecialFXWorldPos*)fxDelete);
				break;
			case E_InGameSpecialFXBase_RearWheels:
				mRearWheelPool.Free((C_InGameSpecialFXRearWheel*)fxDelete);
				break;
			case E_InGameSpecialFXBase_VehicleToMatrix:
				mVehicleToMatrixPool.Free((C_InGameSpecialFXVehicleToMatrix*)fxDelete);
				break;
		}
	}
}
