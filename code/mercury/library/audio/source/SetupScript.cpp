#include "AudioPch.h"

#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif

using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptDefinition Load
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool SetupScriptDefinition::Load(const char* strFilename)
{
	Audio::File file;
	if (! file.OpenRead(strFilename))
		return false;

	SetupScriptDefinitionHeader header;

	U32 u32FileSize = file.Size();

	file.Read(&header, sizeof(SetupScriptDefinitionHeader));

	m_u32NumInstructions = header.u32NumScriptLines;
	m_u32StringTableOffset = header.u32StringTableOffset;

	m_pData = new char[u32FileSize - sizeof(SetupScriptDefinitionHeader)];

	if (! m_pData)
		return false;

	file.Read(m_pData, u32FileSize - sizeof(SetupScriptDefinitionHeader));

	file.Close();

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScript constructor
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SetupScript::SetupScript(SetupScriptManager* pManager)
{
	m_u32CurrentScriptLine = 0;
	m_u32CurrentParameter = 0;
	m_pManager = pManager;
	m_bParsed = true;
	m_u32FirstLineFail = 0;
	m_u32NumErrors = 0;
	m_bSubmittedForParsing = false;
	m_s32ScriptFinished = 0;
	m_pDefinition = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScript desctructor
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SetupScript::~SetupScript()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScript::AddScriptLine
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
//bool SetupScript::AddScriptLine(SetupScriptCommand eCommand, ScriptLineDef& scriptLine)
//{
//	if (m_u32CurrentScriptLine == MAX_NUM_SCRIPT_LINES)
//		return false;
//
//	m_aLine[m_u32CurrentScriptLine].m_eCommand = eCommand;
//
//	if (scriptLine.strParam1)
//		m_aLine[m_u32CurrentScriptLine].m_Param1String = m_pManager->AllocateString(scriptLine.strParam1);
//
//	if (scriptLine.strParam2)
//		m_aLine[m_u32CurrentScriptLine].m_Param2String = m_pManager->AllocateString(scriptLine.strParam2);
//
//	m_aLine[m_u32CurrentScriptLine].m_Param1Index = scriptLine.u16Param1Index;
//	m_aLine[m_u32CurrentScriptLine].m_Param2Index = scriptLine.u16Param2Index;
//
//	m_u32CurrentScriptLine++;
//
//	return true;
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScript::AddInputParameter
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
U16 SetupScript::AddInputParameter(EnvironmentVariableType eType, const void* const pData)
{
	if (m_u32CurrentParameter == MAX_NUM_SCRIPT_PARAMETERS)
		return false;

	U16 u16RetValue = m_u32CurrentParameter;

	m_aParameter[m_u32CurrentParameter].m_pData = (void*)pData;
	m_aParameter[m_u32CurrentParameter].m_eType = eType;

	m_u32CurrentParameter++;

	return u16RetValue;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScript::Parse
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool SetupScript::Parse(ObjectManager* pObjectManager)
{
	for (U32 i = 0; i < m_u32CurrentScriptLine; i++)
	{
		bool bParsed = true;
		Object* pObjFrom = 0;
		Object* pObjTo = 0;

		if ((U32)m_aLine[i].m_eCommand >= eConnectMono &&
		(U32)m_aLine[i].m_eCommand <= eConnectEvent)
		{
			if (m_aParameter[m_aLine[i].m_Param1Index].m_eType != eInstance
				|| m_aParameter[m_aLine[i].m_Param2Index].m_eType != eInstance)
				bParsed = false;
			else
			{
				if (! (m_aParameter[m_aLine[i].m_Param1Index].m_pData))
				{
					// TODO: error- instance from is NULL
					return false;
				}
				else if (! (m_aParameter[m_aLine[i].m_Param2Index].m_pData))
				{
					// TODO: error- instance to is NULL
					return false;
				}
				pObjFrom = ((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData))->GetObject();
				pObjTo = ((Instance*)(m_aParameter[m_aLine[i].m_Param2Index].m_pData))->GetObject();
			}
		}
/* jkc
		if (bParsed)
		{
			switch(m_aLine[i].m_eCommand)
			{
				case eConnectMono:
				{
					pObjectManager->ReceiveConnect((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData),
														pObjFrom->GetMonoAudioOutput(m_pDefinition->GetString(m_aLine[i].m_Param1StringOffset)),
														(Instance*)(m_aParameter[m_aLine[i].m_Param2Index].m_pData),
														pObjTo->GetMonoAudioInput(m_pDefinition->GetString(m_aLine[i].m_Param2StringOffset)));
				}
				break;

				case eConnectStereo:
				{
					pObjectManager->ReceiveConnect((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData),
														pObjFrom->GetStereoAudioOutput(m_pDefinition->GetString(m_aLine[i].m_Param1StringOffset)),
														(Instance*)(m_aParameter[m_aLine[i].m_Param2Index].m_pData),
														pObjTo->GetStereoAudioInput(m_pDefinition->GetString(m_aLine[i].m_Param2StringOffset)));
				}
				break;

				case eConnectSurround:
				{
					pObjectManager->ReceiveConnect((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData),
														pObjFrom->GetSurroundAudioOutput(m_pDefinition->GetString(m_aLine[i].m_Param1StringOffset)),
														(Instance*)(m_aParameter[m_aLine[i].m_Param2Index].m_pData),
														pObjTo->GetSurroundAudioInput(m_pDefinition->GetString(m_aLine[i].m_Param2StringOffset)));
				}
				break;

				case eConnectSurroundLFE:
				{
					pObjectManager->ReceiveConnect((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData),
														pObjFrom->GetSurroundLFEAudioOutput(m_pDefinition->GetString(m_aLine[i].m_Param1StringOffset)),
														(Instance*)(m_aParameter[m_aLine[i].m_Param2Index].m_pData),
														pObjTo->GetSurroundLFEAudioInput(m_pDefinition->GetString(m_aLine[i].m_Param2StringOffset)));
				}
				break;

				case eConnectEvent:
				{
					pObjectManager->ReceiveConnect((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData),
														pObjFrom->GetEventOutput(m_pDefinition->GetString(m_aLine[i].m_Param1StringOffset)),
														(Instance*)(m_aParameter[m_aLine[i].m_Param2Index].m_pData),
														pObjTo->GetEventInput(m_pDefinition->GetString(m_aLine[i].m_Param2StringOffset)));
				}
				break;

				case eConnectControl:
				{
					pObjectManager->ReceiveConnect((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData),
														pObjFrom->GetControlOutput(m_pDefinition->GetString(m_aLine[i].m_Param1StringOffset)),
														(Instance*)(m_aParameter[m_aLine[i].m_Param2Index].m_pData),
														pObjTo->GetControlInput(m_pDefinition->GetString(m_aLine[i].m_Param2StringOffset)));
				}
				break;

				case eTriggerActionIn:
				{
					Object* pObj = ((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData))->GetObject();

					if (! pObj)
					{
						// TODO: error
						return false;
					}

					ActionIn* pActionIn = pObj->GetActionIn(m_pDefinition->GetString(m_aLine[i].m_Param1StringOffset));

					if (! pActionIn)
					{
						// TODO: error- ActionIn not found
						return false;
					}

					if (! pActionIn->TriggerFromScript((Instance*)(m_aParameter[m_aLine[i].m_Param1Index].m_pData)))
					{
						// TODO: error- specified component was not an event
						return false;
					}
				}

				default:
				break;
			}
		}
*/
		if (! bParsed)
		{
			if (m_bParsed)
			{
				m_bParsed = false;
				m_u32FirstLineFail = i;
			}
			m_u32NumErrors++;
		}
	}

	m_Lock.Increment(&m_s32ScriptFinished, 1);

	m_pDefinition->ScriptFinished();

	return m_bParsed;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScript SetupFromDefinition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool SetupScript::SetupFromDefinition(SetupScriptDefinition* pDef)
{
	m_pDefinition = pDef;

	char* pData = pDef->m_pData;

	for (U32 i = 0; i < pDef->m_u32NumInstructions; i++)
	{
		if (m_u32CurrentScriptLine == MAX_NUM_SCRIPT_LINES)
			return false;

		SetupScriptCommand eCommand = (SetupScriptCommand)(*((U32*)pData));

		m_aLine[m_u32CurrentScriptLine].m_eCommand = eCommand;
		pData += sizeof(U32);

		m_aLine[m_u32CurrentScriptLine].m_Param1Index = *((U16*)pData);
		pData += sizeof(U32);

		if ((eCommand >= eConnectMono && eCommand <= eConnectEvent) || eCommand == eTriggerActionIn)
		{
			m_aLine[m_u32CurrentScriptLine].m_Param1StringOffset = *((U32*)pData);
			pData += sizeof(U32);
		}

		if (eCommand >= eConnectMono && eCommand <= eConnectEvent)
		{
			m_aLine[m_u32CurrentScriptLine].m_Param2Index = *((U16*)pData);
			pData += sizeof(U32);
			
			m_aLine[m_u32CurrentScriptLine].m_Param2StringOffset = *((U32*)pData);
			pData += sizeof(U32);
		}

		m_u32CurrentScriptLine++;		
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager LoadDefinition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SetupScriptDefHandle SetupScriptManager::LoadDefinition(const char* const strFilename)
{
	SetupScriptDefinition* pDef = new SetupScriptDefinition();

	if (! pDef)
		return Audio::INVALID_HANDLE;

	if (! pDef->Load(strFilename))
	{
		delete pDef;
		return Audio::INVALID_HANDLE;
	}

	return m_vDefinition.AddEntry(pDef);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager constructor
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SetupScriptManager::SetupScriptManager()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager destructor
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SetupScriptManager::~SetupScriptManager()
{
	for (U32 i = 0; i < m_vScript.GetHighestNodeIndex(); i++)
	{
		if (m_vScript[i])
		{
			delete m_vScript[i];
			m_vScript.FreeByIndex(i);
		}
	}

	for (U32 i = 0; i < m_vDefinition.GetHighestNodeIndex(); i++)
	{
		if (m_vDefinition[i])
		{
			delete m_vDefinition[i];
			m_vDefinition.FreeByIndex(i);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager::CreateManualScript
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SetupScriptHandle SetupScriptManager::CreateManualScript()
{
	SetupScript* pNewScript = new SetupScript(this);

	if (! pNewScript)
		return Audio::INVALID_HANDLE;

	return m_vScript.AddEntry(pNewScript);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager::CreateManualScriptDefinition
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SetupScriptDefHandle SetupScriptManager::CreateManualScriptDefinition()
{
	SetupScriptDefinition* pNewDef = new SetupScriptDefinition();

	if (! pNewDef)
		return Audio::INVALID_HANDLE;

	return m_vDefinition.AddEntry(pNewDef);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager::CreateScript
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SetupScriptHandle SetupScriptManager::CreateScript(SetupScriptDefHandle hDef)
{
	SetupScriptDefinition* pDef = m_vDefinition.GetByHandle(hDef);

	if (! pDef)
		return Audio::INVALID_HANDLE;

	SetupScriptHandle hScript = CreateManualScript();

	if (hScript == Audio::INVALID_HANDLE)
		return hScript;

	m_vScript.GetByHandle(hScript)->SetupFromDefinition(pDef);

	return hScript;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager::AddScriptLine
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
//bool SetupScriptManager::AddScriptLine(SetupScriptHandle hScript, SetupScriptCommand eCommand, ScriptLineDef& scriptLine)
//{
//	SetupScript* pScript = m_vScript.GetByHandle(hScript);
//
//	if (! pScript)
//		return false;
//
//	return pScript->AddScriptLine(eCommand, scriptLine);
//}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager::AddInputParameter
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
U16 SetupScriptManager::AddInputParameter(SetupScriptHandle hScript, EnvironmentVariableType eType, const void* const pData)
{
	SetupScript* pScript = m_vScript.GetByHandle(hScript);

	if (! pScript)
		return false;

	return pScript->AddInputParameter(eType, pData);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager::SubmitScript
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool SetupScriptManager::SubmitScript(SetupScriptHandle hScript)
{
	if (! m_vScript.GetByHandle(hScript))
		return false;

	if (m_vScript.GetByHandle(hScript)->HasBeenSubmitted())
		return false;

	m_vScript.GetByHandle(hScript)->Submit();

	return System::Get()->GetObjectManager()->RunSetupScript(hScript);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SetupScriptManager::Parse
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool SetupScriptManager::Parse(SetupScriptHandle hScript)
{
	return m_vScript.GetByHandle(hScript)->Parse(System::Get()->GetObjectManager());
}


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
