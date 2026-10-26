#ifndef ENVIRONMENT_SETTINGS_MANAGER_H
#define ENVIRONMENT_SETTINGS_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/Messaging.h>

namespace Game
{

class C_EnvironmentSettingsManager : public Messaging::C_MessageHandler
{
public:

	C_EnvironmentSettingsManager()
	{
	};

	void Init();
	void Update();
	static C_EnvironmentSettingsManager& Instance(){return s_instance;};

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

private:
	static C_EnvironmentSettingsManager s_instance;
};

} // namespace Game

#endif // ENVIRONMENT_SETTINGS_MANAGER_H