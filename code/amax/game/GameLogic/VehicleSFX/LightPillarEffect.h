#ifndef LIGHT_PILLAR_EFFECT_H
#define LIGHT_PILLAR_EFFECT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SpecialFX
{
	class C_SpecialFX;
	class I_SpecialFXInstance;
}

namespace Game
{

	class C_GameWorld;

class C_LightPillarEffect
{
public:
	C_LightPillarEffect();
	void		reset();
	bool		active() const;
	void		update(F32 elapsedTime, F32 desiredIntensity, const Maths::HMat43* vehicleTransform);

	bool		start();
	bool		visible() const;
	void		stop();

	void		setEffect(const SpecialFX::C_SpecialFX* pEffect);
	static void setGameWorld(C_GameWorld* pGameWorld);

private:
	static C_GameWorld*						m_pGameWorld;

	const SpecialFX::C_SpecialFX*			m_pEffect;
	SpecialFX::I_SpecialFXInstance*			mDistanceEffect;
	F32										mIntensity;
	F32										mDistanceEffectTime;
};

}	// namespace Game


#endif // LIGHT_PILLAR_EFFECT_H