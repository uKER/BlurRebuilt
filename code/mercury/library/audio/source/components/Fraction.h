#ifndef _FRACTION_H_
#define _FRACTION_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Sin
////////////////////////////////////////////////////////////////////////////////
    class Fraction : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  input
			Control ctrlInput;
        };

        struct State
        {
            U32 uLastFrame;
            F32 fPrevious;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        Fraction(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "Fraction"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return flagControl; }

        static U32 Size() { return sizeof(Fraction); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->ctrlInput.sCom = -1;
			pData->ctrlInput.uOut = 0;
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

#endif // _FRACTION_H_
