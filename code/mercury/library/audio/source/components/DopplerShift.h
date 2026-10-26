#ifndef _DOPPLERSHIFT_H_
#define _DOPPLERSHIFT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	DopplerShift
////////////////////////////////////////////////////////////////////////////////
    class DopplerShift : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
			Option optSpeedOfSound;
			// input vectors
            Com sInputSourceVelocityVec;
            Com sInputListenerVelocityVec;
			Com sInputSourcePositionVec;
			Com sInputListenerPositionVec;
        };

        struct State
        {
            F32 fDopplerShiftFactor;
			F32 fDopplerShiftPitchFactor;

			F32 outputVecX;
			F32 outputVecY;
			F32 outputVecZ;
        };

        enum Controls
        {
            ValueCtrl,
			FrequencyCtrl,
        };

    public:
        DopplerShift(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        F32 Value(Instance* pInstance, U32 uOutput = 0);
		void Coords(Instance* pInstance, Maths::Vector* pVector);
        void Update(Instance* pInstance);

#ifndef PLATFORM_SPU
        const char* Type() const { return "DopplerShift"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

        static U32 Size() { return sizeof(DopplerShift); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->optSpeedOfSound.ctrl.sCom = -1;
			pData->optSpeedOfSound.ctrl.uOut = 0;
			pData->optSpeedOfSound.fVal = 343.f;
			pData->sInputSourceVelocityVec = -1;
            pData->sInputListenerVelocityVec = -1;
			pData->sInputSourcePositionVec = -1;
			pData->sInputListenerPositionVec = -1;
        }

        static bool ValidateData(Data* pData)
        {
			return ((pData->sInputSourceVelocityVec != -1) &&
				(pData->sInputListenerVelocityVec != -1) &&
				(pData->sInputSourcePositionVec != -1)&& 
				(pData->sInputListenerPositionVec != -1));
        }

        static S32 GetControl(const char* pchControl)
        {
            if (strcmp(pchControl, "Value") == 0)
                return 0;
			if (strcmp(pchControl, "Frequency") == 0)
                return 1;
            else
                return -1;
        }

    private:
    };
}

#endif // _DOPPLERSHIFT_H_
