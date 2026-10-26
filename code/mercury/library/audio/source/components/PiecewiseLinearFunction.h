#ifndef _PIECEWISELINEARFUNCTION_H_
#define _PIECEWISELINEARFUNCTION_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	PiecewiseLinearFunction
////////////////////////////////////////////////////////////////////////////////
    class PiecewiseLinearFunction : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  input
			Control ctrlInput;
            //  curve
			Option optCurve;
            //  num points
            ArraySize uNumPoints;
        };

        struct State
        {
            bool bIsDynamic;
            U32 uLastFrame;
            F32 fPrevIn;
            F32 fPrevOut;
        };

        struct Point
        {
            //  x
			AudioFloat fXVal;
			// y
			Option optY;
        };

        enum Controls
        {
            ValueCtrl
        };

    public:
        PiecewiseLinearFunction(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

        const char* Type() const { return "PiecewiseLinearFunction"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

		static U32 Size() { return sizeof(PiecewiseLinearFunction); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uNumPoints * sizeof(Point); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->ctrlInput.sCom = -1;
			pData->ctrlInput.uOut = 0;
			pData->optCurve.ctrl.sCom = -1;
			pData->optCurve.ctrl.uOut = 0;
            pData->optCurve.fVal = 1.f;
            pData->uNumPoints = 0;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->ctrlInput.sCom == -1)
                return false;
            else if (pData->uNumPoints < 2)
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
        ixInit m_uPointOffset;
    };
}

#endif // _PIECEWISELINEARFUNCTION_H_
