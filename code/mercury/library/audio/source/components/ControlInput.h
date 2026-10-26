#ifndef _CONTROLINPUT_H_
#define _CONTROLINPUT_H_

namespace Audio
{
    class ControlOutput;
////////////////////////////////////////////////////////////////////////////////
//	ControlInput
////////////////////////////////////////////////////////////////////////////////
    class ControlInput : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
        };

        struct State
        {
            Instance* pInstanceOut;
			F32 f32Value;
		    Com uControlOutput;
		};

        enum Controls
        {
            ValueCtrl,
        };

    public:
        ControlInput(Object* pObject);
        ~ControlInput();

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "ControlInput"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return flagControl | flagInputConnector | flagControlConnector; }

		static U32 Size() { return sizeof(ControlInput); }
        static U32 DataSize(void* pData) { return sizeof(Data); }

        void SetOutput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uControlOutput);

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

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Value") == 0)
                return 0;
            else
                return -1;
        }

#if defined (AUDIO_ENABLE_PROFILING)
		Instance* GetOtherInstance(Instance* pThisInstance);
		Com GetOtherComponent(Instance* pThisInstance);
#endif

    private:
    };
}

#endif // _CONTROLINPUT_H_
