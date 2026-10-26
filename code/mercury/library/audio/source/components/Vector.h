#ifndef _VECTOR_H_
#define _VECTOR_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Vector
////////////////////////////////////////////////////////////////////////////////
    class Vector : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            Option optX;
            Option optY;
            Option optZ;
            Com sInputVec;
		};

        struct State
        {
            F32 fX;
            F32 fY;
            F32 fZ;
            F32 fMagnitude;
        };

        enum Controls
        {
            MagnitudeCtrl,
            XCtrl,
            YCtrl,
            ZCtrl,
        };

    public:
        Vector(Object* pObject) : Component(pObject)
        {
        }
        ~Vector() {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void Coords(Instance* pInstance, Maths::Vector* pVector);
        void Update(Instance* pInstance);

        const char* Type() const { return "Vector"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagControl | flagVector | flagUpdater); }

        static U32 Size() { return sizeof(Vector); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->optX.ctrl.sCom = -1;
			pData->optX.ctrl.uOut = 0;
			pData->optX.fVal = 0.f;
			pData->optY.ctrl.sCom = -1;
			pData->optY.ctrl.uOut = 0;
			pData->optY.fVal = 0.f;
			pData->optZ.ctrl.sCom = -1;
			pData->optZ.ctrl.uOut = 0;
			pData->optZ.fVal = 0.f;
			pData->sInputVec = -1;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Magnitude") == 0)
                return 0;
            else if (strcmp(pchControl, "X") == 0)
                return 1;
            else if (strcmp(pchControl, "Y") == 0)
                return 2;
            else if (strcmp(pchControl, "Z") == 0)
                return 3;
            else
                return -1;
        }

    private:
    };
}

#endif // _VECTOR_H_
