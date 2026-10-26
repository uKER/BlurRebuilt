#ifndef _PRODUCT_H_
#define _PRODUCT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Product
////////////////////////////////////////////////////////////////////////////////
    class Product : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  num inputs
            ArraySize uNumInputs;
        };

        struct State
        {
            F32 fValue;
            U32 uLastFrame;
        };

        struct Input
        {
            //  input
			Option optInput;
        };

        enum Controls
        {
            ValueCtrl
        };

    public:
        Product(Object* pObject) : Component(pObject)
        {
        }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        F32 Value(Instance* pInstance, U32 uOutput = 0);

        const char* Type() const { return "Product"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return flagControl; }

		static U32 Size() { return sizeof(Product); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uNumInputs * sizeof(Input); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->uNumInputs = 0;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->uNumInputs < 2)
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
        ixInit m_uInputOffset;
    };
}

#endif // _PRODUCT_H_
