#ifndef _PARAMETERIN_H_
#define _PARAMETERIN_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	ParameterIn
////////////////////////////////////////////////////////////////////////////////
    class ParameterIn : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
            AudioFloat fDefault;
        };

        struct State
        {
            F32 fValue;
			bool bDisable;
        };

        enum Controls
        {
            ValueCtrl,
        };

        enum Messages
        {
            ResetMsg,

            NumMessages
        };

        struct Packet
        {
            ParameterIn* pThis;
            State* pState;
            F32 fValue;
        };

    public:
        ParameterIn(Object* pObject) : Component(pObject)
        {
        }
        ~ParameterIn();

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);
        void DoMessage(Instance* pInstance, U32 uMessage);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "ParameterIn"; }
        const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagControl | flagParameterIn); }

        static U32 Size() { return sizeof(ParameterIn); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        //  Queue methods
        bool SetValue(Instance* pInstance, F32 fValue);

		//	For profiler
        bool ForceSetValue(Instance* pInstance, F32 fValue);
        void SetDisableFlag(Instance* pInstance, bool bDisable);

		F32 GetDefaultValue();

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->fDefault = 0.f;
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

		static S32 GetMessage(const char* pchMessage)
		{
			if (strcmp(pchMessage, "Reset") == 0)
				return 0;
			else
				return -1;
		}

    private:
		void Reset( Instance* pInstance );
	};
}

#endif // _PARAMETERIN_H_
