#ifndef _METRONOME_H_
#define _METRONOME_H_


namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Metronome
////////////////////////////////////////////////////////////////////////////////
    class Metronome : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optTempo;
            Option optBeatsPerBar;
            Option optLength;
			Event evBeat;
			Event evBar;
			Event evFinished;
        };

        struct State
        {
            bool bActive;
            F32 fFraction;
            F32 fBeat;
            F32 fBar;
        };

        enum Controls
        {
            FractionCtrl,
            BeatCtrl,
            BarCtrl,
        };

        enum Messages
        {
            StartMsg,
            StopMsg,
            ResetMsg,

            NumMessages
        };

    public:
        Metronome(Object* pObject) : Component(pObject)
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
        const char* Type() const { return "Metronome"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

        static U32 Size() { return sizeof(Metronome); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->optTempo.ctrl.sCom = -1;
            pData->optTempo.ctrl.uOut = 0;
            pData->optTempo.fVal = 120.f;
            pData->optBeatsPerBar.ctrl.sCom = -1;
            pData->optBeatsPerBar.ctrl.uOut = 0;
            pData->optBeatsPerBar.fVal = 4.f;
            pData->optLength.ctrl.sCom = -1;
            pData->optLength.ctrl.uOut = 0;
            pData->optLength.fVal = 300.f;
			pData->evBeat.sCom = -1;
			pData->evBeat.uMsg = 0;
			pData->evBar.sCom = -1;
			pData->evBar.uMsg = 0;
			pData->evFinished.sCom = -1;
			pData->evFinished.uMsg = 0;
		}

        static bool ValidateData(Data* pData)
        {
            return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Fraction") == 0)
                return 0;
            else if (strcmp(pchControl, "Beat") == 0)
                return 1;
            else if (strcmp(pchControl, "Bar") == 0)
                return 2;
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
			else
				return -1;
		}


    private:
        inline void Start(Instance* pInstance);
        inline void Stop(Instance* pInstance);
        inline void Reset(Instance* pInstance);

    private:
    };
}

#endif // _METRONOME_H_
