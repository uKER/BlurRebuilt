#include "AudioPch.h"


char* strErrorLevelStrings[] =
{
	"",
	"WARNING",
	"ERROR",
	"FATAL ERROR"
};


bool ReceiveParserError(unsigned int u32LineNum, char* strError, char* strFile)
{
	ErrorData errorData;

	errorData.m_strErrorString = strError;
	errorData.m_strFilename = strFile;
	errorData.m_u32LineNumber = u32LineNum;
	errorData.m_eErrorLevel = eFatalError;

	return AudioDataFile::GetDiagnostics().ReceiveError(&errorData);
}


bool ReceiveScriptError(U32 u32LineNum, char* strError, char* strFile, char* strScript)
{
	ErrorData errorData;

	errorData.m_strErrorString = strError;
	errorData.m_strFilename = strFile;
	errorData.m_strScriptName = strScript;
	errorData.m_u32LineNumber = u32LineNum;
	errorData.m_eErrorLevel = eFatalError;

	return AudioDataFile::GetDiagnostics().ReceiveError(&errorData);
}


Diagnostics::Diagnostics()
{
	m_bStrictParsing = false;
	m_u32Errors = 0;
	m_u32Warnings = 0;
	m_eWorstErrorEncountered = eSuccess;
}


bool Diagnostics::ReceiveError(ErrorData* pErrorData)
{
	if (pErrorData->m_eErrorLevel == eWarning)
		m_u32Warnings++;
	else
		m_u32Errors++;

	if ((U32)(pErrorData->m_eErrorLevel) > (U32)m_eWorstErrorEncountered)
		m_eWorstErrorEncountered = pErrorData->m_eErrorLevel;

	if (m_eWorstErrorEncountered == eWarning)
		system("color e");

	printf("%s: %s at line %d\n", strErrorLevelStrings[U32(pErrorData->m_eErrorLevel)], pErrorData->m_strErrorString, pErrorData->m_u32LineNumber);

	if (pErrorData->m_strScriptName)
		printf("\toccurred in script %s\n", pErrorData->m_strScriptName);

	if (pErrorData->m_strFilename)
		printf("\toccurred in file '%s'\n", pErrorData->m_strFilename);

	if (pErrorData->m_eErrorLevel != eWarning)
	{
		system("color c");

		if (m_bStrictParsing)
			system("pause 0");
	}

	return (pErrorData->m_eErrorLevel != eFatalError);
}


void Diagnostics::FinalReport(bool bAlwaysPause)
{
	bool bFailed = false;

	if (m_eWorstErrorEncountered == eWarning && m_bStrictParsing)
	{
		printf("\nParsing failed (no errors but encountered warnings and strict mode is set)\n");
		bFailed = true;
	}
	else if (m_eWorstErrorEncountered == eSuccess || m_eWorstErrorEncountered == eWarning)
		printf("\nParsing successful.\n");
	else
	{
		printf("\nParsing failed.\n");
		bFailed = true;
	}

	printf("Encountered %d errors, %d warnings.\n", m_u32Errors, m_u32Warnings);

	if (bAlwaysPause || bFailed)
		system("pause 0");
}