#ifndef _SAMPLEANDHOLD_H_
#define _SAMPLEANDHOLD_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	SampleAndHold
////////////////////////////////////////////////////////////////////////////////
    class SampleAndHold : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
		    Control ctrlInput;
        };

        struct State
        {
            F32 fValue;
        };

        enum Controls
        {
            ValueCtrl,
        };

        enum Messages
        {
            SampleMsg,

            NumMessages
        };

    public:
        SampleAndHold(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage(Instance* pInstance, U32 uMessage);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "SampleAndHold"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

        static U32 Size() { return sizeof(SampleAndHold); }
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

		static S32 GetMessage(const char* pchMessage)
		{
			if (strcmp(pchMessage, "Sample") == 0)
				return 0;
			else
				return -1;
		}

    protected:
		void Sample(Instance* pInstance);

    private:
    };
}

#endif // _SAMPLEANDHOLD_H_

