#ifndef _MIN_H_
#define _MIN_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Min
////////////////////////////////////////////////////////////////////////////////
    class Min : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  num inputs
            ArraySize uNumInputs;
        };

        struct State
        {
            F32 fValue;
            U32 uLastFrame;
        };

        struct Input
        {
            //  input
			Option optInput;
        };

        enum Controls
        {
            ValueCtrl
        };

    public:
        Min(Object* pObject) : Component(pObject)
        {
			m_uInputsOffset = 0;
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "Min"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return flagControl; }

		static U32 Size() { return sizeof(Min); }
        static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uNumInputs * sizeof(Input); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->uNumInputs = 0;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->uNumInputs < 2)
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
		ixInit m_uInputsOffset;
    };
}

#endif // _MIN_H_
