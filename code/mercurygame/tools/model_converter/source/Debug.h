#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/debug/metadebug/metadebug.h>

namespace MetaDebug
{
	class C_MetaLogSubscriber;
}

class C_AppMetaDebug
{
public:
	C_AppMetaDebug();
	~C_AppMetaDebug();

	//! This class is not a singleton although only one instance should be created at once. 
	//! GetInstance can be used to return the currently created instance of the class. 
	//! If the class has not been created GetInstance will return NULL.
	static C_AppMetaDebug* GetInstance();

	MetaDebug::C_MetaNode* GetRootNode();
	MetaDebug::C_AttributeList* GetInfoNode();
	MetaDebug::C_Log* GetLogNode();

	void AddLogSubscriber();
	void RemoveLogSubscriber();

private:	
	MetaDebug::C_MetaNode m_RootNode;
	MetaDebug::C_AttributeList*	m_pInfoNode;
	MetaDebug::C_Log* m_pLogNode;


	MetaDebug::C_MetaLogSubscriber* m_pMetaLogSubscriber;
};


#endif
