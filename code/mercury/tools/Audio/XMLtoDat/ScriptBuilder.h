#ifndef _SCRIPTBUILDER_H_
#define _SCRIPTBUILDER_H_

#include <string>

// forwards decl's
class ComponentData;

typedef bool (*SCRIPT_ERROR_CALLBACK) ( U32 u32LineNumber, char* strError, char* strFile, char* strScript );


namespace Audio
{

//------------------------------------------------------------------------------
//  class ScriptBuilder
//------------------------------------------------------------------------------
    class ScriptBuilder
    {
    public:
        //  class ComponentInfo
#ifdef AUDIO_PACK_DATA
#pragma pack(push, 1)
#endif // AUDIO_PACK_DATA
        struct InitData
        {
            U32 strID;
            U32 iSleep;
            U32 iStack;
            //  Audio::Float fEpsilon;  //  for compare/equals

        };
#ifdef AUDIO_PACK_DATA
#pragma pack(pop)
#endif // AUDIO_PACK_DATA

        //  stack
        F32* pfStack;
        U32 uStackPtr;

        //  sleep
        U32 uCounter;

        //  state
        U32 uPC;

        bool bActive;

        bool bLessThan;
        bool bGreaterThan;
        bool bEqual;

		SCRIPT_ERROR_CALLBACK errorCallback;
/*
        class ComponentInfo
        {
        public:
            ComponentInfo(std::string& strName, std::string& strType, int iIndex, int iFileID = -1) { m_strName = strName; m_strType = strType; m_iIndex = iIndex; m_iFileID = iFileID; }

            inline int GetIndex() { return m_iIndex; }
            inline std::string& GetName() { return m_strName; }
            inline std::string& GetType() { return m_strType; }
            inline int GetFileID() { return m_iFileID; }

        private:
            int m_iIndex;
            std::string m_strName;
            std::string m_strType;
            int m_iFileID;
        };
*/
        //  class Token
        class Token
        {
        public:
            Token(int iLine) { m_iLine = iLine; }
            ~Token() {}

            inline const int GetLine() const { return m_iLine; }
            inline std::string& GetName() { return m_strName; }

        private:
            int m_iLine;
            std::string m_strName;
        };

        //  class Instruction
        class Instruction
        {
        public:
            Instruction(int iLine) { m_iLine = iLine; m_iOffset = -1; m_wOpCode = 0xffffffff; m_wParam1 = 0xffffffff; m_wParam2 = 0xffffffff; }

            inline void SetOffset(int iOffset) { m_iOffset = iOffset; }
            inline void SetOpCode(U32 wOpCode) { m_wOpCode = wOpCode; }
            inline void SetParam1(U32 dwParam1) { m_wParam1 = dwParam1; }
            inline void SetParam2(U32 dwParam2) { m_wParam2 = dwParam2; }
            inline void SetParam1(F32 fParam1) { m_fParam1 = fParam1; }
            inline void SetParam2(F32 fParam2) { m_fParam2 = fParam2; }

            inline const int GetLine() const { return m_iLine; }
            inline const int GetOffset() const { return m_iOffset; }

            inline std::string& GetOpCodeString() { return m_strOpCode; }
            inline std::string& GetParam1String() { return m_strParam1; }
            inline std::string& GetParam2String() { return m_strParam2; }

            inline U32 GetOpCode() { return m_wOpCode; }
            inline U32 GetParam1() { return m_wParam1; }
            inline U32 GetParam2() { return m_wParam2; }

        private:
            int m_iOffset;
            int m_iLine;

            std::string m_strOpCode;
            std::string m_strParam1;
            std::string m_strParam2;

            U32 m_wOpCode;
			union
			{
	            U32 m_wParam1;
				F32 m_fParam1;
				U32 m_u32ComponentAndOutput;
			};
			union
			{
	            U32 m_wParam2;
				F32 m_fParam2;
				U32 m_u32ComponentAndOutput;
			};
        };

        //  class Label
        class Label
        {
        public:
            Label() { m_pInstruction = NULL; }

            void SetInstruction(Instruction* pInstruction) { m_pInstruction = pInstruction; }

            inline std::string& GetName() { return m_strName; }
            inline Instruction* GetInstruction() { return m_pInstruction; }

        private:
            std::string m_strName;
            Instruction* m_pInstruction;
        };

        //  class Message
        class Message
        {
        public:
            Message(U32 u32ComponentType, char* pchName, int iEnum) { m_u32ComponentType = u32ComponentType; m_strName = pchName; m_iEnum = iEnum; }

            inline const std::string& GetName() const { return m_strName; }
            inline const U32 GetComType() const { return m_u32ComponentType; }
            inline const int GetEnum() const { return m_iEnum; }

        private:
			U32 m_u32ComponentType;
            std::string m_strName;
            int m_iEnum;
        };

    public:
		ScriptBuilder(Audio::Core::Array<ComponentData*> apComponentData, Audio::Core::Array<Constant*> apConstant)
		{
			m_apComponentData = apComponentData;
			m_apConstant = apConstant;
			m_iCurrLine = 0;
			m_iOffset = 0;
			m_pCode = NULL;
			errorCallback = 0;
			m_bContainsAnExit = false;
		}
        ~ScriptBuilder() { Release(); }

        bool Make(const char* pchName, const char* pchSource, ComponentData* pCompData);
        void CleanUp();

		void SetErrorCallback(SCRIPT_ERROR_CALLBACK errCallback) { errorCallback = errCallback; }

		void Release();

        U32* GetCode() { return m_pCode; }
        int GetCodeLength() { return m_iOffset; }

		bool ContainsAnExit();

    private:
        void Init();                            //  make message list
        void Lex(const char* pchText);          //  make token list
        void Parse();                           //  make instruction list (optionally pass a component prefix)
        void Validate(ComponentData* pCompData);//  check semantics
        void Create();                          //  create dword code

		bool GetParam1CompOrFloat(Instruction* pInstruction, Script::OpCodes eCode, const char* strCommand, ComponentData* pCompData);
		bool GetParam2CompOrFloat(Instruction* pInstruction, Script::OpCodes eCode, const char* strCommand, ComponentData* pCompData);
        U32 GetComponentIndex(std::string& strName, ComponentData* pCompData, U32 u32LineNumber = -1);
        int GetLabelOffset(std::string& strName);
        Message* GetMessage(std::string& strComType, std::string& strName, ComponentData* pCompData);
		U32 GetComponentType(std::string& strName);

        char* GetToken(char* pchText, std::string* pstrToken);

    private:
        Audio::List<Token*> m_lTokens;
        Audio::List<Label*> m_lLabels;
	    Audio::List<Instruction*> m_lInstructions;

		Audio::Core::Array<ComponentData*>	m_apComponentData;
		Audio::Core::Array<Constant*>		m_apConstant;

		Audio::Core::Array<Message*> m_lMessages;

        int m_iCurrLine;

        int m_iOffset;

        U32* m_pCode;

		bool m_bContainsAnExit;
    };
}


#endif  // ifndef _SCRIPTBUILDER_H_