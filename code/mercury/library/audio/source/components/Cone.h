#ifndef _CONE_H_
#define _CONE_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
///	\brief This class describes an audio cone component.
///
///	A cone can have both forward and rear facing volumes with independent levels 
///	and angles for each. The output value will blend between the front and rear 
///	an arbitrary curve for the blend. The output is dependent on an input angle 
///	of incidence.
////////////////////////////////////////////////////////////////////////////////
    class Cone : public Component
    {
    public:
		///	Cone init data structure
        struct Data
        {
			///	Component name string table index
			StrOffset strName;
			///	Input control for angle of incidence
            Control ctrlIncidentAngle;
			///	Option for cone inner angle (-1 to +1), default = 0
            Option optInnerAngle;
			///	Option for cone inner gain (dB), default = 0
            Option optInnerGain;
			///	Option for cone inner level (0 to 1), default = 1
            Option optInnerLevel;
			///	Option for cone inner angle (-1 to +1), default = 1
            Option optOuterAngle;
			///	Option for cone outer gain (dB), default = -6
            Option optOuterGain;
			///	Option for cone outer level (0 to 1), default = 0.5
            Option optOuterLevel;
			///	Option for curve shape (0 to 1), default = 1 (log curve)
            Option optCurve;
        };

        struct State
        {
            F32 fConeLevel;
            U32 uLastFrame;
            F32 fIncidentAngle;
            F32 fInnerAngle;
            F32 fInnerLevel;
            F32 fOuterAngle;
            F32 fOuterLevel;
            F32 fCurve;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        Cone(Object* pObject);

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
		///	This function returns the output control value for this component
		///	\param pInstance a pointer to the parent instance
		///	\param uOutput an unsigned integer specifying which output value the component should return
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "Cone"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return flagControl; }

		static U32 Size() { return sizeof(Cone); }
        static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->ctrlIncidentAngle.sCom = -1;
            pData->ctrlIncidentAngle.uOut = 0;
            pData->optInnerAngle.ctrl.sCom = -1;
            pData->optInnerAngle.ctrl.uOut = 0;
            pData->optInnerAngle.fVal = 0.f;
            pData->optInnerGain.ctrl.sCom = -1;
            pData->optInnerGain.ctrl.uOut = 0;
            pData->optInnerGain.fVal = 0.f;
            pData->optInnerLevel.ctrl.sCom = -1;
            pData->optInnerLevel.ctrl.uOut = 0;
            pData->optInnerLevel.fVal = 1.f;
            pData->optOuterAngle.ctrl.sCom = -1;
            pData->optOuterAngle.ctrl.uOut = 0;
            pData->optOuterAngle.fVal = 1.f;
            pData->optOuterGain.ctrl.sCom = -1;
            pData->optOuterGain.ctrl.uOut = 0;
            pData->optOuterGain.fVal = -6.f;
            pData->optOuterLevel.ctrl.sCom = -1;
            pData->optOuterLevel.ctrl.uOut = 0;
            pData->optOuterLevel.fVal = 0.5f;
            pData->optCurve.ctrl.sCom = -1;
            pData->optCurve.ctrl.uOut = 0;
            pData->optCurve.fVal = 1.f;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->ctrlIncidentAngle.sCom == -1)
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

#endif // _CONE_H_
