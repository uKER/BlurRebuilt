#include "stdafx.h"


using namespace Audio;


ControlScriptParser::ControlScriptParser()
{
	m_pFirstScriptLine = 0;
	m_pLastScriptLine = 0;
	m_u32NumParameters = 0;
	m_pFirstString = 0;
	m_pLastString = 0;
	m_u32StringTableSize = 0;
	m_u32InstructionNumber = 0;
	m_pFileIn = 0;
	m_u32NumErrors = 0;
	m_u32NumFunctionTableEntries = 0;
	Close();
}


ControlScriptParser::~ControlScriptParser()
{
	if (m_pFirstScriptLine)
	{
		delete m_pFirstScriptLine;
		m_pFirstScriptLine = 0;
	}

	if (m_pFirstString)
	{
		delete m_pFirstString;
		m_pFirstString = 0;
	}
}


bool ControlScriptParser::Parse()
{
	ControlScriptDefinitionInstruction instruction;
	U32 u32LineNumber = 1;
	FunctionPosition eFunctionPosition = eOutsideFunction;
	FunctionPosition ePrevFunctionPosition = eOutsideFunction;

	for (ControlScriptResult eRes = ParseLine(instruction, eFunctionPosition, &u32LineNumber); eRes != eNothingToParse; eRes = ParseLine(instruction, eFunctionPosition, &u32LineNumber))
	{
		if (eRes == eSuccess)
		{
			if (ePrevFunctionPosition == eInsideFunction && eFunctionPosition == eOutsideFunction)
			{
				EndCurrentFunction(GetCurrentInstructionIndex());
			}
			else if (instruction.eCommand == eFunction)
			{
				AddFunctionTableEntry(GetCurrentInstructionIndex(), AddString(instruction.strOperand1Name));
			}
			else
			{
				AddNewInstruction();


				switch (instruction.eCommand)
				{
					case eConnectMono:
					case eConnectStereo:
					case eConnectSurround:
					case eConnectSurroundLFE:
					case eConnectControl:
					case eConnectEvent:
					{
						ConnectInstruction parsedInst;

						if (! CheckStringExists(instruction.strOperand1Name))
						{
							DoError(eUnrecognisedParameter, u32LineNumber, instruction.strOperand1Name);
							return false;
						}
						else if (! CheckStringExists(instruction.strOperand2Name))
						{
							DoError(eUnrecognisedParameter, u32LineNumber, instruction.strOperand2Name);
							return false;
						}

						//if (! strlen(instruction.strOperand1String)
						//	|| ! strlen(instruction.strOperand2String))
						//{
						//	DoError(eMissingString, u32LineNumber);
						//	return false;
						//}

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32FromInstanceComponentIndex = AddString(instruction.strOperand1Name);
						parsedInst.m_s32ToInstanceComponentIndex = AddString(instruction.strOperand2Name);
						//parsedInst.m_s32FromInstanceStringIndex = AddString(instruction.strOperand1Name);
						//parsedInst.m_s32FromComponentNameStringIndex = AddString(instruction.strOperand1String);
						//parsedInst.m_s32ToInstanceStringIndex = AddString(instruction.strOperand2Name);
						//parsedInst.m_s32ToComponentNameStringIndex = AddString(instruction.strOperand2String);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(ConnectInstruction));
					}
					break;

					case eSpawnInstance:
					{
						SpawnInstanceInstruction parsedInst;

						if (! CheckStringExists(instruction.strOperand2Name))
						{
							DoError(eUnrecognisedParameter, u32LineNumber, instruction.strOperand2Name);
							return false;
						}

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32ObjectNameStringIndex = AddString(instruction.strOperand2Name);
						parsedInst.m_s32InstanceNameStringIndex = AddString(instruction.strOperand1Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(SpawnInstanceInstruction));
					}
					break;

					case eDestroyInstance:
					{
						DestroyInstanceInstruction parsedInst;

						if (! CheckStringExists(instruction.strOperand1Name))
						{
							DoError(eUnrecognisedParameter, u32LineNumber, instruction.strOperand1Name);
							return false;
						}

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32InstanceNameStringIndex = AddString(instruction.strOperand1Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(DestroyInstanceInstruction));
					}
					break;

					case eLoadObject:
					{
						LoadObjectInstruction parsedInst;

						if (! strlen(instruction.strOperand2Name))
						{
							DoError(eMissingString, u32LineNumber);
							return false;
						}

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32ObjectNameStringIndex = AddString(instruction.strOperand1Name);
						parsedInst.m_s32ObjectPathStringIndex = AddString(instruction.strOperand2Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(LoadObjectInstruction));
					}
					break;

					case eUnloadObject:
					{
						UnloadObjectInstruction parsedInst;

						if (! CheckStringExists(instruction.strOperand1Name))
						{
							DoError(eUnrecognisedParameter, u32LineNumber, instruction.strOperand1Name);
							return false;
						}

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32ObjectNameStringIndex = AddString(instruction.strOperand1Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(UnloadObjectInstruction));
					}
					break;

					case eTriggerActionIn:
					{
						TriggerActionInInstruction parsedInst;

						if (! CheckStringExists(instruction.strOperand1Name))
						{
							DoError(eUnrecognisedParameter, u32LineNumber, instruction.strOperand1Name);
							return false;
						}

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32InstanceComponentIndex = AddString(instruction.strOperand1Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(TriggerActionInInstruction));
					}
					break;

					case eInstanceIn:
					{
						InstanceInInstruction parsedInst;

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32InstanceNameStringIndex = AddString(instruction.strOperand1Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(InstanceInInstruction));
					}
					break;

					case eObjectIn:
					{
						ObjectInInstruction parsedInst;

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32ObjectNameStringIndex = AddString(instruction.strOperand1Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(ObjectInInstruction));
					}
					break;

					case eGetObjectComponent:
					{
						GetObjectComponent parsedInst;

						if (! CheckStringExists(instruction.strOperand2Name))
						{
							DoError(eUnrecognisedParameter, u32LineNumber, instruction.strOperand2Name);
							return false;
						}

						if (! strlen(instruction.strOperand2String))
						{
							DoError(eMissingString, u32LineNumber);
							return false;
						}

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32ObjectComponentNameStringIndex = AddString(instruction.strOperand1Name);
						parsedInst.m_s32ObjectNameStringIndex = AddString(instruction.strOperand2Name);
						parsedInst.m_s32ComponentNameStringIndex = AddString(instruction.strOperand2String);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(GetObjectComponent));
					}
					break;

					case eObjectComponentIn:
					{
						ObjectComponentIn parsedInst;

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32ObjectComponentNameStringIndex = AddString(instruction.strOperand1Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(ObjectComponentIn));
					}
					break;

					case eInstanceComponentIn:
					{
						InstanceComponentIn parsedInst;

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32InstanceComponentNameStringIndex = AddString(instruction.strOperand1Name);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(InstanceComponentIn));
					}
					break;

					case eGetInstanceComponent:
					{
						GetInstanceComponent parsedInst;

						parsedInst.eCommand = instruction.eCommand;
						parsedInst.m_s32InstanceComponentNameStringIndex = AddString(instruction.strOperand1Name);
						parsedInst.m_s32InstanceNameStringIndex = AddString(instruction.strOperand2Name);
						parsedInst.m_s32ObjectComponentNameStringIndex = AddString(instruction.strOperand2String);

						m_pLastScriptLine->SetData(&parsedInst, sizeof(GetInstanceComponent));
					}
					break;

					default:
					{
						DoError(eUnknownError, u32LineNumber);
						return false;
					}
				}
			}
		}
		else
		{
			DoError(eRes, u32LineNumber);
			return false;
		}

		ePrevFunctionPosition = eFunctionPosition;
	}

	return true;
}


