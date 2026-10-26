#ifndef _VECTOR_NEAREST_POINT_ON_LINE_H_
#define _VECTOR_NEAREST_POINT_ON_LINE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	VectorNearestPointOnLine
////////////////////////////////////////////////////////////////////////////////
    class VectorNearestPointOnLine : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
			// input vectors
            Com sInputAVec;
            Com sInputBVec;
			Com sInputAlignVec;
		};

        struct State
        {
            Maths::Vector vOutputVec;
        };

        enum Controls
        {
            MagnitudeCtrl,
            XCtrl,
            YCtrl,
            ZCtrl,
        };

    public:
        VectorNearestPointOnLine(Object* pObject) : Component(pObject)
        {
        }
        ~VectorNearestPointOnLine() {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void Coords(Instance* pInstance, Maths::Vector* pVector);
        void Update(Instance* pInstance);

        const char* Type() const { return "VectorNearestPointOnLine"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagControl | flagVector | flagUpdater); }

        static U32 Size() { return sizeof(VectorNearestPointOnLine); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->sInputAVec = -1;
            pData->sInputBVec = -1;
			pData->sInputAlignVec = -1;
        }

        static bool ValidateData(Data* pData)
        {
            return ((pData->sInputAVec != -1) &&
					(pData->sInputBVec != -1) &&
					(pData->sInputAlignVec != -1));
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

#endif // _VECTOR_NEAREST_POINT_ON_LINE_H_
