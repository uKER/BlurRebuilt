#ifndef EDITOR_LIB_CONNECTION_H
#define EDITOR_LIB_CONNECTION_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_PlugConnection
	{
	public:
		C_PlugConnection() : m_pSrcConnection(0) {}
		~C_PlugConnection() { BIZ_ASSERT(m_pSrcConnection == 0); }

		void Connect(I_Plug* pSrc, I_Plug* pDst);

		void Disconnect(I_Plug* pDst);

		I_Plug* GetSource() { return m_pSrcConnection; }
		const I_Plug* GetSource() const { return m_pSrcConnection; }

	private:
		I_Plug* m_pSrcConnection;
	};


	class C_ConnectedPlugs
	{
	public:
		~C_ConnectedPlugs();

		Btl::List<I_Plug*>& GetPlugs() { return m_dstConnections; }

		const Btl::List<const I_Plug*>&  GetPlugs() const { return (const Btl::List<const I_Plug*>&)m_dstConnections; }

	private:
		void Attach(I_Plug* pDst);

		void Detach(I_Plug* pDst);

		Btl::List<I_Plug*> m_dstConnections;

		FRIEND_CLASS C_PlugConnection;
	};
}

#endif
