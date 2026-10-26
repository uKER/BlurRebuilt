#include "SystemPch.h"

#include <System/Xml/XmlBlockParser.h>
#include <System/DebugLog.h>

namespace XmlBlockParser
{

Debug::Logger &g_Log = Debug::GetLogger("mercury.XmlBlockParser");

XmlBlockParser::C_XmlBlock* C_XmlBlock::NewChildBlock( const Util::C_String& name, const Sys::C_XmlAttributeList& attributes )
{
	//LOGDEBUG( g_Log, "<%s>\n", name.Chars());
	return this;
}

void C_XmlBlock::EndChildBlock()
{
	//LOGDEBUG( g_Log, "EndBlock\n");
}

C_XmlParser::C_XmlParser( C_XmlBlock* pRootBlock )
{
	m_pRootBlock = pRootBlock;
}

bool C_XmlParser::StartDocument()
{
	Debug::GetEffectiveLevel(g_Log);
	Debug::_dbgMessageProxy::LogS( g_Log, ::DebugLevel::Debug, ::Debug::StaticLocationInfoClass(DEBUG_LOCATION), "jan" );

    if(m_pBlockStack.Size())
    {
        LOGERROR(g_Log, "Already parsing xml file\n");
    }
    //LOGDEBUG( g_Log, "BEGIN XML DOCUMENT\n");
	m_pBlockStack.PushBack(m_pRootBlock);
	return true;
}

bool C_XmlParser::EndDocument()
{
	//LOGDEBUG( g_Log, "END XML DOCUMENT\n");
	//if(m_pCurrBlock != m_pRootBlock)
	if(m_pBlockStack.Size() != 1)
	{
		LOGERROR(g_Log, "XML Nesting error\n");
	}
	//m_pCurrBlock = NULL;
	m_pBlockStack.PopBack();
	return true;
}

bool C_XmlParser::StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes)
{	
	C_XmlBlock* pCurrBlock = m_pBlockStack[m_pBlockStack.Size() - 1];
    m_pBlockStack.PushBack( pCurrBlock->NewChildBlock(name, attributes) );    
	return true;
}

bool C_XmlParser::EndElement(const Util::C_String& name)
{
    //LOGDEBUG( g_Log, "End: %s\n", name.Chars());
	C_XmlBlock* pCurrBlock = m_pBlockStack[m_pBlockStack.Size() - 1];
    pCurrBlock->EndChildBlock();
	m_pBlockStack.PopBack();
	return true;
}


} // namespace XmlBlockParser
