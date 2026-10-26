//#include "stdafx.h"

#include <core/containers/string.h>
#include <core/maths.h>
#include <btl/containers/map.h>

#include <d3d9.h>
#include <windows.h>
#include "d3d9.h"
#include "d3d9types.h"
#include "d3dx9.h"
#include <stdio.h>

#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"d3d9.lib")

#include <core/core.h>
#include <core/containers/string.h>
#include <system/scheduler/core/tl_defs.h>
#include <system/serialise/I_BinaryStream.h>
#include <system/serialise/C_MemoryStream.h>
#include <system/serialise/BinarySerialise.h>
#include <system/serialise/serialise.h>
#include <core/containers/containerserialisation.h>
#include <core/utility/endian.h>
#include <System/Serialise/C_FileSystem.h>
#include <system/Perf/PerfEvent.h>
#include <system/Perf/ProfilerUtil.h>
#include <system/Perf/ProfilerNetworkCode.h>
#include <system/Debug/Log/Debuglog.h>
#include <system/Misc/StringUtil.h>

///////#include "texture_export/d3dx/d3dx.h"
#include "fx/fxtools/fxfile.h"
#include "fx/fxtools/static_strings.h"
#include "fx/effect.h"
//#include "fx/SamplerStates.h"

#include <system/debuglog.h>
#include <algorithm>
#include <system/Perf/Profiler.h>