bool ControlScriptParser::Output(char* strOutputFilename, CommandLineApp* app)
{
	FILE* fOut = fopen(strOutputFilename, "wb");

	if (! fOut)
		return false;

	ControlScriptDefinitionHeader header;

	U32 u32NumInstructions = 0;
	U32 u32ScriptSize = 0;

	for (ControlScriptParsedInstruction* pInst = m_pFirstScriptLine; pInst; pInst = pInst->GetNext())
	{
		u32NumInstructions++;
		u32ScriptSize += pInst->GetSize();
	}

	header.u32Id = 1;
	header.u32VersionMajor = CONTROL_SCRIPT_VERSION_MAJOR;
	header.u32VersionMinor = CONTROL_SCRIPT_VERSION_MINOR;
	header.u32NumScriptLines = u32NumInstructions;
	header.u32StringTableOffset = u32ScriptSize + (m_u32NumFunctionTableEntries * sizeof(ControlScriptFunction));
	header.u32NumFunctionTableEntries = m_u32NumFunctionTableEntries;

	app->WriteBytes(&header, sizeof(ControlScriptDefinitionHeader), fOut);

	app->WriteBytes(m_aFunctions, m_u32NumFunctionTableEntries * sizeof(ControlScriptFunction), fOut);

	for (ControlScriptParsedInstruction* pInst = m_pFirstScriptLine; pInst; pInst = pInst->GetNext())
	{
		app->WriteBytes(pInst->GetData(), pInst->GetSize(), fOut);
	}

	for (ControlScriptString* pString = m_pFirstString; pString; pString = pString->pNext)
	{
		fwrite(pString->str, strlen(pString->str) + 1, 1, fOut);
	}

	return Close();
}


