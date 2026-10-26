#ifndef _VECTORIN_H_
#define _VECTORIN_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	VectorIn
////////////////////////////////////////////////////////////////////////////////
    class VectorIn : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            AudioFloat fX;
            AudioFloat fY;
            AudioFloat fZ;
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

        struct Packet
        {
            VectorIn* pThis;
            State* pState;
            F32 fX;
            F32 fY;
            F32 fZ;
        };

    public:
        VectorIn(Object* pObject) : Component(pObject)
        {
        }
        ~VectorIn() {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void Coords(Instance* pInstance, Maths::Vector* pVector);

        const char* Type() const { return "VectorIn"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagControl | flagVectorIn); }

        static U32 Size() { return sizeof(VectorIn); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        //  Queue methods
        bool SetDimensions(Instance* pInstance, F32* afDimensions);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->fX = 0.f;
            pData->fY = 0.f;
            pData->fZ = 0.f;
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

#endif // _VECTORIN_H_
