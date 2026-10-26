#include "stdafx.h"

#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

C_UpdateSignaller::C_UpdateSignaller(I_DgNode* pNode, E_ObjType invokingType, C_UpdateHintCollection* pHc)
{
	m_pNode = pNode;
	m_invokingType = invokingType;

	if (pHc)
	{
		m_pHc = pHc;
	}
	else
	{
		m_pHc = new C_UpdateHintCollection(pNode);
		m_emitSignal = pNode->IsDirty();
	}
}

C_UpdateSignaller::~C_UpdateSignaller()
{
	BIZ_ASSERT(m_pHc && m_pHc->pNode);
	if (m_pHc->pNode->GetType() == m_invokingType)
	{
		if (m_emitSignal)
		{
			BIZ_ASSERT(m_pNode == m_pHc->pNode);

			//If our node is still flagged as dirty then we'll need to bin our hint collection and
			//assume that everything could have been updated. A complete update is now the only way
			//to ensure sanity.
			C_UpdateHintCollection* pHc;
			bool signal = m_pNode->IsNonXtDirty();
			if (signal)
			{
				pHc = 0; //No hints
			}
			else
				pHc = m_pHc; //Use hints.

			m_pNode->ResetDirty();
			C_Core::Instance().GetGlobalSignals().dgNodeUpdated(*m_pHc->pNode, pHc);
		}
		SAFE_DELETE_PTR(m_pHc);
	}
}

}