bool ControlScriptParser::AddNewInstruction()
{
	m_u32InstructionNumber++;

	// if previous instruction has not had a command specified, just re-use that instruction,
	//	as there must be multiple newline chars between script lines

	if (m_pLastScriptLine && (! m_pLastScriptLine->IsDataSet()))
		return true;

	ControlScriptParsedInstruction* pNewLine = new ControlScriptParsedInstruction();

	if (m_pLastScriptLine)
	{
		m_pLastScriptLine->SetNext(pNewLine);
		m_pLastScriptLine = pNewLine;
	}
	else
	{
		m_pFirstScriptLine = pNewLine;
		m_pLastScriptLine = pNewLine;
	}

	return true;
}


bool ControlScriptParser::IsInstructionSet()
{
	if (! m_pLastScriptLine)
		return false;

	return (m_pLastScriptLine->IsDataSet());
}


bool ControlScriptParser::CheckStringExists(char* str)
{
	for (ControlScriptString* pString = m_pFirstString; pString; pString = pString->pNext)
	{
		if (strcmp(pString->str, str) == 0)
			return true;
	}

	return false;
}


bool ControlScriptParser::AddFunctionTableEntry(U32 u32StartInstruction, U32 u32FunctionNameStringIndex)
{
	if (m_u32NumFunctionTableEntries == MAX_NUM_FUNCTIONS)
	{
		DoError(eTooManyFunctionsDeclared, 0);
		return false;
	}

	m_aFunctions[m_u32NumFunctionTableEntries].u32StartInstruction = u32StartInstruction;
	m_aFunctions[m_u32NumFunctionTableEntries].u32NameStringIndex = u32FunctionNameStringIndex;

	return true;
}


bool ControlScriptParser::EndCurrentFunction(U32 u32EndInstruction)
{
	m_aFunctions[m_u32NumFunctionTableEntries].u32EndInstruction = u32EndInstruction;

	m_u32NumFunctionTableEntries++;

	return true;
}


U32 ControlScriptParser::AddString(char* str)
{
	for (ControlScriptString* pString = m_pFirstString; pString; pString = pString->pNext)
	{
		if (strcmp(pString->str, str) == 0)
			return pString->u32Offset;
	}

	ControlScriptString* pNewString = new ControlScriptString();

	pNewString->SetString(str);
	pNewString->u32Offset = m_u32StringTableSize;

	m_u32StringTableSize += (U32)(strlen(str) + 1);

	if (! m_pFirstString)
	{
		m_pFirstString = pNewString;
		m_pLastString = pNewString;
	}
	else
	{
		m_pLastString->pNext = pNewString;
		m_pLastString = pNewString;
	}

	return pNewString->u32Offset;
}


void ControlScriptParser::DoError(ControlScriptResult eResult, U32 u32LineNumber, char* strInfo)
{
	system("color c");
	char strError[128] = {0};

	switch (eResult)
	{
		case eInstructionNotBeingBuilt:
			sprintf(strError, "Instruction not being built");
			break;

		case eMultipleKeywordsInInstruction:
			sprintf(strError, "Multiple keywords in instruction");
			break;

		case eTooManyParameters:
			sprintf(strError, "Too many parameters");
			break;

		case eInvalidSyntax:
			sprintf(strError, "Invalid syntax");
			break;

		case eNoKeywordAtStartOfInstruction:
			sprintf(strError, "No keyword at start of instruction");
			break;

		case eUnrecognisedParameter:
			if (strInfo && strlen(strInfo))
				sprintf(strError, "Unrecognised parameter- %s", strInfo);
			else
				sprintf(strError, "Unrecognised parameter");
			break;

		case eStringTooLong:
			sprintf(strError, "String too long");
			break;

		case eBracesNotProperlyMatched:
			sprintf(strError, "Braces not properly matched");
			break;

		case eMissingString:
			sprintf(strError, "Missing string");
			break;

		case eFunctionSyntaxError:
			sprintf(strError, "Function syntax error");
			break;

		case eTooManyFunctionsDeclared:
			sprintf(strError, "Too many functions declared");
			break;

		default:
		case eUnknownError:
			sprintf(strError, "Unknown error");
			break;

	}
	printf("Error:\n%s\nOccurred at line %d\n\n", strError, u32LineNumber);
	m_u32NumErrors++;
}
