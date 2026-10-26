#ifndef _GAME_ACTOR_
#define _GAME_ACTOR_

#include "netStream/netSerialise.h"

namespace netGame
{
	class I_TestAppActor : public netSerialisable
	{
	private:
		I_TestAppActor()
		{;}
	public:
		I_TestAppActor(int uLocalId, int uTypeId) 
			: m_uLocalId(uLocalId)
			, m_uTypeId(uTypeId)
		{;}
		int LocalId()	{ return m_uLocalId; }
		int TypeId()	{ return m_uTypeId; }
	private:
		int m_uLocalId;
		int m_uTypeId;
	};

	// health state (part of a game actor)
	class C_HealthState : public I_TestAppActor
	{
	public:
		C_HealthState(const int uId, int uTypeId);
		virtual bool serialise(class netStream* stream);
		virtual bool deserialise(class netStream* stream);
	public:
		float m_fHealth;
	};

	// actor state (part of a game actor)
	class C_GameActor : public I_TestAppActor
	{
	public:
						C_GameActor(const int uId, int uTypeId);
						~C_GameActor();
		void			Set(const Maths::HVec3& xPos, const Maths::HVec3& xVel, const float fRadius, const float fMaxSpeed, const Maths::RGBA& xColour);
		I_TestAppActor* GetComponent(int uType);
		void			Update(float t);
		Maths::HVec3	NextPos(float t);
		void			NewVel();
		virtual bool	serialise(class netStream* stream);
		virtual bool	deserialise(class netStream* stream);

	public:
		Maths::HVec3		m_xPos;
		Maths::HVec3		m_xVel;
		Maths::RGBA			m_xColour;
		float				m_fRadius;
		float				m_fMaxSpeed;
		C_HealthState*		m_pxHealthState;
	};
};

#endif