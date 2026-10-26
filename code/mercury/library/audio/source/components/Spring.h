#ifndef _SPRING_H_
#define _SPRING_H_

#ifdef _MSC_VER
#pragma once
#endif

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Spring
////////////////////////////////////////////////////////////////////////////////
    class Spring : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
		    Control ctrlInput;
		    Option optFrequency;
		    Option optDamping;
        };

        struct State
        {
			F32 fPosition;
			F32 fVelocity;
        };

        enum Controls
        {
            PositionCtrl,
            SpeedCtrl,
        };

    public:
        Spring(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
        U32 ContextSize();
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
		void Update(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

#ifndef PLATFORM_SPU
        const char* Type() const { return "Spring"; }

		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return (flagUpdater | flagControl); }

        static U32 Size() { return sizeof(Spring); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->ctrlInput.sCom = -1;
            pData->ctrlInput.uOut = 0;
            pData->optFrequency.ctrl.sCom = -1;
            pData->optFrequency.ctrl.uOut = 0;
            pData->optFrequency.fVal = 5.f;
            pData->optDamping.ctrl.sCom = -1;
            pData->optDamping.ctrl.uOut = 0;
            pData->optDamping.fVal = 0.2f;
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

#endif // _SPRING_H_

