#include "SystemPch.h"

// generic storage for debug metadata
#include <system/serialise/I_File.h>
#include <system/serialise/BinarySerialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include <system/debug/metadebug/MetaDebug.h>
#include <system/debug/metadebug/MetaDebugAttributeList.h>
#include <system/debug/metadebug/MetaDebugLog.h>
#include <system/debug/metadebug/MetaDebugRender.h>

namespace MetaDebug
{

Debug::Logger &g_Log = Debug::GetLogger("metadebug");

enum E_Version
{
	E_Version_Initial = 0,
	E_Version_Current = E_Version_Initial
};

// C_MetaNode
C_MetaNode::C_MetaNode()
	: m_pParent(NULL)
	, m_pNextSibling(NULL)
	, m_pFirstChild(NULL)
	, m_Name()
	, m_DebugLevel( DebugLevel::Debug )
{
}

C_MetaNode::C_MetaNode( const Util::C_String& name )
	: m_pParent(NULL)
	, m_pNextSibling(NULL)
	, m_pFirstChild(NULL)
	, m_Name(name)
	, m_DebugLevel( DebugLevel::Debug )
{
}

C_MetaNode::C_MetaNode( const Util::C_String& name, DebugLevel::Level debugLevel )
	: m_pParent(NULL)
	, m_pNextSibling(NULL)
	, m_pFirstChild(NULL)
	, m_Name(name)
	, m_DebugLevel( debugLevel )
{
}

C_MetaNode::~C_MetaNode()
{
	// delete all child nodes (in reverse order!)
	Btl::Vector<C_MetaNode*> childList;
	childList.Reserve(8);
	C_MetaNode* pChild = m_pFirstChild;
	while(pChild)
	{
		childList.PushBack(pChild);
		pChild = pChild->m_pNextSibling;
	}

	while(childList.Size())
	{
		delete childList[ childList.Size() - 1 ];
		childList.PopBack();
	}

	// remove from parent's child list
	if(m_pParent)
	{
		if(m_pParent->m_pFirstChild == this)
		{
			m_pParent->m_pFirstChild = m_pNextSibling;
		}
		else
		{
			C_MetaNode* pPrevSibling = m_pParent->m_pFirstChild;
			while((pPrevSibling != NULL) && (pPrevSibling->m_pNextSibling != this))
			{
				pPrevSibling = pPrevSibling->m_pNextSibling;
			}

			if(pPrevSibling)
			{
				pPrevSibling->m_pNextSibling = m_pNextSibling;
			}
		}
	}

	m_pFirstChild = NULL;
	m_pNextSibling = NULL;
	m_pParent = NULL;
}

S32 C_MetaNode::GetChildCount() const
{
	S32 childCount = 0;
	C_MetaNode* pChild = m_pFirstChild;
	while(pChild)
	{
		childCount++;
		pChild = pChild->m_pNextSibling;
	}

	return childCount;
}

C_MetaNode* C_MetaNode::GetParent() const
{
	return m_pParent;
}

C_MetaNode* C_MetaNode::GetNextSibling() const
{
	return m_pNextSibling;
}

C_MetaNode* C_MetaNode::GetFirstChild() const
{
	return m_pFirstChild;
}

C_MetaNode* C_MetaNode::GetChildByIndex(S32 index) const
{
	if( index < 0 )
		return NULL;
	if( index >= GetChildCount() )
		return NULL;

	C_MetaNode* pChild = GetFirstChild();
	for(S32 i=0; i<index; i++)
	{
		pChild = pChild->GetNextSibling();
	}

	return pChild;
}

C_MetaNode* C_MetaNode::GetChildByName(const Util::C_String& name) const
{
	C_MetaNode* pChild = GetFirstChild();
	while(pChild)
	{
		if(pChild->GetName() == name)
			return pChild;
		pChild = pChild->GetNextSibling();
	}

	return NULL;
}

C_MetaNode* C_MetaNode::AddNode( const Util::C_String& name, DebugLevel::Level debugLevel )
{
	C_MetaNode* pChild = new C_MetaNode( name, debugLevel );
	AddChild(pChild);
	return pChild;
}

C_AttributeList* C_MetaNode::AddAttributeList( const Util::C_String& name, DebugLevel::Level debugLevel )
{
	C_AttributeList* pChild = new C_AttributeList( name, debugLevel );
	AddChild(pChild);
	return pChild;
}

C_Log* C_MetaNode::AddLog( const Util::C_String& name, DebugLevel::Level debugLevel )
{
	C_Log* pChild = new C_Log( name, debugLevel );
	AddChild(pChild);
	return pChild;
}

C_DebugRender* C_MetaNode::AddDebugRender( const Util::C_String& name, DebugLevel::Level debugLevel )
{
	C_DebugRender* pChild = new C_DebugRender( name, debugLevel );
	AddChild(pChild);
	return pChild;
}

void C_MetaNode::AddChild( C_MetaNode* pChild )
{
	pChild->m_pParent = this;

	// add to END of child list
	if( m_pFirstChild == NULL )
	{
		m_pFirstChild = pChild;
	}
	else
	{
		C_MetaNode* pPrevChild = m_pFirstChild;
		while(pPrevChild->m_pNextSibling != NULL)
		{
			pPrevChild = pPrevChild->m_pNextSibling;
		}

		pPrevChild->m_pNextSibling = pChild;
	}
}

E_Type C_MetaNode::GetType() const
{
	return E_Type_Node;
}

const Util::C_String& C_MetaNode::GetName() const
{
	return m_Name;
}

Util::C_String C_MetaNode::GetFullName() const
{
	// follow up parents accumulating name
	Util::C_String name = m_Name;

	C_MetaNode* pNode = this->m_pParent;
	while(pNode != NULL)
	{
		name = pNode->m_Name + "." + name;
		pNode = pNode->m_pParent;
	}

	return name;
}

DebugLevel::Level C_MetaNode::GetDebugLevel() const
{
	return (DebugLevel::Level)m_DebugLevel;
}

void C_MetaNode::Dump() const
{
	Dump("");
}

void C_MetaNode::Dump(const Util::C_String& prefix ) const
{
	Util::C_String printPrefix;
	Util::C_String subnodePrefix;

	bool isFirstNode = false;
	bool isLastNode = false;

	// find out where we are in the sibling list
	{
		if(GetParent() == NULL)
			isFirstNode = true;
		else
		{
			if(GetParent()->GetFirstChild() == this)
			{
				isFirstNode = true;
			}
		}

		isLastNode = (GetNextSibling() == NULL);
	}

	if( isLastNode )
	{
		// Last
		printPrefix = prefix +		"`---";
		subnodePrefix = prefix +	"    ";
	}
	else
	if( isFirstNode )
	{
		// First
		printPrefix = prefix +		"+---";
		subnodePrefix = prefix +	"|   ";
	}
	else
	{
		// Middle
		printPrefix = prefix +		"|---";
		subnodePrefix = prefix +	"|   ";
	}

	switch(GetType())
	{
		case E_Type_Node:
			LOGDEBUG( g_Log, "%s<Node> '%s'\n", printPrefix.Chars(), GetName().Chars());
		break;

		case E_Type_AttributeList:
			LOGDEBUG( g_Log, "%s<AttributeList> '%s'\n", printPrefix.Chars(), GetName().Chars());
		break;

		case E_Type_Log:
			LOGDEBUG( g_Log, "%s<Log> '%s'\n", printPrefix.Chars(), GetName().Chars());
		break;

		case E_Type_DebugRender:
			LOGDEBUG( g_Log, "%s<DebugRender> '%s'\n", printPrefix.Chars(), GetName().Chars());
		break;

	}

	DumpData( subnodePrefix );

	C_MetaNode* pChild = GetFirstChild();
	while(pChild)
	{
		pChild->Dump( subnodePrefix );

		pChild = pChild->GetNextSibling();
	}
}

void C_MetaNode::DumpData(const Util::C_String& prefix ) const
{
}

void C_MetaNode::Read( Sys::I_File* pFile, I_MetaDebugNotify* pDebugNotify )
{
	{
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);
		U32 fileVersion;
		ar >> fileVersion;
	}
	ReadNode(pFile, pDebugNotify);
}