namespace FX
{

extern const char *g_pszPlatform;

bool CompileShader(bool bPixelShader, Effect *effect, Function *function, const char* fxname, const char* name, const char* target, const char *define, BOOL PS3_FastPrecision, Util::C_String cmdFlags);
void FixupParameters(Effect *effect);

// Parses XXXX=yyyy into an array of D3DXMACRO structures. -Ipath not currently supported.
static void ParseDefines(const char **ppszDefines, D3DXMACRO* defs, int maxdefs)
{
	int ndefs = 0;
	for (int i=0; ppszDefines[i][0] > 0; i++)
	{
		const char *p = ppszDefines[i];

		// Copy name.
		const char* start = p;
		while (isalpha(*p) || isdigit(*p) || *p == '_') p++;
		int len = p-start;
		char* name = (char*)malloc(len+1);
		strncpy(name, start, len);
		name[len] = '\0';
		defs[ndefs].Name = name;
		// Copy definition.
		if (*p == '=')
		{
			p++;
			const char* start = p;
			while (isalpha(*p) || isdigit(*p) || *p == '_') p++;
			int len = p-start;
			char* def = (char*)malloc(len+1);
			strncpy(def, start, len);
			def[len] = '\0';
			defs[ndefs].Definition = def;
		}
		else
			defs[ndefs].Definition = _strdup("1");
		ndefs++;
		if (ndefs == maxdefs-1)
			break;
	}
	defs[ndefs].Name = _strdup(g_pszPlatform);
	defs[ndefs].Definition = _strdup("1");

	defs[ndefs+1].Name = _strdup("__VARIABLE_PARSING_STAGE__");
	defs[ndefs+1].Definition = _strdup("1");

	defs[ndefs+2].Name = NULL;
	defs[ndefs+2].Definition = NULL;
}

// matches D3DDECLUSAGE
const char *g_szVertexInputSemantic[] = {
"POSITION",
"BLENDWEIGHT",
"BLENDINDICES",
"NORMAL",
"PSIZE",
"TEXCOORD",
"TANGENT",
"BINORMAL",
"TESSFACTOR",
"COLOR",
"FOG",
"DEPTH",
"SAMPLE",
""
};

// find a specific input semantic in the existing list on the effect
// returns the index, or -1 if not found
int FindVertexInputSemantic(Effect *pEffect, int Usage, int UsageIndex)
{
	for (int i=0; i<pEffect->VtxInputDeclaration.Size(); i++)
	{
		if (pEffect->VtxInputDeclaration[i].Usage == Usage && 
			pEffect->VtxInputDeclaration[i].UsageIndex == UsageIndex)
			return i; // exists
	}
	return -1; // not found
}

// add a semantic to the list of used semantics on the effect
// checks for duplicates
void AddVertexInputSemantic(Effect *pEffect, int Usage, int UsageIndex)
{
	// see if it already exists
	if (FindVertexInputSemantic(pEffect, Usage, UsageIndex) != -1)
		return;
	// add new
	InputSemantic item;
	item.Usage = (E_FXSemantic)Usage;
	item.UsageIndex = UsageIndex;
	pEffect->VtxInputDeclaration.PushBack(item);
}

// convert vertex input semantic string to 
bool GetVertexInputSemanticFromString(const char *pszName, InputSemantic *pSemantic)
{
	int i=0;
	while (g_szVertexInputSemantic[i])
	{
		int c = strlen(g_szVertexInputSemantic[i]);
		if (!strncmp(pszName, g_szVertexInputSemantic[i], c))
		{
			pSemantic->Usage = (E_FXSemantic)i;
			pSemantic->UsageIndex = atoi(pszName+c);
			return true;
		}
		i++;
	}
	return false;
}

// as above, but works with semantic strings
void AddVertexInputSemantic(Effect *pEffect, const char *pszName)
{
	InputSemantic sem;
	if (!GetVertexInputSemanticFromString(pszName, &sem))
	{
		printf("ERROR: Unrecognised vertex input semantic '%s' in AddVertexInputSemantic.\n",pszName);
		return;
	}

	AddVertexInputSemantic(pEffect, sem.Usage, sem.UsageIndex);
}

void WriteInputSemanticAnnotations(Btl::Vector<Annotation> &Annotations, const char *pszString);

// scan .fx file for input semantic annotations and put them into the Effect class
void WriteInputSemanticAnnotations(Effect *pEffect, FxFile &fxFile)
{
	PERF_EVENT("WriteInputSemanticAnnotationsWritePass");

	for (U32 i=0; i<fxFile.toks.size(); i++)
	{
		// find a token with our special name, make sure it has a token in front of it which is the name
		// of the semantic and a quoted string behind it which is the annotation string
		if (fxFile.toks[i].type == FxFile::Tok::CNAME && !(strcmp(fxFile.toks[i].c_str(), "___VERTEX_DECLARATION_ANNOTATION___")) &&
			i>2 && i<fxFile.toks.size()-2
			&& fxFile.toks[i-1].type == FxFile::Tok::WS && fxFile.toks[i+1].type == FxFile::Tok::WS
			&& fxFile.toks[i-2].type == FxFile::Tok::CNAME && fxFile.toks[i+2].type == FxFile::Tok::QUOTE
			&& fxFile.toks[i-3].type == FxFile::Tok::WS && fxFile.toks[i-3].type == FxFile::Tok::WS
			&& fxFile.toks[i-4].type == FxFile::Tok::COPERATOR && fxFile.toks[i-4] == hash_colon
			)
		{
			const char *pszSemantic = fxFile.toks[i-2].c_str();
			const char *pszAnnotation = fxFile.toks[i+2].c_str();

			InputSemantic sem;
			if (!GetVertexInputSemanticFromString(pszSemantic, &sem))
			{
				printf("WARNING: Found annotation for unrecognised vertex input semantic '%s' in .fx file .\n",pszSemantic);
				continue;
			}
			int x = FindVertexInputSemantic(pEffect, sem.Usage, sem.UsageIndex);
			if (x == -1)
			{
				printf("WARNING: Found annotation for input semantic '%s', which wasn't used on any shaders.\n",pszSemantic);
				continue;
			}
			if (pEffect->VtxInputDeclaration[x].Annotations.Size() != 0)
			{
				printf("WARNING: Found multiple annotations for input semantic '%s'. Using first found.\n", pszSemantic);
				continue;
			}

			// parse annotations
			WriteInputSemanticAnnotations(pEffect->VtxInputDeclaration[x].Annotations, pszAnnotation);
		}
	}
}

// returns true if this struct should be exported
bool NeedExportStruct(const FxFile::StructDesc *pDesc)
{
	bool bExport = false;
	for (int i=0; i<pDesc->annotations.Size(); i++)
	{
		if (!stricmp(pDesc->annotations[i].Name, "export"))
			return true;
	}
	return false;
}

// copy the data for a single member over
bool WriteMember(Btl::Vector<StructMember> &list, const FxFile::MemberDesc *pMember)
{
	int iSize = 0;

	if (!stricmp(pMember->type.c_str(), "float"))
		iSize = 1;
	if (!stricmp(pMember->type.c_str(), "float2"))
		iSize = 2;
	if (!stricmp(pMember->type.c_str(), "float3"))
		iSize = 3;
	if (!stricmp(pMember->type.c_str(), "float4"))
		iSize = 4;

	if (iSize == 0)
		return false;

	list.PushBack();
	list.Back().Name = SharedString::GetString(pMember->name.c_str());
	list.Back().Annotations = pMember->annotations;
	list.Back().Size = iSize;
	return true;
}

// export this struct into the effect
void WriteStruct(Effect *pEffect, const FxFile::StructDesc *pDesc)
{
	FX::Struct item;

	item.Name = SharedString::GetString(pDesc->name.c_str());

	list<FxFile::MemberDesc>::const_iterator iter;
	for (iter = pDesc->members.begin(); iter != pDesc->members.end(); iter++)
	{
		if (!WriteMember(item.Members, &*iter))
			return; // don't write out structs with matrices or other types that we don't recognise
	}

	pEffect->Structs.PushBack(item);
}

/// structs that have been marked as 'export' get their members written into the shader for debugging
void WriteStructs(Effect *pEffect, FxFile &fxFile)
{
	// for each struct, write out a bit of sorcery schema
	list<FxFile::StructDesc>::const_iterator iter;
	for (iter = fxFile.structDescs.begin(); iter != fxFile.structDescs.end(); iter++)
	{
		if (NeedExportStruct(&*iter))
		{
			WriteStruct(pEffect, &*iter);
		}			
	}
}

static void FreeDefines(D3DXMACRO* defs, int maxdefs)
{
	for (int i = 0; i < maxdefs && defs[i].Name; i++)
	{
		free((void*)defs[i].Name);
		free((void*)defs[i].Definition);
	}
}

// Case in-sensitive versions of D3DX methods.
D3DXHANDLE GetAnnotationByName(ID3DXEffectCompiler* Effect, D3DXHANDLE hParam, const char* name)
{
	for (UINT i = 0; ; i++)
	{
		D3DXHANDLE hAnn = Effect->GetAnnotation(hParam,i);
		if (!hAnn) break;
		D3DXPARAMETER_DESC AnnDesc;
		Effect->GetParameterDesc(hAnn, &AnnDesc);
		if (_stricmp(AnnDesc.Name, name) == 0)
			return hAnn;
	}
	return 0;
}

static const char* GetAnnotationString(ID3DXEffectCompiler* Effect, D3DXHANDLE Param, const char* Name, const char* Default=NULL)
{
	D3DXHANDLE ann = GetAnnotationByName(Effect, Param, Name);
	if (!ann) return Default;
	const char* str;
	Effect->GetString(ann, &str);
	return str;
}

static int GetAnnotationInt(ID3DXEffectCompiler* Effect, D3DXHANDLE Param, const char* Name, int Default=0)
{
	D3DXHANDLE ann = GetAnnotationByName(Effect, Param, Name);
	if (!ann) return Default;
	int val;
	Effect->GetInt(ann, &val);
	return val;
}

static BOOL GetAnnotationBOOL(ID3DXEffectCompiler* Effect, D3DXHANDLE Param, const char* Name, BOOL Default=0)
{
	D3DXHANDLE ann = GetAnnotationByName(Effect, Param, Name);
	if (!ann) return Default;
	BOOL val;
	Effect->GetBool(ann, &val);
	return val;
}

// Parsing code for the contents of passes.
#define PASS_IDENT_MAX 1024

static hString GetStateValue(FxFile::States* pPass, hString name)
{
	const vector<hString> *val = pPass->FindState(name);
	if(!val || val->size() != 1)
		return hString();
	return val->back();
}

static bool GetStateBool(FxFile::States* fxPass, hString hash_name, bool def)
{
	const vector<hString> *pBool = fxPass->FindState(hash_name);
	if(pBool == NULL)
		return def;
	return pBool->back() == hash_true || pBool->back() == hash_TRUE || atoi(pBool->back().c_str());
}

static FX::E_Bool GetStateFXBool(FxFile::States* fxPass, hString hash_name)
{
	const vector<hString> *pBool = fxPass->FindState(hash_name);
	if(pBool == NULL)
		return FX::E_Bool_Unknown;

	const char *pstr = pBool->back().c_str();
	if (pstr && pstr[0] >= '0' && pstr[0] <= '9')
		return atoi(pstr)?FX::E_Bool_True:FX::E_Bool_False;

	if (pBool->back() == hash_true || pBool->back() == hash_TRUE)
		return FX::E_Bool_True;

	if (pBool->back() == hash_false || pBool->back() == hash_FALSE)
		return FX::E_Bool_False;

	return FX::E_Bool_Unknown;
}

static float GetStateFloat(FxFile::States* pass, hString name, float def)
{
	hString value = GetStateValue(pass, name);
	if(value.c_str() == NULL)
		return def;
	return (float)atof(value.c_str());
}

static int GetStateUInt(FxFile::States* pass, hString name, int def)
{
	hString value = GetStateValue(pass, name);
	if(value.c_str() == NULL)
		return def;
	return strtoul(value.c_str(),NULL,0);
}

struct StateTableEntry
{
	hString name;
	unsigned value;
};

static int GetStateTable(FxFile::States *pPass, hString name, int nentries, const StateTableEntry* entries, unsigned def)
{
	hString value = GetStateValue(pPass, name);
	if(value.c_str() == NULL)
		return def;
	for (int i = 0; i < nentries; i++)
		if (value.LC() == entries[i].name.LC())
			return entries[i].value;
	return def;
}

// Write out annotation structures as an AnnotationList for any D3DX structure with annotations.
void WriteAnnotations(Btl::Vector<Annotation> *annotations, ID3DXEffectCompiler* DXEffect, D3DXHANDLE DXHandle, int count)
{
	annotations->Resize(count);

	for (int i = 0; i < count; i++)
	{
		D3DXHANDLE hAnn = DXEffect->GetAnnotation(DXHandle,i);
		D3DXPARAMETER_DESC AnnDesc;
		DXEffect->GetParameterDesc(hAnn, &AnnDesc);

		Annotation *ann = &annotations->At(i);
		ann->Name = SharedString::GetString(AnnDesc.Name);

		if (AnnDesc.Class == D3DXPC_OBJECT)
		{
			if (AnnDesc.Type == D3DXPT_STRING)
			{
				ann->Type = NGLFX_ANNOTATION_STRING;
				const char* str;
				DXEffect->GetString(hAnn, &str);
				ann->String = new char[strlen(str)+1];
				strcpy((char*)(size_t)ann->String, str);
				continue;
			}
		}
		if (AnnDesc.Class == D3DXPC_SCALAR)
		{
			if (AnnDesc.Type == D3DXPT_INT)
			{
				ann->Type = NGLFX_ANNOTATION_INT;
				DXEffect->GetInt(hAnn, &ann->Int);
				continue;
			}
			if (AnnDesc.Type == D3DXPT_BOOL)
			{
				ann->Type = NGLFX_ANNOTATION_BOOL;
				DXEffect->GetBool(hAnn, &ann->Bool);
				continue;
			}
			if (AnnDesc.Type == D3DXPT_FLOAT)
			{
				ann->Type = NGLFX_ANNOTATION_FLOAT;
				DXEffect->GetFloat(hAnn, &ann->Float);
				continue;
			}
		}
		if (AnnDesc.Type == D3DXPC_VECTOR)
		{
			if ( AnnDesc.Type == D3DXPT_FLOAT )
			{
				ann->Type = NGLFX_ANNOTATION_VECTOR;
				DXEffect->GetVector(hAnn, (D3DXVECTOR4*)&ann->Vector);
				continue;
			}
		}
		if (AnnDesc.Class == D3DXPC_MATRIX_ROWS || AnnDesc.Class == D3DXPC_MATRIX_COLUMNS)
		{
			if ( AnnDesc.Type == D3DXPT_FLOAT )
			{
				ann->Type = NGLFX_ANNOTATION_MATRIX;
				DXEffect->GetMatrix(hAnn, (D3DXMATRIX*)&ann->Matrix);
				continue;
			}
		}
		// Unsupported annotation type.
		ann->Type = NGLFX_ANNOTATION_NONE;
	}
}

// allocate some memory on the parameterdata array
template<typename T>
void AllocParamMemory(Effect *effect, Parameter *pParam, int n = 1)
{
	int cSize = sizeof(T) * n;
	cSize = (cSize+3)/4;
	int x = effect->ParameterData.Size();
	effect->ParameterData.Resize(x + cSize);
	pParam->Data = &effect->ParameterData[x];
	pParam->Offset = x*4;
	pParam->Size = cSize;
	memset(pParam->Data, 0, cSize*4);
}

const StateTableEntry AddressTable[] =
{
	{ hString("WRAP"),		E_TextureAddress_Wrap },
	{ hString("CLAMP"),		E_TextureAddress_Clamp },
	{ hString("MIRROR"),	E_TextureAddress_Mirror },
	{ hString("BORDER"),	E_TextureAddress_Border },
};

const StateTableEntry FilterTable[] =
{
	{ hString("POINT"),			E_TextureFilter_Point },
	{ hString("LINEAR"),		E_TextureFilter_Linear },
	{ hString("ANISOTROPIC"),	E_TextureFilter_Anisotropic },
};

ACTRESULT WriteSamplerParameter(Parameter *param, Effect *effect, ID3DXEffectCompiler* DXEffect, D3DXHANDLE hParam, FxFile &fxFile, FxFile::Var *fxVar)
{
	// Regular texture reference.  Assumed to be filled in at runtime.
	param->Type = NGLFX_PARAM_SAMPLER;

	AllocParamMemory<nglFXParam_Sampler>(effect, param);

	int U = GetStateTable(fxVar, hash_AddressU, tl_elementsof(AddressTable), AddressTable, E_TextureAddress_Wrap);
	int V = GetStateTable(fxVar, hash_AddressV, tl_elementsof(AddressTable), AddressTable, E_TextureAddress_Wrap);
	int W = GetStateTable(fxVar, hash_AddressW, tl_elementsof(AddressTable), AddressTable, E_TextureAddress_Wrap);

	int mip = GetStateTable(fxVar, hash_MipFilter, tl_elementsof(FilterTable), FilterTable, E_TextureFilter_Point);
	int min = GetStateTable(fxVar, hash_MinFilter, tl_elementsof(FilterTable), FilterTable, E_TextureFilter_Point);
	int mag = GetStateTable(fxVar, hash_MagFilter, tl_elementsof(FilterTable), FilterTable, E_TextureFilter_Point);

	int aniso = GetStateUInt(fxVar, hash_MaxAnisotropy, 1);
	int bias = GetStateFloat(fxVar,hash_MipMapLodBias, 0.0f);

	int srgb = GetStateBool(fxVar, hash_SRGBTexture, false);

	int X = ((U&7)<<0) + ((V&7)<<3) + ((W&7)<<6);
	X += ((mip&3)<<9) + ((min&3)<<11) + ((mag&3)<<13);
	X += ((aniso&15)<<16) + ((srgb&1)<<20) + ((bias&127)<<21);

	param->Sampler->iSamplerState = X;	

	param->Size = 1; // this is a hack that prevents the sample states being overwritten when the texture itself is set

	return ACT_OK;
}

bool operator <(Parameter &a, Parameter &b)
{
	return (a.Type != NGLFX_PARAM_TEXTURE && b.Type == NGLFX_PARAM_TEXTURE);
}

void SortTextureParametersToBack(Effect *pEffect)
{
	int n = pEffect->Parameters.Size();
	if (!n)
		return;
	std::sort(&pEffect->Parameters[0], &pEffect->Parameters[0]+n);
}

/// find all sampler parameters and give them the name of the texture parameter that they're associated with
/// also copies the annotations off the texture onto the sampler
void FixupSamplerParameterNames(Effect *pEffect, FxFile &fxFile)
{
	for (int iParam=0; iParam<pEffect->Parameters.Size(); iParam++)
	{
		Parameter *pParam = &pEffect->Parameters[iParam];

		if (pParam->Type == NGLFX_PARAM_SAMPLER)
		{
			hString hash_name(pParam->Name);

			FxFile::Var *fxVar = fxFile.FindVar(hash_name);

			const vector<hString> *val = fxVar->FindState(hash_Texture);

			if(val && val->size() == 3) {
				hString hash_tex = (*val)[1];

				// find the texture parameter for this sampler
				for (int i = 0; i < pEffect->Parameters.Size(); i++)
				{
					Parameter *pTex = &pEffect->Parameters[i];

					if (!_stricmp(hash_tex.c_str(), pTex->Name.Chars()))
					{
						// copy the name and annotations from the texture to the sampler
						pParam->Name = pTex->Name;

						for (int iA=0; iA<pTex->Annotations.Size(); iA++)
							pParam->Annotations.PushBack(pTex->Annotations[iA]);

						pParam->bShared |= pTex->bShared;

						// delete the texture parameter
						pEffect->Parameters.EraseByIndex(i);
						assert(i>iParam);
						break;
					}
				}
			}
		}
	}
}

Btl::Map<Util::C_String,int> g_StructOffsetMap;

int GetStructOffset(Util::C_String name)
{
	Btl::Map<Util::C_String,int>::Iterator i = g_StructOffsetMap.Find(name);
	if (i == g_StructOffsetMap.End())
	{
		printf( "WARNING: Couldn't find struct member %s.\n", name.Chars());
		return 0;
	}

	return i->GetValue();
}

/// utility function that gets all of the default data out for a particular parameter
void GetParameter(ID3DXEffectCompiler* pEffect, D3DXHANDLE hParam, Btl::Vector<float> &data, Util::C_String name = "")
{
	int c = 0;
	D3DXPARAMETER_DESC ParamDesc;
	pEffect->GetParameterDesc(hParam, &ParamDesc);
	name = name + ParamDesc.Name;
	g_StructOffsetMap.Insert(name,data.Size());

	switch (ParamDesc.Class)
	{
	case D3DXPC_STRUCT:
		for (U32 i=0; i<ParamDesc.StructMembers; i++)
		{
			D3DXHANDLE memberHandle=pEffect->GetParameter(hParam,i);
			GetParameter(pEffect, memberHandle, data, name + ".");
		}
		return;
	case D3DXPC_SCALAR:
	case D3DXPC_VECTOR:
	case D3DXPC_MATRIX_ROWS:
	case D3DXPC_MATRIX_COLUMNS:
		assert((ParamDesc.Bytes&3) == 0);
		c = ParamDesc.Bytes / 4;
		break;
	default:
		c = 0;
		break;
	}

	// get the default data for this parameter
	if (c > 0)
	{
		int x = data.Size();
		data.Resize(data.Size() + c);

		pEffect->GetValue(hParam, &data[x], c*4);
	}
}

// Write out parameter structures for all supported D3DX parameters in the effect file.
static ACTRESULT WriteParameters(Effect *effect, ID3DXEffectCompiler* DXEffect, const char* fxname, FxFile &fxFile)
{
	PERF_EVENT("WriteParameters");

	D3DXEFFECT_DESC EffectDesc;
	DXEffect->GetDesc(&EffectDesc);

	effect->Parameters.Resize(EffectDesc.Parameters);

	for (UINT iParam = 0; iParam < EffectDesc.Parameters; iParam++)
	{
		D3DXHANDLE hParam = DXEffect->GetParameter(NULL, iParam);

		D3DXPARAMETER_DESC ParamDesc;
		DXEffect->GetParameterDesc(hParam, &ParamDesc);

		Parameter *param = &effect->Parameters[iParam];
		param->Name = SharedString::GetString(ParamDesc.Name);

		hString hash_name(ParamDesc.Name);

		FxFile::Var *fxVar = fxFile.FindVar(hash_name);

		if (!fxVar)
		{
			printf("ERROR: Failed to find variable %s in file\n",hash_name.c_str());
			return ACT_ERROR;
		}

		if (ParamDesc.Semantic)
			param->Semantic = SharedString::GetString(ParamDesc.Semantic);

		param->Type = NGLFX_PARAM_NONE;
		param->Data = NULL;
		param->Offset = -1;
		param->Size = 0;
//		param->SecondaryNext = NULL;
		param->bShared = ParamDesc.Flags & D3DX_PARAMETER_SHARED;

		const char* semantic = ParamDesc.Semantic;
		if (!semantic) semantic = ParamDesc.Name;

		// Write parameter annotations.
		WriteAnnotations(&param->Annotations, DXEffect, hParam, ParamDesc.Annotations);

		if ( ParamDesc.Class == D3DXPC_OBJECT )
		{
			if (ParamDesc.Type == D3DXPT_TEXTURE || ParamDesc.Type == D3DXPT_TEXTURE1D || ParamDesc.Type == D3DXPT_TEXTURE2D || ParamDesc.Type == D3DXPT_TEXTURE3D || ParamDesc.Type == D3DXPT_TEXTURECUBE)
			{
				param->Type = NGLFX_PARAM_TEXTURE;

				continue;
			}
			if (ParamDesc.Type == D3DXPT_SAMPLER || ParamDesc.Type == D3DXPT_SAMPLER1D || ParamDesc.Type == D3DXPT_SAMPLER2D || ParamDesc.Type == D3DXPT_SAMPLER3D || ParamDesc.Type == D3DXPT_SAMPLERCUBE)
			{
				if (WriteSamplerParameter(param, effect, DXEffect, hParam, fxFile,fxVar) != ACT_OK)
					return ACT_ERROR;
				continue;
			}
		}
		else
		{
			Btl::Vector<F32> data;
			GetParameter(DXEffect, hParam, data);
			while (data.Size() &3)
				data.PushBack(0.f);

			if (data.Size() > 0)
			{
				AllocParamMemory<F32>(effect, param, data.Size());
				memcpy(param->Data, &data[0], data.Size()*sizeof(F32));
				param->Type = NGLFX_PARAM_VECTOR;
			}
			else
				param->Type = NGLFX_PARAM_NONE;
		}
	}

	return ACT_OK;
}

const StateTableEntry BlendTable[] =
{
	{ hString("ZERO"), E_Blend_Zero },
	{ hString("ONE"), E_Blend_One },
	{ hString("SRCCOLOR"), E_Blend_SrcColour },
	{ hString("INVSRCCOLOR"), E_Blend_InvSrcColour },
	{ hString("SRCALPHA"), E_Blend_SrcAlpha },
	{ hString("INVSRCALPHA"), E_Blend_InvSrcAlpha },
	{ hString("DESTALPHA"), E_Blend_DstAlpha },
	{ hString("INVDESTALPHA"), E_Blend_InvDstAlpha },
	{ hString("DESTCOLOR"), E_Blend_DstColour },
	{ hString("INVDESTCOLOR"), E_Blend_InvDstColour },
};

const StateTableEntry BlendOpTable[] =
{
	{ hString("ADD"), E_BlendOp_Add },
	{ hString("SUBTRACT"), E_BlendOp_Subtract },
	{ hString("REVSUBTRACT"), E_BlendOp_RevSubtract },
	{ hString("MIN"), E_BlendOp_Min },
	{ hString("MAX"), E_BlendOp_Max },
};



#define	GPU_CMP_ZDEFAULT	GPU_CMP_LESSEQUAL

const StateTableEntry CompareTable[] =
{
	{ hString("NEVER"), E_CmpFunc_Never },
	{ hString("ALWAYS"), E_CmpFunc_Always },
	{ hString("LESS"), E_CmpFunc_Less },
	{ hString("LESSEQUAL"), E_CmpFunc_LessEqual },
	{ hString("GREATER"), E_CmpFunc_Greater },
	{ hString("GREATEREQUAL"), E_CmpFunc_GreaterEqual },
	{ hString("EQUAL"), E_CmpFunc_Equal },
	{ hString("NOTEQUAL"), E_CmpFunc_NotEqual },
	{ hString("NEAR_NEVER"), E_CmpFunc_Never },
	{ hString("NEAR_ALWAYS"), E_CmpFunc_Always },
	{ hString("NEAR_LESS"), E_CmpFunc_Less },
	{ hString("NEAR_LESSEQUAL"), E_CmpFunc_LessEqual },
	{ hString("NEAR_GREATER"), E_CmpFunc_Greater },
	{ hString("NEAR_GREATEREQUAL"), E_CmpFunc_GreaterEqual },
	{ hString("NEAR_EQUAL"), E_CmpFunc_Equal },
	{ hString("NEAR_NOTEQUAL"), E_CmpFunc_NotEqual },
};



const StateTableEntry StencilTable[] =
{
	{ hString("KEEP"), E_StencilOp_Keep },
	{ hString("ZERO"), E_StencilOp_Zero },
	{ hString("REPLACE"), E_StencilOp_Replace },
	{ hString("INCRSAT"), E_StencilOp_IncrSat },
	{ hString("INCR"), E_StencilOp_Incr },
	{ hString("DECRSAT"), E_StencilOp_DecrSat },
	{ hString("DECR"), E_StencilOp_Decr },
	{ hString("INVERT"), E_StencilOp_Invert },
};

const StateTableEntry CullTable[] =
{
	{ hString("CW"), E_CullMode_Cw },
	{ hString("CCW"), E_CullMode_Ccw },
	{ hString("NONE"), E_CullMode_None },
};

const StateTableEntry ShadeTable[] =
{
	{ hString("FLAT"), 0 },
	{ hString("GOURAUD"), 1 },
};

const StateTableEntry FillTable[] =
{
	{ hString("POINT"), E_FillMode_Point },
	{ hString("WIREFRAME"), E_FillMode_Wireframe },
	{ hString("SOLID"), E_FillMode_Solid },
};


static void WriteRenderStates(Pass *pass, FxFile::PassDesc *pPass)
{
	pass->RenderStates.AlphaBlendEnable = GetStateFXBool(pPass, hString("AlphaBlendEnable"));
	pass->RenderStates.SrcBlend = (E_Blend)GetStateTable(pPass, hString("SrcBlend"), tl_elementsof(BlendTable), BlendTable, 0);
	pass->RenderStates.DestBlend = (E_Blend)GetStateTable(pPass, hString("DestBlend"), tl_elementsof(BlendTable), BlendTable, 0);
	pass->RenderStates.BlendOp = (E_BlendOp)GetStateTable(pPass, hString("BlendOp"), tl_elementsof(BlendOpTable), BlendOpTable, 0);
	pass->RenderStates.TextureFactor = GetStateUInt(pPass, hString("TextureFactor"), 0xffffffff);

	pass->RenderStates.AlphaTestEnable = GetStateFXBool(pPass, hString("AlphaTestEnable"));
	pass->RenderStates.AlphaFunc = (E_CmpFunc)GetStateTable(pPass, hString("AlphaFunc"), tl_elementsof(CompareTable), CompareTable, 0);
	pass->RenderStates.AlphaRef = GetStateUInt(pPass, hString("AlphaRef"), 0);

	pass->RenderStates.DepthBias = GetStateFloat(pPass, hString("DepthBias"), 0.0f);
	pass->RenderStates.SlopeScaleDepthBias = GetStateFloat(pPass, hString("SlopeScaleDepthBias"), 0.0f);

	pass->RenderStates.DitherEnable = GetStateBool(pPass, hString("DitherEnable"), false);
	pass->RenderStates.CullMode = (E_CullMode)GetStateTable(pPass, hString("CullMode"), tl_elementsof(CullTable), CullTable, 0);
	pass->RenderStates.FillMode = (E_FillMode)GetStateTable(pPass, hString("FillMode"), tl_elementsof(FillTable), FillTable, 0);
	pass->RenderStates.ShadeMode = (gpuShadeMode)GetStateTable(pPass, hString("ShadeMode"), tl_elementsof(ShadeTable), ShadeTable, 0);

	uint ColorWriteEnable = GetStateUInt(pPass, hString("ColorWriteEnable"), 0xf);
	uint gpumask = 0;
	if (ColorWriteEnable & 1) gpumask |= GPU_WRITE_R;
	if (ColorWriteEnable & 2) gpumask |= GPU_WRITE_G;
	if (ColorWriteEnable & 4) gpumask |= GPU_WRITE_B;
	if (ColorWriteEnable & 8) gpumask |= GPU_WRITE_A;
	pass->RenderStates.ColorWriteEnable = (gpuWriteMask)gpumask;

	pass->RenderStates.ZTestEnable = GetStateBool(pPass, hString("ZEnable"), true);

	pass->RenderStates.ZFunc = (E_CmpFunc)GetStateTable(pPass, hString("ZFunc"), tl_elementsof(CompareTable), CompareTable, 0);

	pass->RenderStates.ZWriteEnable = GetStateBool(pPass, hString("ZWriteEnable"), true);

	pass->RenderStates.StencilEnable = GetStateBool(pPass, hString("StencilEnable"), false);
	pass->RenderStates.StencilPass = (E_StencilOp)GetStateTable(pPass, hString("StencilPass"), tl_elementsof(StencilTable), StencilTable, 0);
	pass->RenderStates.StencilFail = (E_StencilOp)GetStateTable(pPass, hString("StencilFail"), tl_elementsof(StencilTable), StencilTable, 0);
	pass->RenderStates.StencilZFail = (E_StencilOp)GetStateTable(pPass, hString("StencilZFail"), tl_elementsof(StencilTable), StencilTable, 0);
	pass->RenderStates.StencilFunc = (E_CmpFunc)GetStateTable(pPass, hString("StencilFunc"), tl_elementsof(CompareTable), CompareTable, 0);
	pass->RenderStates.StencilMask = GetStateUInt(pPass, hString("StencilMask"), 0xffffffff);
	pass->RenderStates.StencilRef = GetStateUInt(pPass, hString("StencilRef"), 0);
	pass->RenderStates.StencilWriteMask = GetStateUInt(pPass, hString("StencilWriteMask"), 0xffffffff);

	pass->RenderStates.PointSpriteEnable = GetStateBool(pPass, hString("PointSpriteEnable"), false);
	pass->RenderStates.PointSize = GetStateFloat(pPass, hString("PointSize"), 0);
	pass->RenderStates.PointSize_Min = GetStateFloat(pPass, hString("PointSize_Min"), 0);
	pass->RenderStates.PointSize_Max = GetStateFloat(pPass, hString("PointSize_Max"), 0);

	pass->RenderStates.AlphaToMaskEnable = GetStateFXBool(pPass, hString("AlphaToMaskEnable"));

	// Xenon specific states not supported on other platforms.
	pass->RenderStates.Tessellate = 0xffffffff;
	pass->RenderStates.TessellationLevel = 1.0f;

	pass->RenderStates.AlphaToMaskOffsets = 0;

	pass->RenderStates.HiZEnable = 0;
}

char *SkipWhitespace(char *p)
{
	while (*p == ' ')
		p++;
	return p;
}

Util::C_String GenerateAnnotationCmdLineParams(Btl::Vector<Annotation> &annotations)
{
	Util::C_String sRtn;
	for (int i=0; i<annotations.Size(); i++)
	{
		Annotation *pA = &annotations[i];
		if (pA->Type == NGLFX_ANNOTATION_STRING && !stricmp(pA->Name,"Define"))
		{
			// extract definitions (separated by ;) and add to commandline
			Util::C_String buf = pA->String;
			char *p = (char*)buf.Chars();
			while (1)
			{
				p = SkipWhitespace(p);

				char *pNext = strchr(p, ';');
				if (pNext)
					*pNext = 0;

				sRtn += Util::C_String("-D") + p + " ";
				if (!pNext)
					break;
				p = pNext + 1;
			}
		}
	}

	return sRtn;
}

ACTRESULT WritePass(Effect *effect, Pass *pass, ID3DXEffectCompiler* DXEffect, D3DXHANDLE DXPass, const char* fxname, FxFile &fxFile, FxFile::TechniqueDesc *fxTech, Util::C_String cmdFlags)
{
	PERF_EVENT("WritePass");

	memset( pass, 0, sizeof(Pass) );

	D3DXPASS_DESC PassDesc;
	DXEffect->GetPassDesc(DXPass, &PassDesc);
	pass->Name = SharedString::GetString(PassDesc.Name);

	hString hash_pass(PassDesc.Name);
	FxFile::PassDesc *fxPass=fxTech->FindPass(hash_pass);

	if (fxTech == NULL) 
	{
		printf("ERROR: Failed to find pass %s in file\n",hash_pass.c_str());
		return ACT_ERROR;
	}

	const vector<hString> *pixelShader = fxPass->FindState(hash_PixelShader);

	// todo- More general system for passing things to the shader compiler, for example arbitrary command line parameters.
	BOOL PS3_FastPrecision = GetAnnotationBOOL(DXEffect, DXPass, "FastPrecision", 1);

	hString pixel = (*pixelShader)[2];
	hString pixel_target = (*pixelShader)[1];

	WriteAnnotations(&pass->Annotations, DXEffect, DXPass, PassDesc.Annotations);

	// get #defines for this pass and add then to cmdFlags
	Util::C_String cmdFlags2 = cmdFlags + " " + GenerateAnnotationCmdLineParams(pass->Annotations);
	
	if (!CompileShader(true, effect, &pass->PixelShader, fxname, pixel.c_str(), pixel_target.c_str(), NULL, PS3_FastPrecision, cmdFlags2))
		return ACT_ERROR;

	const vector<hString> *vertexShader = fxPass->FindState(hash_VertexShader);
	if (!vertexShader)
	{
		printf("ERROR: VertexShader not specified for technique '%s' pass '%s'\n", fxTech->name.c_str(), fxPass->name.c_str());
		return ACT_ERROR;
	}

	hString vertex = (*vertexShader)[2];
	hString vertex_target = (*vertexShader)[1];

	if (!CompileShader(false, effect, &pass->VertexShader, fxname, vertex.c_str(), vertex_target.c_str(), NULL, PS3_FastPrecision, cmdFlags2))
		return ACT_ERROR;

	WriteRenderStates(pass, fxPass );

	return ACT_OK;
}

bool operator <(InputSemantic &a, InputSemantic &b)
{
	if (a.Usage > b.Usage)
		return false;
	if (a.Usage < b.Usage)
		return true;
	return a.UsageIndex < b.UsageIndex;
}

void SortInputSemantics(Effect *pEffect)
{
	PERF_EVENT("SortInputSemantics");

	int n = pEffect->VtxInputDeclaration.Size();
	if (!n)
		return;
	InputSemantic *pS = &pEffect->VtxInputDeclaration[0];
	std::sort(pS, pS+n);
}

static Effect *WriteEffect(ID3DXEffectCompiler* DXEffect, const char* fxname, FxFile &fxFile, Util::C_String cmdFlags)
{
	PERF_EVENT("WriteEffect");

	// Add effect structure.
	D3DXEFFECT_DESC EffectDesc;
	DXEffect->GetDesc(&EffectDesc);

	if (EffectDesc.Techniques == 0)
		return NULL;

	Effect *effect = new Effect;
	memset(effect, 0, sizeof(Effect));

	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath(fxname, drive, dir, fname, ext);
	effect->Name = SharedString::GetString(fname);

	// Write parameters.
	if (WriteParameters(effect, DXEffect, fxname,fxFile) != ACT_OK)
	{
		delete effect;
		return NULL;
	}

	// put texture parameters at the end, this is so we can safely delete them later on
	SortTextureParametersToBack(effect);

	// Write techniques and passes.
	effect->Techniques.Resize(EffectDesc.Techniques);
	memset( &effect->Techniques[0], 0, sizeof(Technique)*EffectDesc.Techniques );

	for (unsigned t = 0; t < EffectDesc.Techniques; t++)
	{
		D3DXHANDLE DXTechnique = DXEffect->GetTechnique(t);
		D3DXTECHNIQUE_DESC TechniqueDesc;
		DXEffect->GetTechniqueDesc(DXTechnique, &TechniqueDesc);

		hString hash_tech(TechniqueDesc.Name);
		FxFile::TechniqueDesc *fxTech = fxFile.FindTechnique(hash_tech);

		if(fxTech == NULL) {
			printf("ERROR: Failed to find technique %s in file\n",hash_tech.c_str());
			delete effect;
			return NULL;
		}

		Technique *technique = &effect->Techniques[t];
		technique->Name = SharedString::GetString(TechniqueDesc.Name);

		technique->Passes.Resize(TechniqueDesc.Passes);

		WriteAnnotations(&technique->Annotations, DXEffect, DXTechnique, TechniqueDesc.Annotations);

		// get #defines for this technique and add then to cmdFlags
		Util::C_String cmdFlags2 = cmdFlags + " " + GenerateAnnotationCmdLineParams(technique->Annotations);

		for (unsigned p = 0; p < TechniqueDesc.Passes; p++)
		{
			D3DXHANDLE DXPass = DXEffect->GetPass(DXTechnique, p);
			Pass *pass = &technique->Passes[p];

			if (WritePass(effect, pass, DXEffect, DXPass, fxname, fxFile, fxTech, cmdFlags2) != ACT_OK)
			{
				delete effect;
				return NULL;
			}
		}
	}

	// write input semantic annotations
	WriteInputSemanticAnnotations(effect, fxFile);

	// sort input semantic annotations
	SortInputSemantics(effect);

	// write out the members of structs that have been flagged with 'export'
	WriteStructs(effect, fxFile);

	// copy sampler parameter names from their associated texture params
	FixupSamplerParameterNames(effect, fxFile);

	return effect;
}

// implementation of a d3dx utility class that reads #include files
class D3DXIncludeImpl : public ID3DXInclude
{
public:
	D3DXIncludeImpl(const char **ppszIncludes)
	{
		m_ppszIncludes = ppszIncludes;
	}

