#ifndef NETSIMULATION_H
#define NETSIMULATION_H

#ifdef _MSC_VER
#pragma once
#endif


#include <bdCore/bdReference/bdReference.h>
#include "netCore\netTime.h"
#include <misc_shared/Messaging/MessageHandler.h>

namespace Samples { class C_Input; };
namespace UI { class C_UiManager; };

BD_REFERENCE(netInvite);
class netApp;
class netSimulation: public Messaging::C_MessageHandler
{
public:
	netSimulation();
	~netSimulation();

	bool step(float dt, const Samples::C_Input& input);
	void render(float fps);
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	netInviteRef		m_invite;
	netApp*				m_app;
	UI::C_UiManager*	m_pUiManager;

	bool  buttonDown(int i) const;
	bool  buttonPressed(int i) const;
	bool  processInputs(const Samples::C_Input& input);
	float m_padState[32][128];
	int   m_padStatePtr;
};

namespace Samples
{
	extern bool SectionNetConnect_Step(float dt, const class C_Input& input);
	extern void SectionNetConnect_Start();
	extern void SectionNetConnect_End();
	extern void SectionNetConnect_Render(float fps);
};

#endif //NETSIMULATION_H
