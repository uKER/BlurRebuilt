#ifndef _SUM_H_
#define _SUM_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Sum
////////////////////////////////////////////////////////////////////////////////
    class Sum : public Component
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
        Sum(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

        const char* Type() const { return "Sum"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

		static U32 Size() { return sizeof(Sum); }
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
        ixInit m_uInputOffset;
    };
}

#endif // _SUM_H_
