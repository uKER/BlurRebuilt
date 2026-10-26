#ifndef _EVENTINPUT_H_
#define _EVENTINPUT_H_

namespace Audio
{
    class EventOutput;
////////////////////////////////////////////////////////////////////////////////
//	EventInput
////////////////////////////////////////////////////////////////////////////////
    class EventInput : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
			Event evOutput;
         };

        struct State
        {
            Instance* pInstanceOut;
		    Com uEventOutput;
			bool bExecute;
        };

    public:
        EventInput(Object* pObject) : Component(pObject)
        {
        }
        ~EventInput();

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize();
        void InitContext(Instance* pInstance);

		const char* Type() const { return "EventInput"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagEvent | flagInputConnector | flagEventConnector | flagUpdater); }

        static U32 Size() { return sizeof(EventInput); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        void SetOutput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uEventOutput);

		inline State* GetContext(U8* pStateData) { return (State*)&pStateData[m_uStateOffset]; }

		void Update(Instance* pInstance);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->evOutput.sCom = -1;
			pData->evOutput.uMsg = 0;
        }

        static bool ValidateData(Data* pData)
        {
			if (pData->evOutput.sCom == -1)
                return false;
            else
                return true;
        }

        void Execute(Instance* pInstance);

#if defined (AUDIO_ENABLE_PROFILING)
		Instance* GetOtherInstance(Instance* pThisInstance);
		Com GetOtherComponent(Instance* pThisInstance);
#endif

    private:
    };
}

#endif // _EVENTINPUT_H_
