#include "NightwingPch.h"
#include "netTestGameActor.h"
#include "netGame/netGameEnums.h"
#include "netStream/netStream.h"

namespace netGame
{
	C_HealthState::C_HealthState(const int uId, int uTypeId)
		: I_TestAppActor(uId, uTypeId)
		, m_fHealth(1)
	{
		;
	}

	bool C_HealthState::serialise(class netStream* stream)
	{
		stream->writeType(m_fHealth);
		return stream->ok();
	}

	bool C_HealthState::deserialise(class netStream* stream)
	{
		stream->readType(m_fHealth);
		return stream->ok();
	}

	C_GameActor::C_GameActor(const int uId, int uTypeId) 
		: I_TestAppActor(uId, uTypeId)
		, m_fRadius(1)
		, m_pxHealthState(NULL)
	{
		// TODO: Create components here?;
	}

	C_GameActor::~C_GameActor()
	{
		if(m_pxHealthState)
		{
			delete m_pxHealthState;
			m_pxHealthState = NULL;
		}
	}

	void C_GameActor::Set(const Maths::HVec3& xPos, const Maths::HVec3& xVel, const float fRadius, const float fMaxSpeed, const Maths::RGBA& xColour)
	{	
		m_xPos = xPos;	
		m_xVel = xVel; 
		m_fRadius = fRadius; 
		m_fMaxSpeed = fMaxSpeed; 
		m_xColour = xColour; 
	}

	I_TestAppActor* C_GameActor::GetComponent(int uType)
	{
		switch(uType)
		{
		case E_NetEnt_ActorCmp_Health: return m_pxHealthState;
		}
		assert(!"fail!");
		return NULL;
	}

	void C_GameActor::Update(float t)
	{	
		m_xPos = NextPos(t);	
	}

	Maths::HVec3 C_GameActor::NextPos(float t)
	{	
		return m_xPos + (m_xVel*t); 
	}

	void C_GameActor::NewVel()
	{	
		m_xVel = Maths::HVec3(((rand()/(float)RAND_MAX)-0.5f)*m_fMaxSpeed, 0, ((rand()/(float)RAND_MAX)-0.5f)*m_fMaxSpeed);
	}

	bool C_GameActor::serialise(class netStream* stream)
	{
		stream->writeType(m_xPos);
		stream->writeType(m_xVel);
		stream->writeType(m_xColour);
		stream->writeType(m_fRadius);
		stream->writeType(m_fMaxSpeed);
		return stream->ok();
	}

	bool C_GameActor::deserialise(class netStream* stream)
	{
		stream->readType(m_xPos);
		stream->readType(m_xVel);
		stream->readType(m_xColour);
		stream->readType(m_fRadius);
		stream->readType(m_fMaxSpeed);
		return stream->ok();
	}
};