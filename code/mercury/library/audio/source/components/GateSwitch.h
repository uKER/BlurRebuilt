#ifndef _GATESWITCH_H_
#define _GATESWITCH_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	GateSwitch
////////////////////////////////////////////////////////////////////////////////
    class GateSwitch : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			Control ctrlInput;
			Option optThreshold;
			Option optHystresis;
			Option optLowLevel;
			Option optHighLevel;
            Event evRising;
            Event evFalling;
        };

        struct State
        {
            U32 uLastFrame;
            bool bState;
            F32 fPrevious;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        GateSwitch(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "GateSwitch"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

        static U32 Size() { return sizeof(GateSwitch); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->ctrlInput.sCom = -1;
			pData->ctrlInput.uOut = 0;
            pData->optThreshold.ctrl.sCom = -1;
            pData->optThreshold.ctrl.uOut = 0;
            pData->optThreshold.fVal = 0.5f;
            pData->optHystresis.ctrl.sCom = -1;
            pData->optHystresis.ctrl.uOut = 0;
            pData->optHystresis.fVal = 0.f;
            pData->optLowLevel.ctrl.sCom = -1;
            pData->optLowLevel.ctrl.uOut = 0;
            pData->optLowLevel.fVal = 0.f;
            pData->optHighLevel.ctrl.sCom = -1;
            pData->optHighLevel.ctrl.uOut = 0;
			pData->optHighLevel.fVal = 1.f;
			pData->evRising.sCom = -1;
			pData->evRising.uMsg = 0;
			pData->evFalling.sCom = -1;
			pData->evFalling.uMsg = 0;
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
        inline void Start();
        inline void Stop();
        inline void Reset();

    private:
    };
}

#endif // _GATESWITCH_H_
