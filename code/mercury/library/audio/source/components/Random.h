#ifndef _RANDOM_H_
#define _RANDOM_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Random
////////////////////////////////////////////////////////////////////////////////
    class Random : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			Option optMin;
			Option optMax;
        };

        struct State
        {
            F32 fCurrent;
        };

        enum Controls
        {
            ValueCtrl,
        };

        enum Messages
        {
            RegenerateMsg,

            NumMessages
        };

    public:
        Random(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage(Instance* pInstance, U32 uMessage);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "Random"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

        static U32 Size() { return sizeof(Random); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->optMin.ctrl.sCom = -1;
			pData->optMin.ctrl.uOut = 0;
			pData->optMin.fVal = 0.f;
			pData->optMax.ctrl.sCom = -1;
			pData->optMax.ctrl.uOut = 0;
			pData->optMax.fVal = 1.f;
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

		static S32 GetMessage(const char* pchMessage)
		{
			if (strcmp(pchMessage, "Regenerate") == 0)
				return 0;
			else
				return -1;
		}

    private:
        void Regenerate(Instance* pInstance);

    private:
    };
}

#endif // _RANDOM_H_
