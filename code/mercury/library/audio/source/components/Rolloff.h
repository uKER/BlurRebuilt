#ifndef _ROLLOFF_H_
#define _ROLLOFF_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Rolloff
////////////////////////////////////////////////////////////////////////////////
    class Rolloff : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  input
			Control ctrlDistance;
            //S32 sDistanceCom;
            //U32 uDistanceOut;
            //  rolloff factor
			Option optRolloffFactor;
            //  max level
			Option optMaxLevel;
            //  ref level
			Option optRefLevel;
            //  ref distance
			Option optRefDistance;
            //  max distance
			Option optMaxDistance;
        };

        struct State
        {
            F32 fRolloffLevel;

            U32 uLastFrame;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        Rolloff(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

        const char* Type() const { return "Rolloff"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

        static U32 Size() { return sizeof(Rolloff); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->ctrlDistance.sCom = -1;
			pData->ctrlDistance.uOut = 0;
            pData->optRolloffFactor.ctrl.sCom = -1;
            pData->optRolloffFactor.ctrl.uOut = 0;
            pData->optRolloffFactor.fVal = 1.f;
            pData->optMaxLevel.ctrl.sCom = -1;
            pData->optMaxLevel.ctrl.uOut = 0;
            pData->optMaxLevel.fVal = 1.f;
            pData->optRefLevel.ctrl.sCom = -1;
            pData->optRefLevel.ctrl.uOut = 0;
            pData->optRefLevel.fVal = 0.5f;
            pData->optRefDistance.ctrl.sCom = -1;
            pData->optRefDistance.ctrl.uOut = 0;
            pData->optRefDistance.fVal = 3.f;
            pData->optMaxDistance.ctrl.sCom = -1;
            pData->optMaxDistance.ctrl.uOut = 0;
			pData->optMaxDistance.fVal = Maths::fMax;
        }

        static bool ValidateData(Data* pData)
        {
			if (pData->ctrlDistance.sCom == -1)
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

#endif // _ROLLOFF_H_
