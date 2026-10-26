// This is the main DLL file.

#include "SystemPch.h"

#include "system/artery/artery_managed.h"

using namespace System::Runtime::InteropServices;

namespace Bizarre
{
	namespace Artery
	{
		bool ArteryManager::Startup(bool startServer, String^ name)
		{
			System::IntPtr ansiPtr = Marshal::StringToHGlobalAnsi(name);
			char *ansi = (char *) ansiPtr.ToPointer();

			bool result = ::ArteryStartup(startServer, ansi);

			Marshal::FreeHGlobal(ansiPtr);

			return result;
		}

		bool ArteryManager::StartupEx(bool startServer, String^ name, int maxPeers)
		{
			System::IntPtr ansiPtr = Marshal::StringToHGlobalAnsi(name);
			char *ansi = (char *) ansiPtr.ToPointer();

			bool result = ::ArteryStartupEx(startServer, ansi, maxPeers);

			Marshal::FreeHGlobal(ansiPtr);

			return result;
		}

		struct Noob
		{
			GCHandle gchCallback;
			GCHandle gchContext;
		};

		void RegisterCallback(ArteryConnection con, void* context)
		{
			Noob* noob = (Noob*)context;

			ArteryManager::Callback^ myCallback = (ArteryManager::Callback^)noob->gchCallback.Target;
			System::Object^ myContext = (System::Object^)noob->gchContext.Target;

			Connection^ myConn = gcnew Connection(con);

			myCallback(myConn, myContext);
		}
		
		void ConnectCallback(ArteryConnection con, void* context)
		{
			Noob* noob = (Noob*)context;

			ArteryManager::Callback^ myCallback = (ArteryManager::Callback^)noob->gchCallback.Target;
			System::Object^ myContext = (System::Object^)noob->gchContext.Target;

			Connection^ myConn = gcnew Connection(con);

			myCallback(myConn, myContext);
		}

		bool ArteryManager::Register(String^ id, Callback^ myCallback, System::Object^ myContext)
		{
			System::IntPtr idAnsiPtr = Marshal::StringToHGlobalAnsi(id);
			char *idAnsi = (char *) idAnsiPtr.ToPointer();

			Noob* noob = new Noob;

			noob->gchCallback = GCHandle::Alloc(myCallback, GCHandleType::Normal);
			noob->gchContext = GCHandle::Alloc(myContext, GCHandleType::Normal);

			bool result = ::ArteryRegister(idAnsi, RegisterCallback, noob);

			Marshal::FreeHGlobal(idAnsiPtr);

			return result;
		}

		bool ArteryManager::Connect(Connection^ con, SockAddr^ target, String^ id, Callback^ myCallback, System::Object^ myContext)
		{
			System::IntPtr idAnsiPtr = Marshal::StringToHGlobalAnsi(id);
			char *idAnsi = (char *) idAnsiPtr.ToPointer();

			Noob* noob = new Noob;

			noob->gchCallback = GCHandle::Alloc(myCallback, GCHandleType::Normal);
			noob->gchContext = GCHandle::Alloc(myContext, GCHandleType::Normal);

			bool result = ArteryConnect(*con->m_pConn, *target->m_pSockAddr, idAnsi, ConnectCallback, noob);

			Marshal::FreeHGlobal(idAnsiPtr);

			return result;
		}

		MessageReader::MessageReader(Message^ msg)
		{
			m_data = gcnew array<byte>(msg->m_pMsg->DataLength());

			m_current = 0;
			m_end = msg->m_pMsg->DataLength();

			pin_ptr<byte>pinDataArray = &m_data[0];
			memcpy(pinDataArray, msg->m_pMsg->Data(), m_end);
		}

		bool MessageReader::Read(void* data, int length)
		{
			if ((m_current + length) > m_end)
			{
				m_current = m_end;
				return false;
			}

			if (data)
			{
				pin_ptr<byte>pinDataArray = &m_data[m_current];
				memcpy(data, pinDataArray, length);
			}

			m_current += length;

			return true;
		}

		bool MessageReader::Read(array<byte>^ data, int length)
		{
			pin_ptr<byte>pinDataArray = &data[0];

			return Read(pinDataArray, length);
		}

		bool MessageReader::Read(System::SByte% value)
		{
			System::SByte netValue;

			bool result = Read(&netValue, sizeof(netValue));

			value = netValue;

			return result;
		}

		bool MessageReader::Read(System::Byte% value)
		{
			System::Byte netValue;

			bool result = Read(&netValue, sizeof(netValue));

			value = netValue;

			return result;
		}

		bool MessageReader::Read(System::Int16% value)
		{
			System::Int16 netValue;

			if (!Read(&netValue, sizeof(netValue)))
				return false;

			value = System::Net::IPAddress::HostToNetworkOrder(netValue);

			return true;
		}

		bool MessageReader::Read(System::UInt16% value)
		{
			System::Int16 v;

			if (!Read(v))
				return false;

			array<byte>^ arrayVal = BitConverter::GetBytes(v);
			value = BitConverter::ToUInt16(arrayVal, 0);

			return true;
		}

