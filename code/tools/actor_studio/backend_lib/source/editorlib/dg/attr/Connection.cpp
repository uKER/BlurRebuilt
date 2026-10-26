#include <stdafx.h>
#include <EditorLib/Dg/Attr/IPlug.h>

namespace EditorLib
{

void C_PlugConnection::Connect(I_Plug* pSrc, I_Plug* pDst)
{
	if (m_pSrcConnection == pSrc)
		return;

	if (pSrc && pDst)
	{
		pSrc->GetConnections().Attach(pDst);
		m_pSrcConnection = pSrc;
		return;
	}
	BIZ_ASSERT(!"Invalid connection attempt!");
}

void C_PlugConnection::Disconnect(I_Plug* pDst)
{
	if (m_pSrcConnection)
	{
		m_pSrcConnection->GetConnections().Detach(pDst);
		m_pSrcConnection = 0;
	}
}

C_ConnectedPlugs::~C_ConnectedPlugs()
{
	Btl::List<I_Plug*>::Iterator it(m_dstConnections.Begin());
	while (it != m_dstConnections.End())
	{
#ifdef BUILD_CONFIG_DEBUG
		S32 size = m_dstConnections.Size();
#endif
		(*it)->SetSourceConnection(0);
#ifdef BUILD_CONFIG_DEBUG
		BIZ_ASSERT(size > m_dstConnections.Size());
#endif
		it = m_dstConnections.Begin();
	}
}

void C_ConnectedPlugs::Attach(I_Plug* pDst)
{
	for (Btl::List<I_Plug*>::Iterator it(m_dstConnections.Begin()); it != m_dstConnections.End(); ++it)
		if (*it == pDst)
			return;

	m_dstConnections.PushBack(pDst);
}

void C_ConnectedPlugs::Detach(I_Plug* pDst)
{
	for (Btl::List<I_Plug*>::Iterator it(m_dstConnections.Begin()); it != m_dstConnections.End(); ++it)
		if (*it == pDst)
		{
			m_dstConnections.Erase(it);
			return;
		}

	BIZ_ASSERT(!"Could not detach connection!");
}

}

