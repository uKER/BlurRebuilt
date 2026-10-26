#ifndef _SCRIPT_H_
#define _SCRIPT_H_

namespace Audio
{
////////////////////////////////////////////////////////////////////////////////
//	Script
////////////////////////////////////////////////////////////////////////////////
    class Script : public Component
    {
    public:
        struct Data
        {
            //  name
			StrOffset strName;
            //  code length
            ArraySize uLength;
        };

        struct State
        {
			Btl::Vector<F32, Btl::VectorParameters_Array<16 > > aStack;

            U32 uPC;
            U32 uStack;

            bool bActive;

            bool bEqual;
            bool bGreaterThan;
            bool bLessThan;
        };

        enum OpCodes
        {
            OpBreak = 0x0000,
            OpSend = 0x1000,
            OpPush = 0x1001,
            OpPull = 0x1002,
            OpDiscard = 0x1003,
			OpPushFloat = 0x1004,
            OpJump = 0x1100,
            OpJumpEqual = 0x1101,
            OpJumpNotEqual = 0x1102,
            OpJumpLessThan = 0x1103,
            OpJumpNotLessThan = 0x1104,
            OpJumpGreaterThan = 0x1105,
            OpJumpNotGreaterThan = 0x1106,
            OpCompare = 0x1300,
            OpAdd = 0x1301,
            OpSubtract = 0x1302,
            OpMultiply = 0x1303,
            OpDivide = 0x1304,
			OpAddFloat = 0x1305,
            OpSubtractFloat = 0x1306,
            OpMultiplyFloat = 0x1307,
            OpDivideFloat = 0x1308,
            OpSetEqual = 0x1310,
            OpSetLessThan = 0x1311,
            OpSetGreaterThan = 0x1312,
            OpClearEqual = 0x1314,
            OpClearLessThan = 0x1315,
            OpClearGreaterThan = 0x1316,
			OpCompareParams = 0x1317,
            OpExecute = 0x1200,
            OpCall = 0x1201,
            OpExit = 0x1202,
			OpDestruct = 0x1203
        };

        enum Messages
        {
            ExecuteMsg,

            NumMessages
        };

    public:
        Script(Object* pObject) : Component(pObject)
        {
            m_uCodeOffset = 0;
       }

        void* Init(void* pData);
        U32 ContextSize() { return sizeof(State); }
        void InitContext(Instance* pInstance);
        void Update(Instance* pInstance);
        void DoMessage(Instance* pInstance, U32 uMessage);

        U32 GetNumMessages() { return StaticGetNumMessages(); }
		static U32 StaticGetNumMessages() { return NumMessages; }
        const char* Type() const { return "Script"; }

		const U32 Flags() { return StaticFlags(); }
		static U32 StaticFlags() { return (flagUpdater | flagEvent | flagScript); }

		static U32 Size() { return sizeof(Script); }
		static U32 DataSize(void* pData) { return sizeof(Data) + ((Data*)pData)->uLength * sizeof(U32); }

        static void DefaultData(Data* pData)
        {
            pData->strName = -1;
            pData->uLength = 0;
        }

        static bool ValidateData(Data* pData)
        {
            return true;
        }

		static S32 GetMessage(const char* pchMessage)
		{
			if (strcmp(pchMessage, "Execute") == 0)
				return 0;
			else
				return -1;
		}

        void Execute(Instance* pInstance);
    private:

        bool DoInstruction(Instance* pInstance);

//        void Stop(Instance* pInstance);
        void Reset(U8* pStateData);

    private:
		ixInit m_uCodeOffset;
    };
}

#endif // _SCRIPT_H_