void C_MetaNode::Write( Sys::I_File* pFile ) const
{
	{
		Sys::BinarySerialiserAutoEndian ar(*pFile);
		ar << (U32)E_Version_Current;
	}
	WriteNode(pFile);
}


void C_MetaNode::ReadNode( Sys::I_File* pFile, I_MetaDebugNotify* pDebugNotify )
{
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);

	ar >> m_Name;
	ar >> m_DebugLevel;

	ReadData(pFile);

	U16 childCount;
	ar >> childCount;

	for(U16 i=0; i<childCount; i++)
	{
		U8 childType;
		ar >> childType;

		C_MetaNode* pChildNode = NULL;
		switch(childType)
		{
			case E_Type_Node:
				pChildNode = new C_MetaNode;
			break;
			case E_Type_AttributeList:
				pChildNode = new C_AttributeList;
			break;
			case E_Type_Log:
				pChildNode = new C_Log;
			break;
			case E_Type_DebugRender:
				pChildNode = new C_DebugRender;
			break;
		}

		pChildNode->Read(pFile, pDebugNotify);

		AddChild(pChildNode);			

		bool keepNode = true;
		if(pDebugNotify)
		{
			keepNode = pDebugNotify->OnLoad(pChildNode);
		}
		
		if(!keepNode)
		{
			SAFE_DELETE_PTR(pChildNode);
		}
	}
}

