#ifndef _SEQUENCE_H_
#define _SEQUENCE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Sequence
////////////////////////////////////////////////////////////////////////////////
    class Sequence : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			Control ctrlInput;
            ArraySize uNumElements;
        };

        struct State
        {
            F32 fCurrentIn;
            F32 fCurrentOut;
            U32 uLastFrame;
        };

        struct Element
        {
			Element()
			{
				fTime = 0.f;
				evEvent.sCom = -1;
				evEvent.uMsg = 0;
				optValue.ctrl.sCom = -1;
				optValue.ctrl.uOut = 0;
				optValue.fVal = 0.f;
			}

			AudioFloat fTime;
			Event evEvent;
			Option optValue;
        };

        enum Controls
        {
            ValueCtrl
        };

    public:
        Sequence(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
		void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

        const char* Type() const { return "Sequence"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

		static U32 Size() { return sizeof(Sequence); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uNumElements * sizeof(Element); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->ctrlInput.sCom = -1;
			pData->ctrlInput.uOut = 0;
            pData->uNumElements = 0;
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

    private:
        ixInit m_uElementOffset;
    };
}

#endif // _SEQUENCE_H_
