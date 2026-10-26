#ifndef _EVENTOUTPUT_H_
#define _EVENTOUTPUT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	EventOutput
////////////////////////////////////////////////////////////////////////////////
    class EventOutput : public Component
    {
    public:
        struct Data
		{
			StrOffset strName;
		};

        struct State
        {
            Instance* pToInstance;
		    Com uEventInput;
			bool bExecuteInput;
        };

        enum Messages
        {
            ExecuteMsg,

            NumMessages
        };

    public:
        EventOutput(Object* pObject) : Component(pObject)
        {
        }
        ~EventOutput();

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU

        void DoMessage(Instance* pInstance, U32 uMessage);


#ifndef PLATFORM_SPU
        const char* Type() const { return "EventOutput"; }

		const U32 Flags() { return StaticFlags(); }

		U32 GetNumMessages() { return NumMessages; }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagEvent | flagOutputConnector | flagEventConnector); }

        static U32 Size() { return sizeof(EventOutput); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        void SetInput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uEventInput);

		inline State* GetContext(U8* pStateData) { return (State*)&pStateData[m_uStateOffset]; }

		void ProcessConnection(Instance* pInstance);

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

#if defined (AUDIO_ENABLE_PROFILING)
		Instance* GetOtherInstance(Instance* pThisInstance);
		Com GetOtherComponent(Instance* pThisInstance);
#endif

    private:
    };
}

#endif // _EVENTOUTPUT_H_
