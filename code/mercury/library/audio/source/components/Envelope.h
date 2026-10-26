#ifndef _ENVELOPE_H_
#define _ENVELOPE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Envelope
////////////////////////////////////////////////////////////////////////////////
    class Envelope : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
		    Control ctrlInput;
		    Option optAttack;
		    Option optHold;
		    Option optRelease;
		    Option optDelay;
			Event evAttackEnd;
			Event evHoldEnd;
			Event evReleaseEnd;
			Event evDelayEnd;
        };

        struct State
        {
            F32 fCurrent;
		    F32 fHold;
		    F32 fDelay;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        Envelope(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize();
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
		void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "Envelope"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

        static U32 Size() { return sizeof(Envelope); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->ctrlInput.sCom = -1;
            pData->ctrlInput.uOut = 0;
            pData->optAttack.ctrl.sCom = -1;
            pData->optAttack.ctrl.uOut = 0;
            pData->optAttack.fVal = 0.f;
            pData->optHold.ctrl.sCom = -1;
            pData->optHold.ctrl.uOut = 0;
            pData->optHold.fVal = 0.f;
            pData->optRelease.ctrl.sCom = -1;
            pData->optRelease.ctrl.uOut = 0;
            pData->optRelease.fVal = 0.f;
            pData->optDelay.ctrl.sCom = -1;
            pData->optDelay.ctrl.uOut = 0;
            pData->optDelay.fVal = 0.f;
			pData->evAttackEnd.sCom = -1;
			pData->evAttackEnd.uMsg = 0;
			pData->evHoldEnd.sCom = -1;
			pData->evHoldEnd.uMsg = 0;
			pData->evReleaseEnd.sCom = -1;
			pData->evReleaseEnd.uMsg = 0;
			pData->evDelayEnd.sCom = -1;
			pData->evDelayEnd.uMsg = 0;
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

#endif // _ENVELOPE_H_

