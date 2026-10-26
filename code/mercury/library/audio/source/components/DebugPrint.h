#ifndef _DEBUGPRINT_H_
#define _DEBUGPRINT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	DebugPrint
////////////////////////////////////////////////////////////////////////////////
    class DebugPrint : public Component
    {
    public:
        struct Data
        {
			StrOffset strName;
			StrOffset strText;
		    Control ctrlInput;
        };

#ifndef CFG_GOLD
        struct State
        {
			bool bFired;
		};
#endif

        enum Messages
        {
            PrintMsg,

            NumMessages
        };

    public:
        DebugPrint(Object* pObject) : Component(pObject)
        {
        }

#ifndef PLATFORM_SPU
        void* Init(void* pData);
#ifndef CFG_GOLD
        U32 ContextSize() { return sizeof(State); }
#else
        U32 ContextSize() { return 0; }
#endif
        void InitContext(Instance* pInstance);
#endif	//	#ifndef PLATFORM_SPU
        void DoMessage(Instance* pInstance, U32 uMessage);

#if !defined (PLATFORM_SPU) && ! defined (CFG_GOLD)
		void ProcessConnection(Instance* pInstance);
#endif


#ifndef PLATFORM_SPU
        U32 GetNumMessages() { return StaticGetNumMessages(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "DebugPrint"; }

#ifndef PLATFORM_SPU
		const U32 Flags() { return StaticFlags(); }
#endif	//	#ifndef PLATFORM_SPU
		static U32 StaticFlags() { return 0; }

        static U32 Size() { return sizeof(DebugPrint); }
		static U32 DataSize(void* pData) { return sizeof(Data); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->strText = -1;
            pData->ctrlInput.sCom = -1;
            pData->ctrlInput.uOut = 0;
        }

        static bool ValidateData(Data* pData)
        {
            if (pData->ctrlInput.sCom == -1)
                return false;
            else
                return true;
        }

    protected:
		void Print(Instance* pInstance);

    private:
    };
}

#endif // _DEBUGPRINT_H_
