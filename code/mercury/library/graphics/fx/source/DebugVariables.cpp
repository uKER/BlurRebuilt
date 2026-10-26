#include "stdafx.h"

#include <system/DebugLog.h>
#include <system/Debug/Variables.h>

#include "fx/Fx.h"
#include "fx/Effect.h"
#include "fx/DebugVariables.h"

#define FX_DBGVAR_NAME "Graphics.Fx"

#if !CFG_GOLD

namespace FX
{

#if CFG_SPEED
Debug::Bool g_bEnableFXDebugVariables("Graphics.FX.DebugVariables", false);
#else
Debug::Bool g_bEnableFXDebugVariables("Graphics.FX.DebugVariables", true);
#endif

bool _g_bEnableFXDebugVariables = false;

/// find and extra the min-max range from a struct member's annotations
/// from fx/EnvironmentSettings.h
bool AnnotationsGetRange(const Btl::Vector<Annotation> &annotations, float *pfMin, float *pfMax);

namespace DebugVariables
{

Manager g_Manager;

/// returns the parameter type
E_VarType Group::GetParameterType(const FX::Parameter *p, Util::C_String *psOutName)
{
	bool bParam = false;
	E_VarType type = E_VarType_Colour;
	if (p->Size > 4)
		type = E_VarType_Array;

	if (p->Type != NGLFX_PARAM_VECTOR)
		return E_VarType_None;

	for (int i=0; i<p->Annotations.Size(); i++)
	{
		const Annotation *pA = &p->Annotations[i];

		if (pA->Type == NGLFX_ANNOTATION_STRING)
		{
			if (strcmp(pA->Name, "Source") == 0 || strcmp(pA->Name, "DebugVariable") == 0)
			{
				*psOutName = pA->String;
				bParam = true;
			}
			else if (strcmp(pA->Name, "Type") == 0)
			{
				if (!stricmp(pA->String, "Float"))
					type = E_VarType_Float;
				else if (!stricmp(pA->String, "Float2"))
					type = E_VarType_Float2;
				else if (!stricmp(pA->String, "Float3"))
					type = E_VarType_Float3;
			}
		}
	}

	return bParam?type:E_VarType_None;
}

/// add tweakable parameter to this group
void Group::AddParameter(const Effect *pEffect, E_VarType type, const char *pszName, const FX::Parameter *pParam)
{
	// check for duplicates
	for (int i=0; i<m_Parameters.Size(); i++)
	{
		if (strcmp(m_Parameters[i].sName, pszName) == 0)
			return;
	}
	if (!Debug::IsDebugVariableNameValid(pszName))
		return;

	float fMin, fMax;
	bool bRange = AnnotationsGetRange(pParam->Annotations, &fMin, &fMax);

	// create new parameter
	m_Parameters.PushBack();
	Parameter &item = m_Parameters.Back();
	item.sName = pszName;
	item.type = type;
	item.sFXParamName = pParam->Name;
	item.psDbgVarName = g_Manager.NewString();
	item.psDbgVarName->Printf("%s.%s.%s", FX_DBGVAR_NAME, pEffect->Name.Chars(), pszName);
	float *pfData = (float*)pParam->Data;
	switch (item.type)
	{
	case E_VarType_Float:
		if (!bRange)
			item.pDbgVar = new Debug::Float(item.psDbgVarName->Chars(), pfData[0]);
		else
			item.pDbgVar = new Debug::Float(item.psDbgVarName->Chars(), pfData[0], Debug::Range(fMin, fMax));
		break;
	case E_VarType_Float2:
	case E_VarType_Float3:
		item.pDbgVar = new Debug::Vector(item.psDbgVarName->Chars(), pfData[0], pfData[1], pfData[2]);
		break;
	case E_VarType_Colour:
		item.pDbgVar = new Debug::Colour(item.psDbgVarName->Chars(), pfData[0], pfData[1], pfData[2], pfData[3]);
		break;
	case E_VarType_Array:
		item.dbgVarVector.Resize(pParam->Size / 4);
		for (S32 i = 0; i < item.dbgVarVector.Size(); i++)
		{
			Util::C_String name;
			name.Printf("%s%i", item.psDbgVarName->Chars(), i); 
			item.dbgVarVector[i] = new Debug::Colour(name.Chars(), pfData[i*4 + 0], pfData[i*4 + 1], pfData[i*4 + 2], pfData[i*4 + 3], Debug::Dynamic() );
		}
		break;
	default:
		assert(0);
		break;
	}
}

/// add the parameters of the given effect into the group
void Group::AddEffect(const Effect *pEffect)
{
	bool bParameterAdded = false;

	for (int i=0; i<pEffect->Parameters.Size(); i++)
	{
		const FX::Parameter *pParam = &pEffect->Parameters[i];
		Util::C_String sVarName;
		E_VarType type = GetParameterType(pParam, &sVarName);
		if (type == E_VarType_None)
			continue;
		if (pParam->bShared)
			continue; // not supporting shared parameters at the moment

		AddParameter(pEffect, type, sVarName, pParam);
		bParameterAdded = true;
	}

	if (bParameterAdded)
		m_Effects.PushBack(pEffect);
}

/// remove the given effect from this group
void Group::ReleaseEffect(const Effect *pEffect)
{
	for (int i=0; i<m_Effects.Size(); i++)
	{
		if (m_Effects[i] == pEffect)
		{
			m_Effects.EraseByIndexNonOrdered(i);
			return;
		}
	}
}

/// update variables in the effects with the overrides from debugvariables
void Group::Update()
{
#ifndef CFG_GOLD
	U8 iOverride = m_pDbgVar->Value()?1:0;
#endif

	for (int iEffect=0; iEffect<m_Effects.Size(); iEffect++)
	{
		const Effect *pEffect = m_Effects[iEffect];

		for (int iParam=0; iParam<m_Parameters.Size(); iParam++)
		{
			Parameter *pParam = &m_Parameters[iParam];
			FX::Parameter *pFXParam = pEffect->GetParameter(pParam->sFXParamName);
			switch (pParam->type)
			{
			case E_VarType_Float:
				{
					float f = *((Debug::Float*)pParam->pDbgVar);
					FX::SetParameter(pFXParam, &f);
				}
				break;
			case E_VarType_Float2:
			case E_VarType_Float3:
				{
					Debug::VectorStruct3 vec = *((Debug::Vector*)pParam->pDbgVar);
					FX::SetParameter(pFXParam, &vec);
				}
				break;
			case E_VarType_Colour:
				{
					Debug::ColourStruct col = *((Debug::Colour*)pParam->pDbgVar);
					FX::SetParameter(pFXParam, &col);
				}
				break;
			case E_VarType_Array:
				{
					float* pData = (float*)alloca(pParam->dbgVarVector.Size() * 4 * 4);
					for (S32 i = 0; i < pParam->dbgVarVector.Size();i++)
					{
						Debug::ColourStruct vec = *((Debug::Colour*)pParam->dbgVarVector[i]);
						memcpy(&pData[i*4], &vec, 4 * 4);
					}
					FX::SetParameter(pFXParam, pData);
				}
				break;
			default:
				assert(0);
				break;
			}
#ifndef CFG_GOLD
			pFXParam->bOverride = iOverride;
#endif
		}
	}
}

/// destructor
Manager::~Manager()
{
	for (int i=0; i<m_Strings.Size(); i++)
		delete m_Strings[i];
}

/// update variables in the effects with the overrides from debugvariables. to be called once per frame
void Manager::Update()
{
	_g_bEnableFXDebugVariables = g_bEnableFXDebugVariables;

	if (!g_bEnableFXDebugVariables)
		return;

	for (int i=0; i<m_Groups.Size(); i++)
	{
		m_Groups[i].Update();
	}
}

/// add the parametes for this effect into the debug variable system
void Manager::AddEffect(const Effect *pEffect)
{
	// get group name from effect
	Group *pGroup = GetGroup(pEffect->Name);

	// add effect to group
	pGroup->AddEffect(pEffect);
}

/// called by the effects system when an effect is freed
void Manager::ReleaseEffect(const Effect *pEffect)
{
	int idx = FindGroup(pEffect->Name.Chars());
	//assert(idx != -1);	// shader was never added?

	if (idx != -1)
	{
		m_Groups[idx].ReleaseEffect(pEffect);
	}
}

/// return a new string
Util::C_String *Manager::NewString()
{
	Util::C_String *p = new Util::C_String;
	m_Strings.PushBack(p);
	return p;
}

/// return a pointer to the group with the given name
/// create a new group if necessary
Group *Manager::GetGroup(const char *pszName)
{
	int idx = FindGroup(pszName);
	if (idx != -1)
		return &m_Groups[idx];

	// create new
	m_Groups.PushBack();
	Group &item = m_Groups.Back();
	item.m_sName = pszName;
	item.m_psDbgVarName = NewString();
	item.m_psDbgVarName->Printf("%s.%s.Override", FX_DBGVAR_NAME, pszName);
	item.m_pDbgVar = new Debug::Bool(item.m_psDbgVarName->Chars(), false);
	return &item;
}

/// return the index of the group with the given name
int Manager::FindGroup(const char *pszName)
{
	for (int i=0; i<m_Groups.Size(); i++)
	{
		if (strcmp(m_Groups[i].m_sName, pszName) == 0)
			return i;
	}

	return -1;
}

}

}

#endif
