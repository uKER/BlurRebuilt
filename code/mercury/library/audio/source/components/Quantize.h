#ifndef _QUANTIZE_H_
#define _QUANTIZE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Quantize
////////////////////////////////////////////////////////////////////////////////
    class Quantize : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			Control ctrlInput;
			Option optStep;
			Option optOffset;
        };

        struct State
        {
            U32 uLastFrame;
            F32 fPrevious;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        Quantize(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

        const char* Type() const { return "Quantize"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

        static U32 Size() { return sizeof(Quantize); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->ctrlInput.sCom = -1;
			pData->ctrlInput.uOut = 0;
			pData->optStep.ctrl.sCom = -1;
			pData->optStep.ctrl.uOut = 0;
			pData->optStep.fVal = 1.f;
			pData->optOffset.ctrl.sCom = -1;
			pData->optOffset.ctrl.uOut = 0;
			pData->optOffset.fVal = 0.f;
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

#endif // _QUANTIZE_H_
