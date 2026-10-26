#ifndef __SYSTEM_SUBSCRIBER_MANAGER_H__
#define __SYSTEM_SUBSCRIBER_MANAGER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <core/containers/sortedbinarytree.h>

/// facilitates the runtime creation of subscribers by the ini file
namespace Debug
{

#define DECLARE_SUBSCRIBER(ClassType) \
    static I_LogSubscriber *createNew##ClassType(); \
	static ::Debug::Registration registrationFor##ClassType;

#ifndef __LINT__
#define IMPLEMENT_SUBSCRIBER(ClassType, FullyQualifiedName) \
	::Debug::I_LogSubscriber *ClassType::createNew##ClassType() { return new ClassType; }		      \
	::Debug::Registration \
		ClassType::registrationFor##ClassType(FullyQualifiedName, ClassType::createNew##ClassType);
#else
#define IMPLEMENT_SUBSCRIBER(A, B)
#endif

// type for the base subscriber object creation function
typedef class I_LogSubscriber* (*NewSubscriberFunction) (void);

/// This manages a registration to the typemanager
class Registration
{
public:
	Registration(const char* name, NewSubscriberFunction func);
	~Registration();

	Util::C_String m_Name;
};

/// facilitates creating subscribers by name at runtime (for loading the ini file)
class SubscriberManager
{
public:
	typedef Ut::UtMap<Util::C_String,NewSubscriberFunction> SubscriberMap;

	/// This adds a new construction function to the type manager
	void						Add(const char*, NewSubscriberFunction construction);

	/// Creates a new object of the required type and returns a pointer to it
	I_LogSubscriber*			CreateInstanceOf(const char* name);

	/// retrieve a reference to the subscriber manager singleton
	static SubscriberManager	&Instance();

private:
	SubscriberMap				m_Map;
};

};


#endif
