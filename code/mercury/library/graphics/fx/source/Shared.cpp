#include "stdafx.h"

#include "fx/Effect.h"
#include "fx/Shared.h"
#include "fx/System.h"
#include "fx/Fx.h"

#include <algorithm>
#include <system/debug/Variables.h>
#include <Display/Device/DeviceTypesEnums.h>
#include "fx/SamplerStates.h"
#include "fx/Manager.h"
#include "system/time/Time.h"
#include "fx/ReloadableEffect.h"
#include "fx/ShaderImpl.h"

namespace FX
{

void DumpGlobalBindings(void*) { g_SharedEffect.DumpGlobalBindings(); }
Debug::Function g_DumpGlobalBindings("Graphics.Fx.DumpGlobalBindings", DumpGlobalBindings, 0);

SharedEffect ALIGN16 g_SharedEffect;

/// find a shared parameter and return its index
int SharedEffect::GetSharedParameterIdxByName(const char *Name)
{
	// find the shared parameter and return it
	for (int i = 0; i < Parameters.Size(); i++)
		if (strcmp(Parameters[i].Name, Name) == 0)
			return i;

	// create a stub shared variable
	Parameters.PushBack();
	memset(&Parameters.Back(), 0, sizeof(Parameter));
	Parameters.Back().Name = SharedString::GetString(Name);
	Parameters.Back().bShared = 1;

	return Parameters.Size()-1;
}

/// find a shared parameter and return a pointer to it
Parameter* SharedEffect::GetSharedParameterByName(const char *Name)
{
	// find the shared parameter and return it
	for (int i = 0; i < Parameters.Size(); i++)
		if (strcmp(Parameters[i].Name, Name) == 0)
			return &Parameters[i];

	return NULL;
}

/// get shared parameter default value off effect
static Parameter *GetDefaultParameter(Effect *pEffect, const char *pszName)
{
	if (pEffect)
	{
		for (int i = 0; i < pEffect->Parameters.Size(); i++)
		if (strcmp(pEffect->Parameters[i].Name, pszName) == 0)
		{
			return &pEffect->Parameters[i];
		}
	}
	return GetParameterPtr(GetSharedParameter(pszName));
}

/// get nuber of registered shared parameters
int SharedEffect::GetNumSharedParameters()
{
	return Parameters.Size();
}

/// constructor
SharedEffect::SharedEffect()
{
	for (int x=0; x<MAX_SPGS*MAX_SPGS*E_SPGResource_Max; x++)
		m_SPGTable[0][0][x] = 255;

	m_bUpdateSharedBindingBitfields = false;
}

/// add new shared parameter
Parameter *SharedEffect::AddSharedParameter(Parameter *param)
{
	Parameters.PushBack();
	Parameter *pParam = &Parameters.Back();
	InitialiseSharedParameter(pParam, param);
	return pParam;
}

/// initialise new shared parameter
void SharedEffect::InitialiseSharedParameter(Parameter *pParam, Parameter *pOriginalParameter)
{
	pParam->Name = pOriginalParameter->Name;
	pParam->Type = pOriginalParameter->Type;
	pParam->Size = pOriginalParameter->Size;
	pParam->bShared = true;

	int c = pParam->Size;
	if (pOriginalParameter->Type == NGLFX_PARAM_SAMPLER)
	{
		pOriginalParameter ->Sampler->Texture = Core::PointerToSlot32(g_System.GetDefaultTexture());	// [Blur2PC][x64]
		c = 2;
	}

	// add more space for this parameter
	U32 *pOrgDataBase = NULL;
	int iNewDataOffset = ParameterData.Size();
	if (ParameterData.Size() > 0)
		pOrgDataBase = &ParameterData[0];
	ParameterData.Resize(ParameterData.Size()+c);

	// fixup data pointers
	int iOffset = 0;
	for (int i=0; i<Parameters.Size(); i++)
	{
		if (&Parameters[i] != pParam)
		{
			if (Parameters[i].Data != NULL)
			{
				int iOffset = (U32*)Parameters[i].Data - pOrgDataBase;
				Parameters[i].Data = &ParameterData[0] + iOffset;
			}
		}
	}
	pParam->Data = &ParameterData[0] + iNewDataOffset;

	// copy over the default data
	memcpy(pParam->Data, pOriginalParameter->Data, c*4);

	// backup the default data
	DefaultParameterData.Resize(ParameterData.Size());
	memcpy(&DefaultParameterData[0]+iNewDataOffset, pOriginalParameter->Data, c*4);

	if (pParam->Type == NGLFX_PARAM_SAMPLER)
		pParam->Size = 1;
}

/// restore default parameter data at the end of every frame
/// this is to stop pointers to deleted textures hanging around in the FX system
void SharedEffect::RestoreDefaultData()
{
	if (DefaultParameterData.Size() == 0)
		return;
	memcpy(&ParameterData[0], &DefaultParameterData[0], sizeof(U32)*DefaultParameterData.Size());
}

/// bind a shared parameter
Parameter *SharedEffect::BindSharedParameter(FunctionBinding* binding, Parameter* param)
{
	Parameter *pBindP = GetSharedParameterByName(param->Name);
	if (!pBindP)
		pBindP = AddSharedParameter(param);
	else if (pBindP->Type ==  NGLFX_PARAM_NONE)
		InitialiseSharedParameter(pBindP, param);

#if !PLATFORM_PS3
	assert(binding->Offset == 0);
#endif

	binding->Source = NGLFX_BIND_SHARED;
	binding->Offset = (U32)(INTPTR)((float*)((size_t)pBindP->Data - (size_t)&ParameterData[0]) + binding->Offset);	// [Blur2PC][x64] a byte offset, not an address
	binding->ParameterIdx = GetSharedParameterIdxByName(param->Name);
	binding->Shared = 1;

	return pBindP;
}

/// for each type of parameter, specify the bind type
const BindingType SharedEffect::m_ParamTypeToBindType[] = 
{
	NGLFX_BIND_NONE,	// NGLFX_PARAM_NONE

	// Basic data types.
	NGLFX_BIND_NONE,	//NGLFX_PARAM_BOOL,
	NGLFX_BIND_INT,		//NGLFX_PARAM_INT,
	NGLFX_BIND_FLOAT,	//NGLFX_PARAM_FLOAT,
	NGLFX_BIND_FLOAT,	//NGLFX_PARAM_VECTOR,
	NGLFX_BIND_FLOAT,	//NGLFX_PARAM_MATRIX,

	NGLFX_BIND_BOOL,	//NGLFX_PARAM_BOOL_ARRAY,
	NGLFX_BIND_INT,		//NGLFX_PARAM_INT_ARRAY,
	NGLFX_BIND_FLOAT,	//NGLFX_PARAM_FLOAT_ARRAY,
	NGLFX_BIND_FLOAT,	//NGLFX_PARAM_VECTOR_ARRAY,
	NGLFX_BIND_FLOAT,	//NGLFX_PARAM_MATRIX_ARRAY,

	// Texture parameters represent texture resources.
	NGLFX_BIND_NONE,	//NGLFX_PARAM_TEXTURE,
	NGLFX_BIND_NONE,	//NGLFX_PARAM_PROCEDURAL_TEXTURE,
	NGLFX_BIND_NONE,	//NGLFX_PARAM_TEMP_TEXTURE,

	// Sampler parameters reference texture parameters, and are what is actually assigned to the shader.
	NGLFX_BIND_SAMPLER,	//NGLFX_PARAM_SAMPLER,

};

/// find a shared parameter in the unique shared binding list
int SharedEffect::FindSharedParameterBinding(const SharedParameter &binding)
{
	for (int i=0; i<m_SharedBindings.Size(); i++)
	{
		SharedParameter *p = &m_SharedBindings[i];
		if (p->Handle == binding.Handle &&
			p->Idx    == binding.Idx &&
			p->Size   == binding.Size &&
			p->Type	  == binding.Type)
		{
			return i;
		}
	}

	return -1;
}

/// add the shared parameters of this shader into the unique shared binding list
void SharedEffect::RegisterShader(Effect *pEffect, Btl::Vector<FunctionBinding> &bindings, int type)
{
	for (int iBinding=0; iBinding<bindings.Size(); iBinding++)
	{
		FunctionBinding *p = &bindings[iBinding];
		Parameter *pParam = &pEffect->Parameters[p->ParameterIdx];
		if (!pParam->bShared)
			continue;
		if (p->Type!=NGLFX_BIND_SAMPLER && p->Type!=NGLFX_BIND_FLOAT)
			continue;

		E_SPGResource resourceType = (E_SPGResource)(((p->Type==NGLFX_BIND_SAMPLER)?E_SPGResource_Texture:E_SPGResource_PixelConstant)+type);
#if PLATFORM_PS3
		if (resourceType == E_SPGResource_PixelConstant)
			continue;
#endif

		Parameter *pBindP = GetSharedParameterByName(pParam->Name);
		if (!pBindP)
			pBindP = AddSharedParameter(pParam);
		else if (pBindP->Type ==  NGLFX_PARAM_NONE)
			InitialiseSharedParameter(pBindP, pParam);

		SharedParameter item;
		item.Idx = GetSharedParameterIdxByName(pParam->Name);
		item.Handle = p->Handle;
		item.Size = p->Count;
		item.Type = resourceType;
		item.Offset = (U32)(INTPTR)((float*)((size_t)pBindP->Data - (size_t)&ParameterData[0]));	// [Blur2PC][x64] a byte offset, not an address
		int x = FindSharedParameterBinding(item);
		if (x == -1)
		{
			// add new
			m_SharedBindings.PushBack(item);
		}
	}

	m_bUpdateSharedBindingBitfields = true;
}

/// calculate the shared binding bitfields for the given bindings
void SharedEffect::CalculateSharedBindingBitfields(Effect *pEffect, Btl::Vector<FunctionBinding> &bindings, int type, bool bShared, SharedBindingBits &usage, SharedBindingBits &mask, SharedBindingBits &global)
{
	for (int iBinding=0; iBinding<bindings.Size(); iBinding++)
	{
		FunctionBinding *p = &bindings[iBinding];
		E_SPGResource resourceType = (E_SPGResource)(((p->Type==NGLFX_BIND_SAMPLER)?E_SPGResource_Texture:E_SPGResource_PixelConstant)+type);
#if PLATFORM_PS3
		if (resourceType == E_SPGResource_PixelConstant)
		{
			if (p->Shared)
			{
				// if this is a shared pixel constant binding, which also happens to be bound to a vertex shader somewhere
				// then that means we have to copy the data out when the parameter is set, in order to patch it in the the pixel
				// shader later... so the parameter can't be global
				for (int i=0; i<m_SharedBindings.Size(); i++)
				{
					if (m_SharedBindings[i].Idx == p->ParameterIdx)
						global.ClearBit(i);
				}
			}
			continue;
		}
#endif
		Parameter *pParam = &pEffect->Parameters[p->ParameterIdx];

		int idx = p->ParameterIdx;
		int iSharedBindIdx = -1;

		if (bShared)
		{
			// add requirement for this shared binding
			SharedParameter item;
			item.Handle = p->Handle;
			item.Size = p->Count;
			idx = GetSharedParameterIdxByName(pParam->Name);
			item.Idx = idx;
			item.Type = resourceType;
			iSharedBindIdx = FindSharedParameterBinding(item);
			assert(iSharedBindIdx != -1);
			usage.SetBit(iSharedBindIdx);
		}

		// check all existing bindings to see if this one conflicts with it
		for (int i=0; i<m_SharedBindings.Size(); i++)
		{
			SharedParameter *pOther = &m_SharedBindings[i];
			if (((int)p->Handle + (int)p->Count > pOther->Handle &&
				pOther->Handle + pOther->Size > (int)p->Handle &&
				pOther->Type == resourceType &&
				idx != pOther->Idx)
				)
			{
				assert(iSharedBindIdx == -1 || iSharedBindIdx != i);
				mask.SetBit(i);
				global.ClearBit(i);
			}
		}
	}
}

/// mask any shared binding bitfields which conflict with maths constants bindings on the vertex shader
void SharedEffect::CalculateSharedBindingBitfields(Effect *pEffect, Display2::C_VertexShader *pShader, SharedBindingBits &mask, SharedBindingBits &global)
{
#if PLATFORM_PS3
	CellCgbProgram *pCgbProgram = pShader->GetCgbProgram();

	const U32 count = cellCgbGetVertexConstantCount(pCgbProgram);
	const F32* value = NULL;
	U16 reg = 0;
	for(U32 i=0; i<count; i++)
	{
		cellCgbGetVertexConstantValues(pCgbProgram, i, &reg, &value);
		
		// check all existing bindings to see if this one conflicts with it
		for (int i=0; i<m_SharedBindings.Size(); i++)
		{
			SharedParameter *pOther = &m_SharedBindings[i];
			if ((int)reg >= pOther->Handle &&
				pOther->Handle + pOther->Size > (int)reg &&
				pOther->Type == E_SPGResource_VertexConstant)
			{
				mask.SetBit(i);
				global.ClearBit(i);
			}
		}
	}
#endif
}

/// calculate the shared binding bitfields on the given effect
void SharedEffect::CalculateSharedBindingBitfields(Effect *pEffect, SharedBindingBits &global)
{
	for (int i = 0; i < pEffect->Techniques.Size(); i++)
	{
		for (int j = 0; j < pEffect->Techniques[i].Passes.Size(); j++)
		{
			Pass* pPass = &pEffect->Techniques[i].Passes[j];

			SharedBindingBits usage, mask;
			usage.ClearBits();
			mask.ClearBits();

			CalculateSharedBindingBitfields(pEffect, pPass->PixelShader.Bindings, 0, false, usage, mask, global);
			CalculateSharedBindingBitfields(pEffect, pPass->VertexShader.Bindings, 1, false, usage, mask, global);
			CalculateSharedBindingBitfields(pEffect, pPass->PixelShader.SharedBindings, 0, true, usage, mask, global);
			CalculateSharedBindingBitfields(pEffect, pPass->VertexShader.SharedBindings, 1, true, usage, mask, global);
			CalculateSharedBindingBitfields(pEffect, (VertexShader*)pPass->VertexShader.pShader, mask, global);

			pPass->SharedBindingUsage = usage;
			pPass->SharedBindingMask = mask;
		}
	}
}

inline void Mask(SharedBindingBits &a, SharedBindingBits &b)
{
	U32 n = a.SizeInBytes()/4;
	U32 *pA = (U32*)a.Data();
	U32 *pB = (U32*)b.Data();
	for (U32 i=0; i<n; i++, pA++, pB++)
		*pA &= ~*pB;
}

/// recalculate the shared parameter bitfields for each effect
void SharedEffect::UpdateSharedBindingBitfields()
{
	if (!m_bUpdateSharedBindingBitfields)
		return;

//	m_bUpdateSharedBindingBitfields = false;

	m_Global.SetBits();
	for (ReloadableEffect *pEffect = g_Manager.GetRootEffect(); pEffect; pEffect = pEffect->m_pNext)
	{
		CalculateSharedBindingBitfields(pEffect, m_Global);
	}
/*
	// remove global shared parameters from the effects
	// TODO: enable this, but bind all global parameters on the main thread first
	for (ReloadableEffect *pEffect = g_Manager.GetRootEffect(); pEffect; pEffect = pEffect->m_pNext)
	{
		for (int i = 0; i <pEffect->Techniques.Size(); i++)
		{
			for (int j = 0; j < pEffect->Techniques[i].Passes.Size(); j++)
			{
				Mask(pEffect->Techniques[i].Passes[j].SharedBindingUsage, m_Global);
			}
		}
	}
*/
}

/// show all global bindings
void SharedEffect::DumpGlobalBindings()
{
	Btl::Vector<Util::C_String> output;

	LOGINFO( g_Log, "Global bindings:");
	for (int i=0; i<m_SharedBindings.Size(); i++)
	{
		SharedParameter *p = &m_SharedBindings[i];

		Util::C_String sConflicts;
		if (m_Global[i])
		{
			sConflicts = " - Global";
		}
		else
		{
			// find the first three conflicting bindings
			int nConflicts = 3;
			for (int j=0; j<m_SharedBindings.Size(); j++)
			{
				SharedParameter *pOther = &m_SharedBindings[j];
				if (((int)p->Handle + (int)p->Size> pOther->Handle &&
					pOther->Handle + pOther->Size > (int)p->Handle &&
					pOther->Type == p->Type &&
					i != j)
					)
				{
					Util::C_String sTmp;
					if (nConflicts == 0)
					{
						sConflicts += "... (more)";
						break;
					}
					else
					{
						if (pOther->Size > 1)
							sTmp.Printf(" - %s (%d-%d)", Parameters[pOther->Idx].Name.Chars(), pOther->Handle, pOther->Handle+pOther->Size-1);
						else
							sTmp.Printf(" - %s (%d)", Parameters[pOther->Idx].Name.Chars(), pOther->Handle);
						sConflicts += sTmp;
						nConflicts--;
					}
				}
			}
		}

		static const char *sType[] = {
			"ps", "vs", "tex", "vstex"
		};

		Util::C_String sTmp;
		if (p->Size > 1)
			sTmp.Printf("    %s (%d-%d) %s%s\n", sType[p->Type], p->Handle, p->Handle+p->Size-1, Parameters[p->Idx].Name.Chars(), sConflicts.Chars());
		else
			sTmp.Printf("    %s (%d) %s%s\n", sType[p->Type], p->Handle, Parameters[p->Idx].Name.Chars(), sConflicts.Chars());

		output.PushBack(sTmp);
	}

	if (output.Size() > 0)
	{
		std::sort(output.Begin(), output.End());
		for (int i=0; i<output.Size(); i++)
			LOGINFO( g_Log, "%s", output[i].Chars());
	}
}

/// show all conflicting bindings for the given parameter
void SharedEffect::DumpConflictingBindings( Debug::Logger &log, SharedParameter *p1, Effect *pEffect, Btl::Vector<FunctionBinding> &bindings, int type, int &nConflicts)
{
	for (int iBinding=0; iBinding<bindings.Size(); iBinding++)
	{
		if (nConflicts<=0)
			return;

		FunctionBinding *p = &bindings[iBinding];
		E_SPGResource resourceType = (E_SPGResource)(((p->Type==NGLFX_BIND_SAMPLER)?E_SPGResource_Texture:E_SPGResource_PixelConstant)+type);
		Parameter *pParam = &pEffect->Parameters[p->ParameterIdx];

		// is this the same binding?
		if (pParam->bShared && p1->Handle == p->Handle && p1->Size == p->Count &&
			resourceType == p1->Type && !strcmp(pParam->Name, Parameters[p1->Idx].Name))
			continue;

		// do these bindings conflict?
		if (p1->Handle + p1->Size > (int)p->Handle &&
			(int)p->Handle + (int)p->Count > p1->Handle &&
			resourceType == p1->Type)
		{
			LOGDEBUG( log, "   %s: %s (%d-%d)", pEffect->Name.Chars(), pParam->Name.Chars(), p->Handle, p->Handle+p->Count-1);
			nConflicts--;
		}
	}
}

/// show all conflicting bindings for the given parameter
void SharedEffect::DumpConflictingBindings( Debug::Logger &log, Effect *pEffect, SharedParameter *p, int &nConflicts)
{
	for (int i = 0; i < pEffect->Techniques.Size(); i++)
	{
		for (int j = 0; j < pEffect->Techniques[i].Passes.Size(); j++)
		{
			Pass* pPass = &pEffect->Techniques[i].Passes[j];

			DumpConflictingBindings(log, p, pEffect, pPass->PixelShader.Bindings, 0, nConflicts);
			DumpConflictingBindings(log, p, pEffect, pPass->VertexShader.Bindings, 1, nConflicts);
			DumpConflictingBindings(log, p, pEffect, pPass->PixelShader.SharedBindings, 0, nConflicts);
			DumpConflictingBindings(log, p, pEffect, pPass->VertexShader.SharedBindings, 1, nConflicts);
		}
	}
}

/// find the first binding for the given parameter
SharedParameter *SharedEffect::GetSharedBinding(const char *pszName)
{
	for (int i=0; i<m_SharedBindings.Size(); i++)
	{
		if (!strcmp(Parameters[m_SharedBindings[i].Idx].Name, pszName))
			return &m_SharedBindings[i];
	}
	return NULL;
}

/// returns true if the given shared parameter conflicts with any other bindings
bool SharedEffect::DoesSharedParameterConflict(int iSharedParameter, const SharedBindingBits *pUsage)
{
	// find the first binding that applies to this parameter
	SharedParameter item = {0};
	int iBindingIdx = 0;
	for (int i=0; i<m_SharedBindings.Size(); i++)
	{
		if (m_SharedBindings[i].Idx == iSharedParameter)
		{
			item = m_SharedBindings[i];
			iBindingIdx = i;
			break;
		}
	}
	if (!item.Size)
	{
#if !PLATFORM_PS3
		assert(0); // shared parameter not found (on PS3 this happens for shared pixel shader constants)
#endif
		return true;
	}

	return !(*pUsage)[iBindingIdx];
}

}
