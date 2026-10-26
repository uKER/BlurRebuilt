#ifndef _DIAGNOSTICS_H_
#define _DIAGNOSTICS_H_


bool ReceiveParserError(unsigned int u32LineNum, char* strError, char* strFile);
bool ReceiveScriptError(U32 u32LineNum, char* strError, char* strFile, char* strScript);


enum ErrorLevel
{
	eSuccess = 0,
	eWarning,
	eError,
	eFatalError
};


struct ErrorData
{
	ErrorData()
	{
		m_eErrorLevel = eError;
		m_strFilename = 0;
		m_strComponent = 0;
		m_u32LineNumber = -1;
		m_u32ScriptLineNumber = -1;
		m_strErrorString = 0;
		m_strScriptName = 0;
	};

	ErrorLevel			m_eErrorLevel;
	char*				m_strFilename;
	char*				m_strComponent;
	U32					m_u32LineNumber;
	U32					m_u32ScriptLineNumber;
	char*				m_strErrorString;
	char*				m_strScriptName;
};


class Diagnostics
{

public:
	Diagnostics();
	~Diagnostics() { }

	bool				ReceiveError(ErrorData* pErrorData);
	void				FinalReport(bool bAlwaysPause);
	void				SetStrictParsingFlag(bool bStrict)	{ m_bStrictParsing = bStrict; }
	bool				IsStrict() { return m_bStrictParsing; }
	bool				CanContinue() { return m_eWorstErrorEncountered != eFatalError; }
	void				SetStrict(bool bStrict) { m_bStrictParsing = bStrict; }

// TODO: make private
    U32					m_u32Errors;
	U32					m_u32Warnings;
	bool				m_bStrictParsing;
	ErrorLevel			m_eWorstErrorEncountered;
};


#endif	// _DIAGNOSTICS_H_