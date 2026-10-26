#ifndef INGAME_SPECIALFX_MANAGER_H
#define INGAME_SPECIALFX_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared\Messaging\MessageMultiHandler.h>

#include <Game/GameUtil/ResourceBag.h>

#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXEnums.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXBase.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXRearWheel.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXVehicle.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXVehicleToMatrix.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXTypes/InGameSpecialFXWorldPos.h>

namespace Game
{
	class C_InGameSpecialFXManager : public Messaging::C_MessageMultiHandler
	{
	public:

		C_InGameSpecialFXManager();
		~C_InGameSpecialFXManager();

		static C_InGameSpecialFXManager& Instance(){return s_instance;};

		void Init();
		void Update();
		void ClearFXStore();
		void CreateSpecialFX(const XtArray<RLPickupSpecialFX>& sfx, Maths::HMat43 matrix, int firingIdx, int targetIdx, U32 id, E_InGameSpecialFX_Trigger priority, 
			float liveTime = -1.f, Maths::HVec3 scale = Maths::HVec3(1,1,1), Maths::HVec3 offset = Maths::HVec3(), bool mExtraOption = false);
		void CreateSpecialFX(const XtArray<RLPickupSpecialFX>& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, Maths::HVec3 planePos, Maths::C_Normal planeNormal, int firingIdx, int targetIdx, U32 id, E_InGameSpecialFX_Trigger priority, 
			float liveTime = -1.f, Maths::HVec3 scale = Maths::HVec3(1,1,1), Maths::HVec3 offset = Maths::HVec3(), bool mExtraOption = false);

