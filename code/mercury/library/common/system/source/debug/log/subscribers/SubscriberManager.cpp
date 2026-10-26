#include "SystemPch.h"

#include "system/debug/log/DebugLog.h"

/// facilitates the runtime creation of subscribers by the ini file
namespace Debug
{

// define the singleton
Singleton<SubscriberManager> g_SubscriberManagerSingleton;
template<> SubscriberManager *Singleton<SubscriberManager>::s_pInstance = 0;

/// Add a new construction function to the type manager
void SubscriberManager::Add(const char *name, NewSubscriberFunction construction)
{
	SubscriberManager::m_Map[ Util::C_String(name).Lower() ] = construction;
}

/// retrieve singleton instance
SubscriberManager &SubscriberManager::Instance()
{
	return Singleton<SubscriberManager>::Instance();
}

/// Creates a new object of the required type and returns a pointer to it
I_LogSubscriber* SubscriberManager::CreateInstanceOf(const char* name)
{
	SubscriberMap::Iterator it = SubscriberManager::m_Map.Find( Util::C_String(name).Lower() );
	if (it == SubscriberManager::m_Map.End())
		return NULL;

	return it->GetValue()();
}

/// register a new subscriber
Registration::Registration(const char *name, NewSubscriberFunction func)
{
	m_Name = name;
	Singleton<SubscriberManager>::Instance().Add(m_Name, func);
}

Registration::~Registration()
{
}

}
