
#include "stdafx.h"
/*

#include <aftk/Aftk.h>
#include <converter_shared/SceneDesc.h>

namespace SceneDesc
{

// C_UserData

C_UserData::C_UserData()
{
}

C_UserData::~C_UserData()
{
}

// C_GroupNode

C_GroupNode::C_GroupNode()
	: m_Name( "<NoName>" )
	, m_Index( -1 )
	, m_pUserData( NULL )
{
}

C_GroupNode::~C_GroupNode()
{
	SAFE_DELETE_PTR( m_pUserData );
}

void C_GroupNode::Dump( const bool dumpAftkNodeList, const Util::C_String& prefix )
{
	C_GroupNode* pNode = this;
	while(pNode)
	{
		Util::C_String printPrefix;
		Util::C_String subnodePrefix;

		const bool isFirstNode = (!pNode->GetPrevSibling());
		const bool isLastNode = (!pNode->GetNextSibling());

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


		LOGDEBUG( Tools::g_log, "%sNode '%s'", printPrefix.Chars(), pNode->GetName().Chars());

		if( dumpAftkNodeList )
		{
			for(Btl::Vector<const Aftk::C_SceneNode*>::Iterator i = pNode->m_AftkNodeList.Begin(); i < pNode->m_AftkNodeList.End(); ++i)
			{
				const Aftk::C_SceneNode* pSceneNode = (*i);
				LOGDEBUG( Tools::g_log, "%s:   AftkNode '%s'", subnodePrefix.Chars(), pSceneNode->GetName().c_str());
			}
		}

		if(pNode->GetFirstChild())
		{
			pNode->GetFirstChild()->Dump( dumpAftkNodeList, subnodePrefix );
			LOGDEBUG( Tools::g_log, "%s", subnodePrefix.Chars());
		}

		pNode = pNode->GetNextSibling();
	}
}

const Maths::HMat43 C_GroupNode::GetMatrix() const
{
	return m_Matrix;
}

void C_GroupNode::SetMatrix( Maths::HMat43& matrix )
{
	m_Matrix = matrix;
}

const Util::C_String& C_GroupNode::GetName() const
{
	return m_Name;
}

void C_GroupNode::SetName( const Util::C_String& name )
{
	m_Name = name;
}

S32 C_GroupNode::GetIndex() const
{
	return m_Index;
}

void C_GroupNode::AddAftkNode( const Aftk::C_SceneNode* pNode )
{
	m_AftkNodeList.PushBack( pNode );
}

C_AftkNodeList& C_GroupNode::GetAftkNodeList()
{
	return m_AftkNodeList;
}

const C_AftkNodeList& C_GroupNode::GetAftkNodeList() const
{
	return m_AftkNodeList;
}

void C_GroupNode::SetUserData( C_UserData* pUserData )
{
	if( pUserData != m_pUserData )
	{
		SAFE_DELETE_PTR( m_pUserData );
	}
	
	m_pUserData = pUserData;
}

C_UserData* C_GroupNode::GetUserData()
{
	return m_pUserData;
}

const C_UserData* C_GroupNode::GetUserData() const
{
	return m_pUserData;
}

// C_Group

C_Group::C_Group()
	: m_Name( "<NoName>" )
	, m_Index(-1)
	, m_pSceneDesc(NULL)
{
}

C_Group::~C_Group()
{
}

void C_Group::Dump( const bool dumpAftkNodeList )
{
	LOGDEBUG( Tools::g_log, "*Group[%d] '%s'", m_Index, m_Name.Chars() );

	if(GetRootNode())
	{
		LOGDEBUG( Tools::g_log, "|" );
		GetRootNode()->Dump( dumpAftkNodeList );
	}
		
	LOGDEBUG( Tools::g_log, "" );
}	

S32 C_Group::GetIndex() const
{
	return m_Index;
}

void C_Group::SetName( const Util::C_String& name )
{
	m_Name = name;
}

const Util::C_String& C_Group::GetName() const
{
	return m_Name;
}

bool C_Group::EnumerateNodes( C_GroupNode* pNode )
{
	bool success = true;

	S32 nodeCount = 0;

	for( SceneDesc::C_GroupNodeIterator i( this ); (const SceneDesc::C_GroupNode*)i; ++i)
	{
		SceneDesc::C_GroupNode* pCurrNode = i;
		pCurrNode->m_Index = nodeCount++;
	}

	assert( nodeCount == GetNodeCount() );

	return success;
}

// C_SceneDesc

C_SceneDesc::C_SceneDesc()
{
}

C_SceneDesc::~C_SceneDesc()
{
}

void C_SceneDesc::Dump( const bool dumpAftkNodeList )
{
	LOGDEBUG( Tools::g_log, "<pre><tt>" );

	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "SceneDesc::C_SceneDesc:" );
	LOGDEBUG( Tools::g_log, "   %d Groups", m_GroupList.Size() );
	LOGDEBUG( Tools::g_log, "" );

	for(Btl::Vector<C_Group>::Iterator i = m_GroupList.Begin(); i < m_GroupList.End(); ++i)
	{
		(*i).Dump( dumpAftkNodeList );
	}

	LOGDEBUG( Tools::g_log, "</tt></pre>" );
}

void C_SceneDesc::SetGroupCount( const S32 groupCount )
{
	m_GroupList.Resize( groupCount );

	for(S32 groupNo=0; groupNo<groupCount; groupNo++)
	{
		m_GroupList[groupNo].m_Index = groupNo;
		m_GroupList[groupNo].m_pSceneDesc = this;
	}
}

S32 C_SceneDesc::GetGroupCount() const
{
	return m_GroupList.Size();
}

C_Group& C_SceneDesc::GetGroup( const S32 index )
{
	return m_GroupList[index];
}

const C_Group& C_SceneDesc::GetGroup( const S32 index ) const
{
	return m_GroupList[index];
}

bool C_SceneDesc::Enumerate()
{
	bool success = true;

	const S32 groupCount = m_GroupList.Size();
	for(S32 groupNo=0; groupNo<groupCount; groupNo++)
	{
		success = m_GroupList[groupNo].EnumerateNodes() && success;
	}

	return success;
}

} // namespace SceneDesc
*/