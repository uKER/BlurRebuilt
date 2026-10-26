#ifndef _EVENTOUT_H_
#define _EVENTOUT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	ActionOut
////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_SPU
    class ActionOut : public Component
#else
    class ActionOut : /*public IActionOut, */public Component
#endif
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
        };

        struct State
        {
			U32 uAppData;
#if defined (PLATFORM_SPU) || defined (PLATFORM_PS3)
            U32 uCount;
#endif
        };

		 enum Messages
        {
            ExecuteMsg,

            NumMessages
        };

    public:
        ActionOut(Object* pObject) : Component(pObject)
        {
			m_pCallback = 0;
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);

		void DoMessage(Instance* pInstance, U32 uMessage);

#ifndef PLATFORM_SPU
        U32 GetNumMessages() { return NumMessages; }
#endif	//	#ifndef PLATFORM_SPU

        const char* Type() const { return "ActionOut"; }

        const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagActionOut | flagEvent); }

		static U32 Size() { return sizeof(ActionOut); }
        static U32 DataSize(void* pData) { return sizeof(Data); }

#ifdef PLATFORM_PS3
		inline U32 GetCallCount(Instance* pInstance) { State* pState = (State*)pInstance->GetStateData(m_uStateOffset); return pState->uCount; }
		inline void ResetCallCount(Instance* pInstance) { State* pState = (State*)pInstance->GetStateData(m_uStateOffset); pState->uCount = 0; }
#endif

		inline void SetCallback(AudioActionOutCallback pCallback) { m_pCallback = pCallback; }
		void SetCallbackData(Instance* pInstance, U32 u32Data);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

		static S32 GetMessage(const char* pchMessage)
		{
			if (strcmp(pchMessage, "Exec") == 0)
				return ExecuteMsg;
			else
				return -1;
		}

        void Execute(Instance* pInstance);

    private:
		AudioActionOutCallback m_pCallback;
    };
}

#endif // _EVENTOUT_H_
