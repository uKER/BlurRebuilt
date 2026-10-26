#ifndef _GRADIENT_H_
#define _GRADIENT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Gradient
////////////////////////////////////////////////////////////////////////////////
    class Gradient : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
		    Control ctrlInput;
        };

        struct State
        {
            F32 fPrevIn;
            F32 fCurrent;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        Gradient(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "Gradient"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

        static U32 Size() { return sizeof(Gradient); }
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

#endif // _GRADIENT_H_

