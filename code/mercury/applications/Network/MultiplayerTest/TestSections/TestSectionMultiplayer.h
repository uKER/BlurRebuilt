#ifndef TESTMULTIPLAYERSECTION_H
#define TESTMULTIPLAYERSECTION_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef CFG_GOLD 

#include <GameFlowBase.h>
#include <GameApp.h>
#include <GameFlowObject.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include "UISection.h"

#include <bdCore/bdCore.h>
#include "netTime/netTime.h"

BD_REFERENCE(netEventStack);
BD_REFERENCE(netInvite);
class C_TestMultiplayerLayer: public C_GameLayerBase
{
public:
    C_TestMultiplayerLayer( GameFlow::C_Section* pSection );
	virtual ~C_TestMultiplayerLayer();
	virtual void Update( const Maths::C_TimeS& timeStep ) {};
	virtual void Render( const U32 renderParam );
};

class C_TestMultiplayerSection  : public C_UiSection
{
public:
    C_TestMultiplayerSection( GameFlow::C_Queue* pQueue, const U32 initParam );
    virtual ~C_TestMultiplayerSection();
	void CustomRender();
private:
};

#endif 

#endif //TESTMULTIPLAYERSECTION_H
