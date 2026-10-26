#ifndef _VectorOut_H_
#define _VectorOut_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	VectorOut
////////////////////////////////////////////////////////////////////////////////
    class VectorOut : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  input
            S32 sInputXCom;
            U32 uInputXOut;
            AudioFloat fInputXVal;

			S32 sInputYCom;
            U32 uInputYOut;
            AudioFloat fInputYVal;

			S32 sInputZCom;
            U32 uInputZOut;
           AudioFloat fInputZVal;

        };

        struct State
        {
            F32 fValueX;
			F32 fValueY;
			F32 fValueZ;
#ifndef PLATFORM_PS3
#ifndef SN_TARGET_PS3_SPU
			CRITICAL_SECTION csLock;
#endif
#endif
		};

		 enum Controls
        {
            ValueXCtrl,
			ValueYCtrl,
			ValueZCtrl,
        };

    public:
        VectorOut(Object* pObject) : Component(pObject)
        {
        }
        ~VectorOut();

        void* Init(void* pData);
#ifndef SN_TARGET_PS3_SPU
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
		void ReleaseContext(Instance* pInstance);
#endif
		void Update(Instance* pInstance);

        const char* Type() { return "VectorOut"; }
        const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagVectorOut); }

        static U32 Size() { return sizeof(VectorOut); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        void GetDimensions(Instance* pInstance, F32* valueX,F32* valueY,F32* valueZ);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->sInputXCom = -1;
            pData->uInputXOut = 0;
            pData->fInputXVal = 0.f;

			pData->sInputYCom = -1;
            pData->uInputYOut = 0;
            pData->fInputYVal = 0.f;

			pData->sInputZCom = -1;
            pData->uInputZOut = 0;
            pData->fInputZVal = 0.f;

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

#endif // _VectorOut_H_
