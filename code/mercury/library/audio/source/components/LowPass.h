#ifndef _LOWPASS_H_
#define _LOWPASS_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	LowPass
////////////////////////////////////////////////////////////////////////////////
    class LowPass : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
		    Control ctrlInput;
		    Option optCutoff;
        };

        struct State
        {
			F32 fA;
			F32 fB;
			F32 fPrevInput;
            F32 fValue;
            F32 fPrevCutoff;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        LowPass(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "LowPass"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

        static U32 Size() { return sizeof(LowPass); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->ctrlInput.sCom = -1;
            pData->ctrlInput.uOut = 0;
			pData->optCutoff.ctrl.sCom = -1;
			pData->optCutoff.ctrl.uOut = 0;
            pData->optCutoff.fVal = 1000.f;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->ctrlInput.sCom == -1)
                return false;
            else
                return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Value") == 0)
                return 0;
            else
                return -1;
        }

    private:
    };
}

#endif // _LOWPASS_H_
