#ifndef _CONTROL_SCRIPT_PARSER_H_
#define _CONTROL_SCRIPT_PARSER_H_


//#define	MAX_NUM_SETUP_SCRIPT_PARAMS		8
#define MAX_NUM_FUNCTIONS	16


//enum ControlScriptDataType
//{
//	eUnknown = 0,
//	eInstance,
//
//	TotalNumControlScriptDataTypes
//};

namespace Audio
{

	struct ControlScriptError
	{
		ControlScriptError(ControlScriptResult eResultIn, U32 u32LineNumberIn) { u32LineNumber = u32LineNumberIn; eResult = eResultIn; }

		U32 u32LineNumber;
		ControlScriptResult eResult;
	};


	//struct ControlScriptDefinitionParameter
	//{
	//	ControlScriptDefinitionParameter() { strName = 0; eType = eUnknown; u32LowestLineOccurrence = 0; }
	//	~ControlScriptDefinitionParameter() { if (strName) delete [] strName; strName = 0; }
	//
	//	void SetName(char* strNewName) { strName = new char[strlen(strNewName) + 1]; strcpy(strName, strNewName); }
	//	char* strName;
	//	ControlScriptDataType eType;
	//	U32 u32LowestLineOccurrence;
	//};


	struct ControlScriptString
	{
		ControlScriptString() { str = 0; pNext = 0; }
		~ControlScriptString()
		{
			if (str)
			{
				delete [] str;
				str = 0;
			}
			if (pNext)
			{
				delete pNext;
				pNext = 0;
			}
		}

		void SetString(char* strNew) { str = new char[strlen(strNew) + 1]; strcpy(str, strNew); str[strlen(strNew)] = 0; }

		char* str;
		ControlScriptString* pNext;
		U32 u32Offset;
	};


	class ControlScriptParsedInstruction
	{

	public:
		ControlScriptParsedInstruction() { m_pData = 0; m_pNext = 0; m_u32Size = 0; }
		~ControlScriptParsedInstruction() { if (m_pData) delete [] m_pData; if (m_pNext) delete m_pNext; }
		bool								SetNext(ControlScriptParsedInstruction* pNext) { if (m_pNext) return false; m_pNext = pNext; return true; }
		ControlScriptParsedInstruction*		GetNext() { return m_pNext; }
		bool								IsDataSet() { return (m_pData != 0); }
		void*								GetData() { return m_pData; }
		bool								SetData(void* pData, U32 u32Size)
		{
			if (! m_pData)
			{
				m_pData = (void*)(new char[u32Size]);
				memcpy(m_pData, pData, u32Size);
				m_u32Size = u32Size;
				return true;
			}
			return false;
		}
		U32									GetSize() { return m_u32Size; }

	private:
		void*								m_pData;
		ControlScriptParsedInstruction*		m_pNext;
		U32									m_u32Size;
	};


	class ControlScriptParser : public ControlScriptSingleCommandParser
	{

	public:
		ControlScriptParser();
		~ControlScriptParser();

		bool								Load(const char* const strFilename) { return Open(strFilename); }
		bool								Open(const char* const pchFilename)
		{
			Close();
			m_pFileIn = fopen(pchFilename, "rb");
			return (m_pFileIn != NULL);
		}
		bool								Close()
		{
			int iRet = 0;
			if (m_pFileIn)
			{
				int iRet = fclose(m_pFileIn);
				m_pFileIn = NULL;
			}
			return (iRet != EOF);
		}
		bool								Parse();
		bool								Output(char* strOutputFilename, CommandLineApp* app);

	private:
		bool								AddNewInstruction();
		U32									GetCurrentInstructionIndex() { return m_u32InstructionNumber; }
		bool								IsInstructionSet();
		void								DoError(ControlScriptResult eResult, U32 u32LineNumber, char* strInfo = 0);
		U32									AddString(char* str);
		bool								CheckStringExists(char* str);
		bool								AddFunctionTableEntry(U32 u32StartInstruction, U32 u32FunctionNameStringIndex);
		bool								EndCurrentFunction(U32 u32EndInstruction);
		char ReadChar()
		{
			return getc(m_pFileIn);
		}

		ControlScriptParsedInstruction*		m_pFirstScriptLine;
		ControlScriptParsedInstruction*		m_pLastScriptLine;
		ControlScriptString*				m_pFirstString;
		ControlScriptString*				m_pLastString;
		ControlScriptFunction				m_aFunctions[MAX_NUM_FUNCTIONS];
	//	ControlScriptDefinitionParameter	m_aParameter[MAX_NUM_SETUP_SCRIPT_PARAMS];
		U32									m_u32NumParameters;
		U32									m_u32StringTableSize;
		U32									m_u32InstructionNumber;
	    FILE*								m_pFileIn;
		U32									m_u32NumErrors;
		U32									m_u32NumFunctionTableEntries;
	};
}

#endif	//	_CONTROL_SCRIPT_PARSER_H_