		bool MessageReader::Read(System::Int32% value)
		{
			System::Int32 netValue;

			if (!Read(&netValue, sizeof(netValue)))
				return false;

			value = System::Net::IPAddress::HostToNetworkOrder(netValue);

			return true;
		}

		bool MessageReader::Read(System::UInt32% value)
		{
			System::Int32 v;

			if (!Read(v))
				return false;

			array<byte>^ arrayVal = BitConverter::GetBytes(v);
			value = BitConverter::ToUInt32(arrayVal, 0);

			return true;
		}

		bool MessageReader::Read(System::Single% value)
		{
			System::Int32 v;

			if (!Read(v))
				return false;

			array<byte>^ arrayVal = BitConverter::GetBytes(v);
			value = BitConverter::ToSingle(arrayVal, 0);

			return true;
		}


		bool MessageReader::Read(System::Int64% value)
		{
			System::Int64 netValue;

			if (!Read(&netValue, sizeof(netValue)))
				return false;

			value = System::Net::IPAddress::HostToNetworkOrder(netValue);

			return true;
		}

		bool MessageReader::Read(System::UInt64% value)
		{
			System::Int64 v;

			if (!Read(v))
				return false;

			array<byte>^ arrayVal = BitConverter::GetBytes(v);
			value = BitConverter::ToUInt64(arrayVal, 0);

			return true;
		}

		bool MessageReader::Read(String^% val)
		{
			System::UInt32 length;

			if (!Read(length))
				return false;

			int current = m_current;

			if (!Read(0, length))
				return false;

			pin_ptr<byte>pinDataArray = &m_data[0];
			val = gcnew String((char*)pinDataArray, current, length-1);

			return true;
		}

		bool MessageBuilder::Build(Message^ msg)
		{
			return msg->Set(m_data, m_length);
		}

		bool MessageBuilder::Expand(int length)
		{
			int requiredCapacity = m_length + length;

			if (requiredCapacity <= m_data->Length)
				return true;

			int newCapacity = 3 * ((requiredCapacity + 16) >> 1);

			//array<byte>^ newData = gcnew array<byte>(newCapacity);

			m_data->Resize(m_data, newCapacity);

			//if (!newData)
			//	return false;

			//memcpy(newData, m_data, _length);

			//m_data		= newData;
			//m_capacity	= newCapacity;

			return true;
		}

		bool MessageBuilder::Write(const void* data, int length)
		{
			if (!Expand(length))
				return false;

			pin_ptr<byte>pinDataArray = &m_data[m_length];
			memcpy(pinDataArray, data, length);

			m_length += length;

			return true;
		}

		bool MessageBuilder::Write(SByte value)
		{
			return Write(&value, sizeof(value));
		}

		bool MessageBuilder::Write(Byte value)
		{
			return Write(&value, sizeof(value));
		}

		bool MessageBuilder::Write(Int16 value)
		{
			Int16 netVal = System::Net::IPAddress::HostToNetworkOrder(value);
			return Write(&netVal, sizeof(netVal));
		}

		bool MessageBuilder::Write(UInt16 value)
		{
			array<byte>^ arrayVal = BitConverter::GetBytes(value);
			Int16 v = BitConverter::ToInt16(arrayVal, 0);

			return Write(v);
		}

		bool MessageBuilder::Write(Int32 value)
		{
			Int32 netVal = System::Net::IPAddress::HostToNetworkOrder(value);
			return Write(&netVal, sizeof(netVal));
		}

		bool MessageBuilder::Write(UInt32 value)
		{
			array<byte>^ arrayVal = BitConverter::GetBytes(value);
			Int32 v = BitConverter::ToInt32(arrayVal, 0);

			return Write(v);
		}

		bool MessageBuilder::Write(Int64 value)
		{
			Int64 netVal = System::Net::IPAddress::HostToNetworkOrder(value);
			return Write(&netVal, sizeof(netVal));
		}

		bool MessageBuilder::Write(UInt64 value)
		{
			array<byte>^ arrayVal = BitConverter::GetBytes(value);
			Int64 v = BitConverter::ToInt64(arrayVal, 0);

			return Write(v);
		}

		bool MessageBuilder::Write(Single value)
		{
			array<byte>^ arrayVal = BitConverter::GetBytes(value);
			Int32 v = BitConverter::ToInt32(arrayVal, 0);

			return Write(v);
		}

		bool MessageBuilder::Write(String^ text)
		{
			Int32 length = text->Length+1;

			if (!Write(length))
				return false;

			System::IntPtr textAnsiPtr = Marshal::StringToHGlobalAnsi(text);
			char *textAnsi = (char *) textAnsiPtr.ToPointer();

			bool result = Write(textAnsi, length);

			Marshal::FreeHGlobal(textAnsiPtr);

			return result;
		}

		bool Connection::Wait(WaitContext^ wc)
		{
			return m_pConn->Wait(*wc->m_pContext);
		}

		bool Connection::Wait(WaitContext^ wc, int timeMsec)
		{
			return m_pConn->Wait(*wc->m_pContext, timeMsec);
		}
	}
}