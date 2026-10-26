#ifndef _PARAMETEROUT_H_
#define _PARAMETEROUT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	ParameterOut
////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_SPU
    class ParameterOut : public Component
#else
    class ParameterOut : /*public IParameterOut, */public Component
#endif
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  input
			Option optInput;
        };

        struct State
        {
            F32 fValue;
		};

    public:
        ParameterOut(Object* pObject) : Component(pObject)
        {
        }
        ~ParameterOut();

        void* Init(void* pData);
#ifndef PLATFORM_SPU
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
		void ReleaseContext(Instance* pInstance);
#endif
		void Update(Instance* pInstance);

        const char* Type() const { return "ParameterOut"; }
        const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagParameterOut); }

        static U32 Size() { return sizeof(ParameterOut); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        F32 Value(Instance* pInstance);

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
			pData->optInput.ctrl.sCom = -1;
			pData->optInput.ctrl.uOut = 0;
			pData->optInput.fVal = 0.f;
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
    };
}

#endif // _PARAMETEROUT_H_
