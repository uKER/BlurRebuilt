// system2.h

#pragma once

using namespace System;

class ArteryConnection;

namespace Bizarre
{
	namespace Artery
	{
		public ref class Message
		{
		public:
			ArteryMessage*	m_pMsg;

		public:
			Message()
			{
				m_pMsg = new ::ArteryMessage();
			}

			~Message()
			{
				delete m_pMsg;
			}

			bool Set(array<byte>^ data, int length)
			{
				pin_ptr<byte>pinDataArray = &data[0];

				return m_pMsg->Set(pinDataArray, length);
			}

			array<byte>^ Test()
			{
				array<byte>^ managedArray = gcnew array<byte>(m_pMsg->DataLength());

				pin_ptr<byte>pinManagedArray = &managedArray[0];

				memcpy(pinManagedArray, m_pMsg->Data(), m_pMsg->DataLength());

				DebugLn("ArteryMessage: Length %d", m_pMsg->DataLength());

				return managedArray;
			}
		};

		public ref class MessageReader
		{
		protected:
			array<byte>^	m_data;
			int				m_current;
			int				m_end;

			bool Read(void* data, int length);

		public:
			MessageReader(Message^ msg);

			bool Read(array<byte>^ data, int length);
			bool Read([System::Runtime::InteropServices::Out] SByte% val);
			bool Read([System::Runtime::InteropServices::Out] Byte% val);
			bool Read([System::Runtime::InteropServices::Out] Int16% val);
			bool Read([System::Runtime::InteropServices::Out] UInt16% val);
			bool Read([System::Runtime::InteropServices::Out] Int32% val);
			bool Read([System::Runtime::InteropServices::Out] UInt32% val);
			bool Read([System::Runtime::InteropServices::Out] Int64% val);
			bool Read([System::Runtime::InteropServices::Out] UInt64% val);
			bool Read([System::Runtime::InteropServices::Out] String^% val);
			bool Read([System::Runtime::InteropServices::Out] float% val);
		};

		public ref class MessageBuilder
		{
		public:
			MessageBuilder() : m_length(0)
			{
				m_data = gcnew array<byte>(0);
			}

			~MessageBuilder()
			{
			}

		public:
			bool Build(Message^ msg);

		public:
			bool Write(const void* data, int length);

		public:
			bool Write(SByte value);
			bool Write(Byte value);
			bool Write(Int16 value);
			bool Write(UInt16 value);
			bool Write(Int32 value);
			bool Write(UInt32 value);
			bool Write(Int64 value);
			bool Write(UInt64 value);
			bool Write(Single value);
			bool Write(String^ text);

		private:
			bool Expand(int length);

		private:
			array<byte>^	m_data;
			int				m_length;
		};

		public ref class WaitContext
		{
		internal:
			ArteryWaitContext* m_pContext;

		public:
			WaitContext()
			{
				m_pContext = new ArteryWaitContext;
			}

			~WaitContext()
			{
				delete m_pContext;
			}
		};

		public ref class Connection
		{
		internal:
			ArteryConnection*	m_pConn;
			bool m_allocated;

		public:
			Connection(ArteryConnection& conn) : m_pConn(&conn), m_allocated(false)
			{
			}

			Connection() : m_allocated(true)
			{
				m_pConn = new ::ArteryConnection();
			}

			~Connection()
			{
				if (m_allocated)
				{
					m_pConn->Close();
					delete m_pConn;
				}
			}

			void Close()
			{
				m_pConn->Close();
			}

			bool IsConnected()
			{
				return m_pConn->IsConnected();
			}

			bool TryReceive(Message^ msg)
			{
				return m_pConn->TryReceive(*msg->m_pMsg);
			}

			bool Receive(Message^ msg)
			{
				return m_pConn->Receive(*msg->m_pMsg);
			}

			bool Send(const Message^ msg)
			{
				return m_pConn->Send(*msg->m_pMsg);
			}

			bool Wait(WaitContext^ wc);
			bool Wait(WaitContext^ wc, int timeMsec);
		};

		public ref class SockAddr
		{
		internal:
			ArterySockAddr* m_pSockAddr;

		public:
			SockAddr(UInt32 addr, UInt32 port)
			{
				m_pSockAddr = new ArterySockAddr(addr, port);
			}

			~SockAddr()
			{
				delete m_pSockAddr;
			}
		};

		public ref class ArteryManager
		{
		public:
			delegate void Callback(Connection^ conn, System::Object^ context);

			static bool Startup(bool startServer, String^ name);
			static bool StartupEx(bool startServer, String^ name, int maxPeers);

			static bool Register(String^ id, Callback^ callback, System::Object^ context);

			static bool Connect(Connection^ con, SockAddr^ target, String^ id, Callback^ callback, System::Object^ context);
		};
	}
}
