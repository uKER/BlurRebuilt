#ifndef _Variable_H_
#define _Variable_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Variable
////////////////////////////////////////////////////////////////////////////////
    class Variable : public /*IVariable, public */Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  default
            AudioFloat fDefault;
        };

        struct State
        {
            F32 fValue;
        };

        enum Controls
        {
            ValueCtrl,
        };

    public:
        Variable(Object* pObject) : Component(pObject)
        {
        }
        ~Variable() {}

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

        const char* Type() const { return "Variable"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagControl | flagVariable); }

        static U32 Size() { return sizeof(Variable); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

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

		void SetValueDirect(Instance* pInstance, F32 fValue);

    private:
    };
}

#endif // _OBJECT_H_
