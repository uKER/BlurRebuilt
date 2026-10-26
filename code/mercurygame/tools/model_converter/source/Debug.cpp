#include "stdafx.h"

#include <system/debug/metadebug/MetaDebug.h>
#include <system/debug/metadebug/MetaDebugAttributeList.h>
#include <system/debug/metadebug/metadebugLog.h>
#include <system\debug\BuildStamp.h>
#include <system/debug/log/LogManager.h>
#include <system/debug/log/Logger.h>
#include "Debug.h"

C_AppMetaDebug* g_pAppMetaDebug = NULL;

void AddExportInfo( MetaDebug::C_AttributeList* pInfoNode )
{
	pInfoNode->AddTime("TimeExported", Time::GetCurrentTime());
	pInfoNode->AddString("BuildPC", Debug::GetBuildPc());
	pInfoNode->AddString("BuildUser", Debug::GetBuildUser());
	pInfoNode->AddU32("BuildRevision", atoi(Debug::GetBuildRevision()));
	pInfoNode->AddString("BuildDate", Debug::GetBuildDate());
	pInfoNode->AddString("BuildTime", Debug::GetBuildTime());

#ifdef PLATFORM_PC
	char computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD bufferSize = MAX_COMPUTERNAME_LENGTH;
	BOOL success = GetComputerName(computerName, &bufferSize);
	pInfoNode->AddString("ExportPC", computerName);
#endif
}

C_AppMetaDebug::C_AppMetaDebug()
	: m_RootNode("Model")
{
	BIZ_ASSERT(g_pAppMetaDebug == NULL);
	g_pAppMetaDebug = this;

	m_pInfoNode = m_RootNode.AddAttributeList("Info");
	AddExportInfo(m_pInfoNode);

	m_pLogNode = m_RootNode.AddLog("ErrorLog");
	AddLogSubscriber();
}

C_AppMetaDebug::~C_AppMetaDebug()
{
	RemoveLogSubscriber();
	BIZ_ASSERT(g_pAppMetaDebug == this);
	g_pAppMetaDebug = NULL;
}

C_AppMetaDebug* C_AppMetaDebug::GetInstance()
{
	return g_pAppMetaDebug;
}

MetaDebug::C_MetaNode* C_AppMetaDebug::GetRootNode()
{
	return &m_RootNode;
}

MetaDebug::C_AttributeList* C_AppMetaDebug::GetInfoNode()
{
	return m_pInfoNode;
}

MetaDebug::C_Log* C_AppMetaDebug::GetLogNode()
{
	return m_pLogNode;
}

void C_AppMetaDebug::AddLogSubscriber()
{
	m_pMetaLogSubscriber = new MetaDebug::C_MetaLogSubscriber(m_pLogNode);

	m_pMetaLogSubscriber->SetXMLParameter("format", "%c");
	//Subscribe logger
	Debug::GetRootLogger().AddSubscriber(m_pMetaLogSubscriber);
	m_pMetaLogSubscriber->SetXMLParameter("Filter", "Fatal|Error|Warn");
}

void C_AppMetaDebug::RemoveLogSubscriber()
{
	if(m_pMetaLogSubscriber)
	{
		Debug::GetRootLogger().RemoveSubscriber(m_pMetaLogSubscriber);
		SAFE_DELETE_PTR( m_pMetaLogSubscriber );
	}
}

