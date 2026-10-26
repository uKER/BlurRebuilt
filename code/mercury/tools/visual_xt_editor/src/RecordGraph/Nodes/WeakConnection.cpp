#include "stdafx.h"

#include "WeakConnection.h"

#include "WeakPtr.h"
#include "WeakSocket.h"

using namespace RecordGraph;

C_WeakConnection::C_WeakConnection(C_WeakPtrNode* pWeakPtr) : m_pWeakPtrNode(pWeakPtr)
{
}

C_WeakConnection::~C_WeakConnection()
{
}

bool C_WeakConnection::Visit(C_WeakSocket* node)
{
	if(m_pWeakPtrNode->m_pWeakPtr == node->m_pWeakPtr)
	{
		if(Xt::DomField* pSrcField = Xt::AsField(m_pWeakPtrNode->m_DomMonitor))
		{
			// link up the weak ptr to the weak socket if a connection doesn't already exist
			if(pSrcField->value == node->m_pField->value && !m_pWeakPtrNode->m_pWeakSocket && !node->m_pWeakPtrNode)
			{
				m_pWeakPtrNode->m_pWeakSocket = node;
				node->m_pWeakPtrNode = m_pWeakPtrNode;
			}
		}
	}
	
	return true;
}
