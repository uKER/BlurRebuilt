#ifndef _CONTROLOUTPUT_H_
#define _CONTROLOUTPUT_H_

#define AUDIO_NEW_OUTPUT_OUTPUTS

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	ControlOutput
////////////////////////////////////////////////////////////////////////////////
    class ControlOutput : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
#ifdef AUDIO_NEW_OUTPUT_OUTPUTS
            Option optInput;
#else
            Com sInputCom;
#endif
        };

        struct State
        {
            Instance* pInstanceIn;
		    Com uControlInput;
        };

    public:
        ControlOutput(Object* pObject) : Component(pObject)
        {
        }
        ~ControlOutput() {}

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "ControlOutput"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagControl | flagOutputConnector | flagControlConnector); }

        static U32 Size() { return sizeof(ControlOutput); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        void SetInput(Instance* pThisInstance, Instance* pThatInstance, Audio::Com uControlInput);

		inline State* GetContext(U8* pStateData) { return (State*)&pStateData[m_uStateOffset]; }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
#ifdef AUDIO_NEW_OUTPUT_OUTPUTS
			pData->optInput.ctrl.sCom = -1;
			pData->optInput.ctrl.uOut = 0;
			pData->optInput.fVal = 0.f;
#else
            pData->sInputCom = -1;
#endif
        }

        static bool ValidateData(Data* pData)
        {
#ifndef AUDIO_NEW_OUTPUT_OUTPUTS
            if (pData->sInputCom == -1)
                return false;
            else
#endif
                return true;
        }

#if defined (AUDIO_ENABLE_PROFILING)
		Instance* GetOtherInstance(Instance* pThisInstance);
		Com GetOtherComponent(Instance* pThisInstance);
#endif

    private:
    };
}

#endif // _CONTROLOUTPUT_H_
