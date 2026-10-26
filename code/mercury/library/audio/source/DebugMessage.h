#ifndef _DEBUG_MESSAGE_H_
#define _DEBUG_MESSAGE_H_

namespace Audio
{

////////////////////////////////////////////////////////////////////////////////
//	DebugMessage
////////////////////////////////////////////////////////////////////////////////
	class DebugMessage : public IDebugMessage
	{
		bool SetMessageData(void* pData, U32 u32Size);
		bool Despatch();
		void ReceiveData(void* pData, U32 uSize);
	};
}

#endif	//	_DEBUG_MESSAGE_H_

