#ifndef _ARRAY_H_
#define _ARRAY_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Array
////////////////////////////////////////////////////////////////////////////////
    class Array : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Control ctrlInput;
            Option optOffset;
            ArraySize uNumElements;
        };

        struct State
        {
            bool bIsDynamic;

            U32 uLastFrame;

            F32 fOffset;
            F32 fPrevIn;
            F32 fPrevOut;
        };

        struct Element
        {
            Option optElement;
        };

        enum Controls
        {
            ValueCtrl
        };

    public:
        Array(Object* pObject) : Component(pObject)
        {
			m_uElementOffset = 0;
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "Array"; }

        const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return flagControl; }

		static U32 Size() { return sizeof(Array); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uNumElements * sizeof(Element); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->ctrlInput.sCom = -1;
            pData->ctrlInput.uOut = 0;
            pData->optOffset.ctrl.sCom = -1;
            pData->optOffset.ctrl.uOut = 0;
            pData->optOffset.fVal = 0.f;
            pData->uNumElements = 0;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->ctrlInput.sCom == -1)
                return false;
            else if (pData->uNumElements < 2)
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
		ixInit m_uElementOffset;
    };
}

#endif // _ARRAY_H_