	HRESULT __stdcall Open( D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID * ppData, UINT * pBytes, LPSTR, DWORD )
	{
		return Open(IncludeType,pFileName,pParentData,ppData,pBytes);
	}

	HRESULT __stdcall Open( D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID * ppData, UINT * pBytes )
	{
		int i=-1;
		while (1)
		{
			Util::C_String sName;
			if (i == -1)
				sName = Util::C_String(pFileName);
			else
				sName = Util::C_String(m_ppszIncludes[i])+Util::C_String("\\")+Util::C_String(pFileName);
			Sys::C_FileInfo info;
			if (!Sys::GetDefaultFileSystem()->FileInfo(sName, &info))
			{
				i++;
				if (m_ppszIncludes[i][0]==0)
					break;
				continue;
			}
			int c = info.m_FileSize;
			*pBytes = c;
			U8 *pData = new U8[info.m_FileSize];
			Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open(sName);
			if (pFile)
			{
				pFile->Read(pData, c);
				delete pFile;
				*ppData = pData;
				return S_OK;
			}
			else
			{
				perror(pFileName);
				break;
			}
		}
		perror(pFileName);
		*pBytes = 0;
		printf("ERROR: Include file '%s' not found.\n", pFileName);
		return S_FALSE;
	}
	HRESULT __stdcall Close( LPCVOID pData )
	{
		delete [] pData;
		return S_OK;
	}
	const char **m_ppszIncludes;
};

Util::C_String GenerateCompilerFlags(const char **ppszStrings, char letter)
{
	Util::C_String out, tmp;
	for (int i=0; ppszStrings[i][0]>0; i++)
	{
		if (letter == 'D')
			tmp.Printf("-%c%s ", letter, ppszStrings[i]);
		else
			tmp.Printf("-%c\"%s\" ", letter, ppszStrings[i]);
		out = out + tmp;
	}
	return out;
}

Util::C_String g_sOutputFilename;
int g_iTempFilename = 0;

void InitialiseTemporaryFilename(const char *pszOutputFilename)
{
	Util::C_String sOut(pszOutputFilename);
	g_sOutputFilename.Printf("%s%%d.bin", Util::RemoveFileExtension(sOut).Chars());

	g_iTempFilename = 0;
}

Util::C_String GetTemporaryFilename()
{
	Util::C_String sTmp;
	sTmp.Printf(g_sOutputFilename.Chars(), g_iTempFilename++);
	return sTmp;
}

static bool SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char **ppszDefines, const char **ppszIncludes, const char *pszOutputFilename)
{
	Effect *pEffect = NULL;

	InitialiseTemporaryFilename(pszOutputFilename);

//	Perf::ProfilerCommon::Instance().RegisterCallback(Perf::g_CapturePerfCallbackFunction);

//	Debug::AutoConfigureDebugLog();

	PERF_EVENT("SerialOutFX");
	{

	// Parse macro definitions for D3D.
	D3DXMACRO defs[64];
	ParseDefines(ppszDefines, defs, tl_elementsof(defs));
	// setup include paths for D3D
	D3DXIncludeImpl includes(ppszIncludes);

	ID3DXBuffer* ErrorBuf;
	ID3DXEffectCompiler* Compiler;
	{
		PERF_EVENT("CreateEffectCompilerFromFile");
		D3DXCreateEffectCompilerFromFile( fxname, defs, &includes, 0, &Compiler, &ErrorBuf );
	}

	if (ErrorBuf)
	{
		printf("%s", (const char*)ErrorBuf->GetBufferPointer() );
		ErrorBuf->Release();
	}
	if (!Compiler)
		return false;

	FxFile fxFile;
	{
		PERF_EVENT("FxFile");
		map<string,string> defines;
		{
			int i=0;
			while(defs[i].Name) {
				defines[defs[i].Name] = defs[i].Definition;
				i++;
			}
			defines["FXFILE_PARSER"] = "1";
			defines[g_pszPlatform] = "1";
		}
		std::list<std::string> includes;
		for (int i=0; ppszIncludes[i][0]>0; i++)
			includes.push_back(std::string(ppszIncludes[i]));
		fxFile.Init(fxname,&defines,&includes);
	}
	if (!fxFile.isInited)
		return false;

	// create compiler commandline flags from defines and includes list
	Util::C_String cmdFlags;
	cmdFlags.Printf("-D%s=1 ", g_pszPlatform);
	cmdFlags = cmdFlags + GenerateCompilerFlags(ppszDefines, 'D') + GenerateCompilerFlags(ppszIncludes, 'I');

	// generate effect structures
	pEffect = WriteEffect(Compiler, fxname, fxFile, cmdFlags);
	if (pEffect)
	{
		if (pStream->GetEndianSwapFlag())
		{
			Sys::BinarySerialiser<Ut::DoEndianSwap> ar(*pStream);
			ar << *pEffect;
		}
		else
		{
			Sys::BinarySerialiser<Ut::NoEndianSwap> ar(*pStream);
			ar << *pEffect;
		}
		delete pEffect;
	}

	FreeDefines(defs, tl_elementsof(defs));

	Compiler->Release();

	//End frame and print perf data
	}
	Perf::ProfilerCommon::Instance().EndFrame();

	return pEffect != NULL;
}

}
