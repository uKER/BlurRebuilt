#include "stdafx.h"

#include "LightPillarEffect.h"
#include <specialFX\SpecialFX.h>
#include <Game\GameWorld\GameWorld.h>
#include <Kanzen\Camera\Utility.h>
#include <GameApp.h>
#include <Game\GameUtil\ResourceBag.h>

extern Debug::Float gDestructModeDistanceEffectDamping;

namespace Game
{
	C_GameWorld *C_LightPillarEffect::m_pGameWorld = NULL;

	C_LightPillarEffect::C_LightPillarEffect() : m_pEffect(0)
	{
		mDistanceEffect = NULL;
		reset();
	}

	void 
		C_LightPillarEffect::reset()
	{
		if(mDistanceEffect)
		{
			mDistanceEffect->Stop();
			SAFE_DELETE_PTR(mDistanceEffect);
			m_pEffect = NULL;
		}
		mIntensity = 0.0f;
		mDistanceEffectTime = 0.0f;
	}

	bool 
		C_LightPillarEffect::active() const 
	{
		return mDistanceEffect?true:false;
	}

	void 
		C_LightPillarEffect::stop() 
	{
		reset();
	}

	bool 
		C_LightPillarEffect::visible() const 
	{
		return mIntensity > 0.01f;
	}

	void
		C_LightPillarEffect::update(
		F32 elapsedTime,
		F32 desiredIntensity,
		const Maths::HMat43* vehicleTransform
		)
	{
		if (mDistanceEffect)
		{
			mDistanceEffectTime += elapsedTime;
			if (mDistanceEffectTime > mDistanceEffect->GetDuration())
			{
				mDistanceEffectTime = 0.0f;
			}

			mDistanceEffect->Update(mDistanceEffectTime);

			if(vehicleTransform)
			{
				// Update the matrix
				Maths::HMat43 finalMatrix;
				finalMatrix.SetIdentity();
				finalMatrix.AW( vehicleTransform->AW() );

				mDistanceEffect->UpdateWorldMatrix(finalMatrix);
			}

			// Update the intensity
			mIntensity += (desiredIntensity - mIntensity) * Kanzen::DampingAdjust( gDestructModeDistanceEffectDamping, elapsedTime );
		}
	}

	void C_LightPillarEffect::setEffect( 
		const SpecialFX::C_SpecialFX* pEffect
		)
	{
		m_pEffect = pEffect;
	}

	void C_LightPillarEffect::setGameWorld(
		Game::C_GameWorld* pGameWorld
		)
	{
		m_pGameWorld = pGameWorld;
	}

	bool		
		C_LightPillarEffect::start()
	{

		if(mDistanceEffect)
		{
			mDistanceEffect->Stop();
			SAFE_DELETE_PTR(mDistanceEffect);
		}

		if (!m_pEffect)
			return false;

		mDistanceEffectTime = 0.0f;
		mDistanceEffect = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(*m_pEffect, m_pGameWorld->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());

		if (!mDistanceEffect)
			return false;

		mDistanceEffect->SetParameter(SpecialFX::E_ParamID_SetCameraViewport, (void *)0);
		mDistanceEffect->Start();

		return true;
	}



} // namespace Game