#ifndef _SETUP_SCRIPT_H_
#define _SETUP_SCRIPT_H_


#include "AppInterfaceLayer/PagedArray.h"


namespace Audio
{

#define MAX_NUM_STRING_PAGES		32
#define STRING_PAGE_SIZE			512
#define	MAX_NUM_SCRIPT_LINES		32
#define	MAX_NUM_SCRIPT_PARAMETERS	32


	// forward decls
	class SetupScriptManager;


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	structs
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

	//struct StringLookup
	//{
	//	U16					m_u16PageIndex;
	//	U16					m_u16StringIndex;
	//};


	struct ScriptLine
	{
		SetupScriptCommand			m_eCommand;
		U32							m_Param1StringOffset;
		U32							m_Param2StringOffset;
//		StringLookup				m_Param1String;
//		StringLookup				m_Param2String;
		U16							m_Param1Index;
		U16							m_Param2Index;
	};


	struct SetupScriptParameter
	{
		void*						m_pData;	// will become a handle probably
		EnvironmentVariableType				m_eType;
	};


	struct ScriptLineDef
	{
//		SetupScriptHandle hScript;
//		SetupScriptCommand eCommand;
		char* strParam1;
		char* strParam2;
		U16 u16Param1Index;
		U16 u16Param2Index;
	};
/*
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupStringManager
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
	class SetupStringManager
	{

	public:
		SetupStringManager()
		{
			for (U32 i = 0; i < MAX_NUM_STRING_PAGES; i++)
			{
				m_abPageInUse[i] = false;
				m_apPage[i] = 0;
				m_au16NextPage[i] = (U16)-1;
				m_au16CurrentStringIndex[i] = 0;
				m_u16CurrentPage = (U16)-1;
			}
		}
		~SetupStringManager()
		{
			for (U32 i = 0; i < MAX_NUM_STRING_PAGES; i++)
			{
				if (m_apPage[i])
					delete [] m_apPage[i];

				m_apPage[i] = 0;
			}
		}

		U16 RequestNewPage()
		{
			for (U32 i = 0; i < MAX_NUM_STRING_PAGES; i++)
			{
				if (! m_abPageInUse[i])
				{
					if (! m_apPage[i])
						m_apPage[i] = new char[STRING_PAGE_SIZE];

					if (m_apPage[i])
					{
						m_abPageInUse[i] = true;
						m_u16CurrentPage = i;
						return i;
					}
				}
			}

			return (U16)-1;
		}
		StringLookup AllocateString(char* str)
		{
			Assert (strlen(str) < STRING_PAGE_SIZE);

			if (m_au16CurrentStringIndex[m_u16CurrentPage] + strlen(str) >= STRING_PAGE_SIZE)
				m_u16CurrentPage = RequestNewPage();

			Assert(m_u16CurrentPage != (U16)-1);

			StringLookup retvalue;

			retvalue.m_u16PageIndex = m_u16CurrentPage;
			retvalue.m_u16StringIndex = m_au16CurrentStringIndex[m_u16CurrentPage];

			strcpy(m_apPage[m_u16CurrentPage] + m_au16CurrentStringIndex[m_u16CurrentPage], str);
			m_au16CurrentStringIndex[m_u16CurrentPage] += strlen(str);

			return retvalue;
		}
		char* GetString(StringLookup& stringLookup)
		{
			Assert(m_abPageInUse[stringLookup.m_u16PageIndex]);
			Assert(stringLookup.m_u16StringIndex < m_au16CurrentStringIndex[stringLookup.m_u16PageIndex]);

			return m_apPage[stringLookup.m_u16PageIndex] + stringLookup.m_u16StringIndex;
		}
		void ReleasePage(U16 u16PageIndex)
		{
			m_abPageInUse[u16PageIndex] = false;
			m_au16CurrentStringIndex[u16PageIndex] = 0;

			if (m_au16NextPage[u16PageIndex] != (U16)-1)
			{
				ReleasePage(m_au16NextPage[u16PageIndex]);
				m_au16NextPage[u16PageIndex] = (U16)-1;
			}
		}

	private:
		char*						m_apPage[MAX_NUM_STRING_PAGES];
		U16							m_au16NextPage[MAX_NUM_STRING_PAGES];
		U16							m_au16CurrentStringIndex[MAX_NUM_STRING_PAGES];
		bool						m_abPageInUse[MAX_NUM_STRING_PAGES];
		U16							m_u16CurrentPage;
	};
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptDefinition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
	class SetupScriptDefinition
	{

	public:
		SetupScriptDefinition() { m_pData = 0; m_s32ActiveScriptCount = 0; }
		~SetupScriptDefinition() { if (m_pData) { delete [] m_pData; m_pData = 0; } }

		bool						Load(const char* strFilename);
		inline char*				GetString(U32 u32Offset) { return &(m_pData[m_u32StringTableOffset + u32Offset]); }
		void						ScriptStarted()		{ m_Lock.Increment(&m_s32ActiveScriptCount, 1); }
		void						ScriptFinished()	{ m_Lock.Increment(&m_s32ActiveScriptCount, -1); }

		Core::Lock					m_Lock;
		char*						m_pData;
		U32							m_u32NumInstructions;
		U32							m_u32StringTableOffset;
		volatile S32				m_s32ActiveScriptCount;
	};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScript
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
	class SetupScript
	{

	public:
		SetupScript(SetupScriptManager* pManager);
		~SetupScript();

//		bool						AddScriptLine(SetupScriptCommand eCommand, ScriptLineDef& scriptLine);
//		U16							AddScriptParameter(const char* const strParam);
		U16							AddInputParameter(EnvironmentVariableType eType, const void* const pData);
		bool						Parse(ObjectManager* pObjectManager);
		inline bool					HasBeenSubmitted() { return m_bSubmittedForParsing; }
		inline bool					HasCompleted() { return (m_s32ScriptFinished == 1); }
		inline void					Submit() { m_bSubmittedForParsing = true; m_pDefinition->ScriptStarted(); }
		bool						SetupFromDefinition(SetupScriptDefinition* pDef);

	private:
		ScriptLine					m_aLine[MAX_NUM_SCRIPT_LINES];
		SetupScriptParameter		m_aParameter[MAX_NUM_SCRIPT_PARAMETERS];
		SetupScriptManager*			m_pManager;
		Core::Lock					m_Lock;
		U32							m_u32CurrentScriptLine;
		U32							m_u32CurrentParameter;
		U32							m_u32FirstLineFail;
		U32							m_u32NumErrors;
		volatile S32				m_s32ScriptFinished;
		SetupScriptDefinition*		m_pDefinition;
		bool						m_bParsed;
		bool						m_bSubmittedForParsing;
	};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

	class SetupScriptManager : public ISetupScriptManager
	{

//		friend class SetupStringManager;
		friend class SetupScript;

	public:
		SetupScriptManager();
		~SetupScriptManager();

		SetupScriptHandle					CreateManualScript();
		SetupScriptDefHandle				CreateManualScriptDefinition();
		SetupScriptHandle					CreateScript(SetupScriptDefHandle hDef);
		//bool								AddScriptLineToManualScript(SetupScriptHandle hScript, SetupScriptCommand eCommand, ScriptLineDef& scriptLine);
		//bool								AddScriptLineToManualDefinition(SetupScriptDefHandle hDef, SetupScriptCommand eCommand, ScriptLineDef& scriptLine);
		U16									AddInputParameter(SetupScriptHandle hScript, EnvironmentVariableType eType, const void* const pData);
		bool								SubmitScript(SetupScriptHandle hScript);
		bool								Parse(SetupScriptHandle hScript);
		SetupScriptDefHandle				LoadDefinition(const char* const strFilename);

	private:
//		StringLookup						AllocateString(char* str) { return m_StringManager.AllocateString(str); }
//		const char*							GetString(StringLookup& stringLookup) { return m_StringManager.GetString(stringLookup); }
//		inline const char*					GetString(U16 u16PageIndex, U16 u16StringIndex) { StringLookup lookup; lookup.m_u16PageIndex = u16PageIndex; lookup.m_u16StringIndex = u16StringIndex; return GetString(lookup); }

		PagedArray<SetupScript*>			m_vScript;
		PagedArray<SetupScriptDefinition*>	m_vDefinition;
//		SetupStringManager					m_StringManager;
	};
}

#endif	//	#ifndef _SETUP_SCRIPT_H_