void C_MetaNode::WriteNode( Sys::I_File* pFile ) const
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);
	
	ar << m_Name;
	ar << m_DebugLevel;

	WriteData(pFile);

	U16 childCount = GetChildCount();
	ar << childCount;

	C_MetaNode* pChild = m_pFirstChild;
	while(pChild)
	{
		ar << (U8)pChild->GetType();

		pChild->Write(pFile);

		pChild = pChild->m_pNextSibling;
	}
}

void C_MetaNode::ReadData(  Sys::I_File* pFile )
{
}

void C_MetaNode::WriteData(  Sys::I_File* pFile ) const
{	
}

} // namespace MetaDebug

void MetaDebugTest()
{
	MetaDebug::C_MetaNode rootNode("Root");

	MetaDebug::C_AttributeList* pInfo = rootNode.AddAttributeList("Info");

	pInfo->AddU32("Version", 13);
	pInfo->AddS32("Index", 82);
	pInfo->AddF32("Speed", 0.3f);
	pInfo->AddTime("Now", Time::GetCurrentTime());
	pInfo->AddString("Name", "ExportName");

	MetaDebug::C_Log* pLog = rootNode.AddLog("LogMessages");

	pLog->LogMessage( "TestMessage1", DebugLevel::Info );
	pLog->LogMessage( "TestMessage2", DebugLevel::Info );
	pLog->LogMessage( "TestMessage3", DebugLevel::Info );

	MetaDebug::C_DebugRender* pDebugRender = rootNode.AddDebugRender("DebugRender");

	pDebugRender->Draw3dLine( Maths::HVec3(0.0f, 0.0f, 0.0f), Maths::HVec3(0.0f, 10.0f, 0.0f), Maths::RGBA(0xff, 0xff, 0xff, 0xff));
	pDebugRender->Draw3dText( Maths::HVec3(0.0f, 10.0f, 0.0f), 0.25f, "Hello" );

	MetaDebug::C_MetaNode* pNode = rootNode.AddNode("Node");
	MetaDebug::C_MetaNode* pNodeInfo = pNode->AddAttributeList("Info");

	rootNode.Dump();

	// test serialisation of metadata
	{
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("TestMeta.dat", Sys::E_FileAccess_Write );
		if(pFile)
		{
			rootNode.Write(pFile);
			SAFE_DELETE_PTR(pFile);
		}
	}

	// test deserialisation of metadata
	{
		MetaDebug::C_MetaNode readNode("");

		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("TestMeta.dat", Sys::E_FileAccess_Read );
		if(pFile)
		{
			readNode.Read(pFile,NULL);
			SAFE_DELETE_PTR(pFile);
		}

		readNode.Dump();
		MetaDebug::C_MetaNode* pMetaNode = readNode.GetChildByName("DebugRender");
		if(pMetaNode)
		{
			if(pMetaNode->GetType() == MetaDebug::E_Type_DebugRender)
			{
				Maths::HMat43 m;
				m.SetIdentity();
				((MetaDebug::C_DebugRender*)pMetaNode)->DebugRender( DbUtil::DefaultDebugRenderSys()->Persistent(), m);
			}
		}
	}
}
