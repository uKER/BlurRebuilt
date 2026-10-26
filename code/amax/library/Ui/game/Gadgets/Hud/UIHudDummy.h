#ifndef UIHUDHUDDUMMY_H
#define UIHUDHUDDUMMY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include "ui/UIGameDefines.h"

namespace RacingPhysics
{
	class Car;
};

namespace UI
{

// --------
// UIHudDummy
// --------
class UIHudDummy : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_HudDummy, "HudDummy" )

public:
	void	Init();
	void	PostInit();
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	FrameUpdate();

private:
	F32						m_OffsetPosX;
	F32						m_OffsetPosY;
	F32						m_OffsetScale;
	F32						m_OffsetAngle;

	F32						m_OriginX;
	F32						m_OriginY;
	F32						m_OriginScale;
	F32						m_OriginAngle;

	RacingPhysics::Car *	m_pVehicle;

};

};	// namespace UI

#endif /* UIHUDHUDDUMMY_H */