		void SetUpVehicleSFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, int vehIdx, U32 id, E_InGameSpecialFX_Trigger trigger, float liveTime, 
								Maths::HVec3 scale, Maths::HVec3 offset);
		void SetUpVehicleSFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, int vehIdx, U32 id, E_InGameSpecialFX_Trigger trigger, float liveTime, 
			Maths::HVec3 scale, Maths::HVec3 offset);

		void SetUpWorldSFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, int vehIdx, U32 id, float liveTime, Maths::HVec3 scale);
		void SetUpWorldSFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, Maths::HVec3 planePos, Maths::C_Normal planeNormal, int vehIdx, U32 id, float liveTime, Maths::HVec3 scale);

		void SetUpRearWheelFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, int vehIdx, U32 id, E_InGameSpecialFX_Trigger trigger, 
			float liveTime, Maths::HVec3 scale, Maths::HVec3 offset, bool leftWheel);
		void SetUpRearWheelFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, int vehIdx, U32 id, E_InGameSpecialFX_Trigger trigger, 
			float liveTime, Maths::HVec3 scale, Maths::HVec3 offset, bool leftWheel);

		void SetUpVehicleToMatrixFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, int vehIdx,  U32 id, 
			E_InGameSpecialFX_Trigger trigger, float liveTime, Maths::HVec3 scale, Maths::HVec3 offset);
		void SetUpVehicleToMatrixFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, Maths::HVec3 velocity, int vehIdx,  U32 id, 
			E_InGameSpecialFX_Trigger trigger, float liveTime, Maths::HVec3 scale, Maths::HVec3 offset);

		void Insert(C_InGameSpecialFXBase* fxToInsert, Maths::HMat43 matrix, Maths::HVec3 velocity, Maths::HVec3 planePos, Maths::C_Normal planeNormal);

		void DestroyPerkForced(U32 id);
		void DestroyPerk(U32 id);
		void DestroyPerkMesh(U32 id);
		void UpdatePerkMatrix(U32 id, Maths::HMat43 matrix, bool immediateUpdate = false);

		void StartPhotoMode();
		void UpdatePhotoMode();
		void EndPhotoMode();

		void ForceChaseCam(int viewport);
		void CeaseForceChaseCam(int viewport);

		void CopySFXState(U32 myId, U32 hisId);
		void SetState(U32 id, E_InGameSpecialFX_Mode state);
		void SetCurrentTime(U32 id, float time);
		void SetIntensity(U32 id, float time);

		bool IsCamInternal(int vehIdx, int firerViewport);
		bool UsingLocalViewport(int vehIdx);
		void CullVehicleSFX(C_InGameSpecialFXVehicle* fxToInsert);

		// For game messages
		bool MessageHandler(const Messaging::C_MessageBase* pMessage);

		// stuff outside this class allocates these
		Btl::DynamicPool<C_InGameSpecialFXVehicle>			mVehiclePool;

	private:
		static C_InGameSpecialFXManager		s_instance;
		bool								mInPhotoMode;

		Btl::Vector<C_InGameSpecialFXBase*>	mCurrentSFXStore;

		void Delete(C_InGameSpecialFXBase* fxDelete);

		// we use dynamic pools so our allocations are really cheap
		Btl::DynamicPool<C_InGameSpecialFXWorldPos>			mWorldPosPool;
		Btl::DynamicPool<C_InGameSpecialFXRearWheel>		mRearWheelPool;
		Btl::DynamicPool<C_InGameSpecialFXVehicleToMatrix>	mVehicleToMatrixPool;
	};

	INLINE void C_InGameSpecialFXManager::CreateSpecialFX(const XtArray<RLPickupSpecialFX>& sfx, Maths::HMat43 matrix, int firingIdx, int targetIdx, U32 id, E_InGameSpecialFX_Trigger priority, 
		float liveTime, Maths::HVec3 scale, Maths::HVec3 offset, bool mExtraOption)
	{
		CreateSpecialFX(sfx, matrix, Maths::HVec3(), Maths::HVec3(), Maths::C_Normal(), firingIdx, targetIdx, id, priority, liveTime, scale, offset, mExtraOption);
	}

	INLINE void C_InGameSpecialFXManager::SetUpVehicleSFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, int vehIdx, U32 id, E_InGameSpecialFX_Trigger trigger, float liveTime, 
		Maths::HVec3 scale, Maths::HVec3 offset)
	{
		SetUpVehicleSFX(sfx, matrix, Maths::HVec3(), vehIdx, id, trigger, liveTime, scale, offset);
	}

	INLINE void C_InGameSpecialFXManager::SetUpWorldSFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, int vehIdx, U32 id, float liveTime, Maths::HVec3 scale)
	{
		SetUpWorldSFX(sfx, matrix, Maths::HVec3(), Maths::HVec3(), Maths::C_Normal(), vehIdx, id, liveTime, scale);
	}

	INLINE void C_InGameSpecialFXManager::SetUpRearWheelFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, int vehIdx, U32 id, E_InGameSpecialFX_Trigger trigger, 
		float liveTime, Maths::HVec3 scale, Maths::HVec3 offset, bool leftWheel)
	{
		SetUpRearWheelFX(sfx, matrix, Maths::HVec3(), vehIdx, id, trigger, liveTime, scale, offset, leftWheel);
	}

	INLINE void C_InGameSpecialFXManager::SetUpVehicleToMatrixFX(const RLPickupSpecialFX& sfx, Maths::HMat43 matrix, int vehIdx,  U32 id, 
		E_InGameSpecialFX_Trigger trigger, float liveTime, Maths::HVec3 scale, Maths::HVec3 offset)
	{	
		SetUpVehicleToMatrixFX(sfx, matrix, Maths::HVec3(), vehIdx, id, trigger, liveTime, scale, offset);
	}

	// ----------------------------------------------------------------------------------------------------

	struct SpecialFxTriggerSort
	{
	public:
		SpecialFxTriggerSort() {};

		bool operator()(C_InGameSpecialFXVehicle* lhs, C_InGameSpecialFXVehicle* rhs)
		{
			bool lhsIsYounger = (lhs->mTrigger == rhs->mTrigger && lhs->mTotalElapsedTime < rhs->mTotalElapsedTime);
			if( lhs->mTrigger < rhs->mTrigger || lhsIsYounger )
				return true;

			return false;
		}
	};

} // namespace Game

#endif // INGAME_SPECIALFX_MANAGER_H
