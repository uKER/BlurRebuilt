#ifndef _EXPONENT_H_
#define _EXPONENT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Exponent
////////////////////////////////////////////////////////////////////////////////
    class Exponent : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
		    Control ctrlInput;
			Option optPower;
        };

        struct State
        {
            U32 uLastFrame;
            F32 fPrevIn;
            F32 fPrevOut;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        Exponent(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "Exponent"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return flagControl; }

        static U32 Size() { return sizeof(Exponent); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->ctrlInput.sCom = -1;
            pData->ctrlInput.uOut = 0;
            pData->optPower.ctrl.sCom = -1;
            pData->optPower.ctrl.uOut = 0;
            pData->optPower.fVal = 1.f;
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

#endif // _EXPONENT_H_

