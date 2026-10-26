#ifndef _TIMER_H_
#define _TIMER_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Timer
////////////////////////////////////////////////////////////////////////////////
    class Timer : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
			Option optStartAt;
			Option optIncrement;
			Option optAlarm;
			Event evAlarm;
        };

        struct State
        {
            bool bActive;
            bool bAlarmed;
            bool bInitialized;
            F32 fPrevious;
            F32 fCurrent;
        };

        enum Controls
        {
            ValueCtrl,
			ActiveCtrl,
        };

        enum Messages
        {
            StartMsg,
            StopMsg,
            ResetMsg,
			ZeroMsg,

            NumMessages
        };

    public:
        Timer(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage(Instance* pInstance, U32 uMessage);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "Timer"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

        static U32 Size() { return sizeof(Timer); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->optStartAt.ctrl.sCom = -1;
			pData->optStartAt.ctrl.uOut = 0;
			pData->optStartAt.fVal = 0.f;
			pData->optIncrement.ctrl.sCom = -1;
			pData->optIncrement.ctrl.uOut = 0;
			pData->optIncrement.fVal = 1.f;
			pData->optAlarm.ctrl.sCom = -1;
			pData->optAlarm.ctrl.uOut = 0;
			pData->optAlarm.fVal = 1.f;
			pData->evAlarm.sCom = -1;
			pData->evAlarm.uMsg = 0;
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
			if (strcmp(pchMessage, "Start") == 0)
				return 0;
			if (strcmp(pchMessage, "Stop") == 0)
				return 1;
			if (strcmp(pchMessage, "Reset") == 0)
				return 2;
			if (strcmp(pchMessage, "Zero") == 0)
				return 3;
			else
				return -1;
		}

    private:
        inline void Start(Instance* pInstance);
        inline void Stop(Instance* pInstance);
        inline void Reset(Instance* pInstance);
        inline void ResetToZero(Instance* pInstance);

    private:
    };
}

#endif // _TIMER_H_
