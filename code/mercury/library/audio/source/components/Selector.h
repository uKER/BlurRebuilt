#ifndef _SELECTOR_H_
#define _SELECTOR_H_

#define AUDIO_SELECTOR_MAX_WEIGHTS 32

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Selector
////////////////////////////////////////////////////////////////////////////////
    class Selector : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optReset;
            ArraySize uNumElements;
        };

        struct State
        {
			F32 fCurrent;
			F32 afWeights[ AUDIO_SELECTOR_MAX_WEIGHTS ];
			F32 fTotal;
        };

        struct Element
        {
            Option optElement;
            Option optWeight;
            Option optScale;
			Option optOffset;
        };

        enum Controls
        {
            ValueCtrl,
			WeightCtrl
        };

        enum Messages
        {
            SelectMsg,
            ResetMsg,

            NumMessages
        };

    public:
        Selector(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
		void ReleaseContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage(Instance* pInstance, U32 uMessage);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "Selector"; }

        const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

		static U32 Size() { return sizeof(Selector); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uNumElements * sizeof(Element); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->uNumElements = 0;
			pData->optReset.ctrl.sCom = -1;
			pData->optReset.ctrl.uOut = 0;
			pData->optReset.fVal = 0.f;
        }

        static void DefaultDataElement(Element* pData)
        {
			pData->optElement.ctrl.sCom = -1;
			pData->optElement.ctrl.uOut = 0;
			pData->optElement.fVal = 0.f;
			pData->optWeight.ctrl.sCom = -1;
			pData->optWeight.ctrl.uOut = 0;
			pData->optWeight.fVal = 1.f;
			pData->optScale.ctrl.sCom = -1;
			pData->optScale.ctrl.uOut = 0;
			pData->optScale.fVal = 0.f;
			pData->optOffset.ctrl.sCom = -1;
			pData->optOffset.ctrl.uOut = 0;
			pData->optOffset.fVal = 1.f;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->uNumElements < 2)
                return false;
            else
                return true;
        }

        static bool ValidateDataElement(Element* pData)
        {
            if (pData->optWeight.fVal <= 0.f)
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
			if (strcmp(pchMessage, "Select") == 0)
				return 0;
			if (strcmp(pchMessage, "Reset") == 0)
				return 1;
			else
				return -1;
		}

    private:
        void Select(Instance* pInstance);
		void Reset(Instance* pInstance);

	private:
        ixInit m_uElementOffset;
    };
}

#endif // _SELECTOR_H_
