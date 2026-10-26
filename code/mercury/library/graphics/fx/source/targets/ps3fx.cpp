#include "stdafx.h"

#include <format/cgb_format.h>

#define FX_PS3

typedef int BOOL;

#include <core/core.h>
#include <core/containers/string.h>
#include <system/scheduler/core/tl_defs.h>
#include <system/serialise/I_BinaryStream.h>
#include <system/serialise/BinarySerialise.h>
#include <core/utility/endian.h>

#define NGL_TOOL 1

#include "fx/targets/cell/gcm.h"

#include "ngl/mercury_to_ngl.h"
#include "ngl/defs.h"
#include "ngl/system.h"
#include "fx/targets/ps3fx.h"

#define NGL_API
#include "ngl/ps3/ngl_ps3_gpu_types.h"

#include "source/fx_export.cpp"

#define snprintf _snprintf

// Parsing code for sce-cgc assembly listing headers.
#define LIST_FIELD_MAX 1024

namespace FX
{

const char *g_pszPlatform = "PLATFORM_PS3";

void ListNextField(const char*& ch)
{
	while (*ch && *ch != '\n' && *ch != ':') ch++; 
	if (*ch == ':') ch++;
}

void ListReadField(const char*& ch, char* ident)
{
	char* dst = ident;
	while (*ch && *ch != '\n' && *ch != ':' && dst-ident<LIST_FIELD_MAX-1) *dst++ = *ch++;
	*dst = 0;
	if (*ch == ':') ch++;
}

void ListReadWhite(const char*& ch) 
{ 
	while (*ch && isspace(*ch)) ch++; 
}

void ListNextIdent(const char*& ch)
{
	ListReadWhite(ch);
	while (*ch && !isspace(*ch)) ch++;
	ListReadWhite(ch);
}

void ListReadIdent(const char*& ch, char* ident)
{
	ListReadWhite(ch);
	char* dst = ident;
	while (*ch && !isspace(*ch) && dst-ident<LIST_FIELD_MAX-1) *dst++ = *ch++;
	*dst = 0;
	ListReadWhite(ch);
}

void ListNextLine(const char*& ch)
{
	while (*ch && *ch != '\n') ch++;
	if (*ch == '\n') ch++;
}

void MergeBindings(Effect *pEffect, Function *pFunction)
{
	for (int i=0; i<pFunction->Bindings.Size(); i++)
	{
		FunctionBinding *pBinding = &pFunction->Bindings[i];
		Parameter *pParam = &pEffect->Parameters[pBinding->ParameterIdx];

		// only merge shader constants, not textures
		if (pBinding->Type != NGLFX_BIND_FLOAT)
			continue;

		// check whether the parameter's size and the binding's size agree
		if (pParam->Size > (int)pBinding->Count*4)
		{
			// binding comes from a struct. see how many more elements there are in the struct
			// delete as many bindings, as long as their resources are sequential
			int iHandle = pBinding->Handle + pBinding->Count;
			int iEndHandle = pBinding->Handle + pParam->Size/4;
			for (int x=1; iHandle<iEndHandle && x+i<pFunction->Bindings.Size();x++)
			{
				FunctionBinding *pOtherBinding = &pFunction->Bindings[x+i];
				if (pOtherBinding->Type == NGLFX_BIND_FLOAT &&
					pOtherBinding->ParameterIdx == pBinding->ParameterIdx &&
					pOtherBinding->Handle == iHandle)
				{
					pOtherBinding->Type = 0xf; // mark this binding for deletion
					iHandle += pOtherBinding->Count;
					pBinding->Count = Maths::Max(pOtherBinding->Handle - pBinding->Handle + pOtherBinding->Count, pBinding->Count);
				}
				else
					break;
			}
		}
	}

	// delete all the marked bindings
	for (int i=0; i<pFunction->Bindings.Size(); i++)
	{
		if (pFunction->Bindings[i].Type == 0xf)
		{
			pFunction->Bindings.EraseByIndex(i);
			i--;
		}
	}

	// any parameters that have an offset, move their base register and remove the offset
	// this is because the FX system doesn't understand setting part of a shared parameter
	for (int i=0; i<pFunction->Bindings.Size(); i++)
	{
		FunctionBinding *pBinding = &pFunction->Bindings[i];
		Parameter *pParam = &pEffect->Parameters[pBinding->ParameterIdx];

		// only process shader constants, not textures
		if (pBinding->Type != NGLFX_BIND_FLOAT)
			continue;

		if (pBinding->Offset > 0)
		{
			int n = pBinding->Offset/4;
			pBinding->Count += n;
			pBinding->Handle -= n;
			pBinding->Offset = 0;
		}

		if (pBinding->Count < (U32)pParam->Size/4)
			pBinding->Count = pParam->Size/4;
	}
}

struct Float4
{
	float x,y,z,w;
};

template <typename T>
inline T my_align(T p, size_t a) { return (T)(((size_t)(p)+((a)-1))&(~((a)-1))); }

// remove default values from the CGB file's constant table. only leave internal values used for arithmetic or logic 
void StripDefaultValuesFromCGB(Effect *pEffect, Function *function)
{
	U32 n = (function->cMicrocode+3)/4;
	U32 *pBuffer = new U32[n], *pIn = (U32*)function->Microcode;
	memcpy(pBuffer, pIn, n*4);

	CellCgbHeader *pHeader = (CellCgbHeader*)pBuffer;
	CellCgbHeader header = *pHeader;
	Ut::DoEndianSwap::EndianSwap(header.ucode_size);
	BIZ_ASSERT(header.content&CELL_CGB_CONSTANT_TABLE);

	CellCgbVertexConfigurationInternal *pConfig = (CellCgbVertexConfigurationInternal*)(((U8*)pBuffer)+sizeof(CellCgbHeader));

	U32 cOffset1 = sizeof(CellCgbHeader)+sizeof(CellCgbVertexConfigurationInternal)+4+my_align(header.ucode_size,16);
	U8 *pOffset1 = (((U8*)pBuffer)+cOffset1);

	CellCgbLevelAHeader *pConstHeader = (CellCgbLevelAHeader*)(pOffset1);
	CellCgbLevelAHeader constHeader = *pConstHeader;
	Ut::DoEndianSwap::EndianSwap(constHeader.constant_count);

	unsigned short *pConstantRegisters = (unsigned short*)(pOffset1+sizeof(CellCgbLevelAHeader));
	U32 nconstpad = 2*constHeader.constant_count + sizeof(CellCgbLevelAHeader);
	nconstpad = my_align(nconstpad, 16);
	Float4 *pDefaultValues = (Float4*)(pOffset1+nconstpad);

	Btl::Vector<unsigned short> newregisters;
	Btl::Vector<Float4> newvalues;

	U8 *pMappingTable = (pOffset1+nconstpad+sizeof(Float4)*constHeader.constant_count);
	U32 cMappingTable = (U32)((U8*)pBuffer+function->cMicrocode-(U8*)pMappingTable);

	// dump constant table
	unsigned short iNewParamCount = 0;
	for (int i=0; i<constHeader.constant_count; i++)
	{
		unsigned short ir = pConstantRegisters[i];
		Ut::DoEndianSwap::EndianSwap(ir);

		F32 x=pDefaultValues[i].x, y=pDefaultValues[i].y, z=pDefaultValues[i].z, w=pDefaultValues[i].w;
		Ut::DoEndianSwap::EndianSwap((U32&)x);
		Ut::DoEndianSwap::EndianSwap((U32&)y);
		Ut::DoEndianSwap::EndianSwap((U32&)z);
		Ut::DoEndianSwap::EndianSwap((U32&)w);

		// is this the default value for a binding we know of?
		bool bKeep = true;
		for (int b=0; b<function->Bindings.Size(); b++)
		{
			FunctionBinding *pBinding = &function->Bindings[b];
			Parameter *pParam = &pEffect->Parameters[pBinding->ParameterIdx];
			if (ir >= pBinding->Handle && ir < pBinding->Handle + pBinding->Count)
			{
				bKeep = false;
				break;
			}
		}

		if (bKeep)
		{
			newregisters.PushBack(pConstantRegisters[i]);
			newvalues.PushBack(pDefaultValues[i]);
			iNewParamCount++;
		}
	}

	// update new param count and table size
	int nc = iNewParamCount * sizeof(unsigned short);
	int nv = iNewParamCount * sizeof(Float4);
	unsigned short iSize = my_align(nc, 16) + nv + my_align(sizeof(CellCgbLevelAHeader),16);
	pConstHeader->constant_count = iNewParamCount;
	pConstHeader->block_size = iSize;
	Ut::DoEndianSwap::EndianSwap(pConstHeader->constant_count);
	Ut::DoEndianSwap::EndianSwap(pConstHeader->block_size);

	// copy new table over old one
	if (nc) 
	{
		memcpy(pConstantRegisters, &newregisters[0], nc);
	}
	U32 nconstpad2 = 2*iNewParamCount + sizeof(CellCgbLevelAHeader);
	nconstpad2 = my_align(nconstpad2, 16);
	if (nv) 
	{
		memcpy((U8*)pOffset1+nconstpad2, &newvalues[0], nv);
	}

	// move mapping table
	if (cMappingTable)
	{
		memmove((U8*)pOffset1+nconstpad2 + sizeof(Float4)*iNewParamCount, pMappingTable, cMappingTable);
	}

	// calculate new size for whole CGB file
	U32 iTotalSize = cOffset1+nconstpad2+sizeof(Float4)*iNewParamCount+cMappingTable;
	//iTotalSize = my_align(iTotalSize,16);

	// update CGB data on the actual function structure
	BIZ_ASSERT(iTotalSize <= function->cMicrocode);
	function->cMicrocode = iTotalSize;
	memcpy(function->Microcode, pBuffer, iTotalSize);

	delete pBuffer;
}

// Write out compiled microcode and parameter bindings (impelementation specific).
bool CompileShader(bool bPixelShader, Effect *effect, Function *function, const char* fxname, const char* name, const char* target, const char *define, BOOL FastPrecision, Util::C_String cmdFlags)
{
/*
	HANDLE hSemaphore = CreateSemaphore(NULL,1,100,"FXPS3");
	if(!hSemaphore) {
		fprintf(stderr,"ERROR: Failed to create semaphore");
		return false;
	}
	WaitForSingleObject(hSemaphore,INFINITE);
*/
#if 1
	const char* CELL_SDK = getenv("SCE_PS3_ROOT");
	if (!CELL_SDK)
	{
		fprintf( stderr, "ERROR: Need CELL_SDK environment variable to find sce-cgc.\n" );
		function->Microcode = NULL;
		return false;
	}

	char cgc_name[_MAX_PATH];
	
	if (bPixelShader)
	{
		char bin_path[_MAX_PATH];

		GetModuleFileName(NULL, bin_path, _MAX_PATH);

		for (int i=strlen(bin_path)-1; i >= 0; i--)
		{
			if (bin_path[i] == '/' || bin_path[i] == '\\')
			{
				bin_path[i] = 0;
				break;
			}
		}

		snprintf(cgc_name, _MAX_PATH, "%s\\RandomShaderIterator.exe --hash-file shader_hash.txt", bin_path);
	}
	else
	{
		snprintf(cgc_name, _MAX_PATH, "%s\\host-win32\\Cg\\bin\\sce-cgc.exe", CELL_SDK);
	}
#else
	const char* TL = getenv("TL");
	if (!TL)
	{
		fprintf( stderr, "ERROR: Need TL environment variable to find p3x-cgc.exe.\n" );
		function->Microcode = NULL;
		return;
	}

	char cgc_name[_MAX_PATH];
	snprintf(cgc_name, _MAX_PATH, "%s\\ngl\\tools\\bin\\p3x-cgc.exe", TL);
#endif

	Util::C_String bin_name = GetTemporaryFilename();

	char* compile="";

	if (strstr(target, "ps"))
	{
		target = "sce_fp_rsx";
		assert(bPixelShader);
		compile = "__PIXEL__";
	}
	else if (strstr(target, "vs"))
	{
		target = "sce_vp_rsx";
		assert(!bPixelShader);
		compile = "__VERTEX__";
	}

	Util::C_String txt_name = Util::ReplaceFileExtension(bin_name, ".txt");

	//Removed "-firstallocreg 64" so starts at default of 256
	char cmdline[4096];
	if (define)
		snprintf(cmdline, sizeof(cmdline), "%s %s -mcgb -DTL_COMPILER_HLSL=1 -Dshared=\" \" -D%s -O2 -D%s=1 -q %s -unroll all -nobcolor -contalloc -fastmath -fx -entry %s -p %s -o %s -ao %s %s", cgc_name, cmdFlags.Chars(), compile, define,  FastPrecision?"-fastprecision":"-nofastprecision", name, target, bin_name.Chars(), txt_name.Chars(), fxname);
	else
		snprintf(cmdline, sizeof(cmdline), "%s %s -mcgb -DTL_COMPILER_HLSL=1 -Dshared=\" \" -D%s -O2 -fx -q %s -unroll all -nobcolor -contalloc -fastmath -entry %s -p %s -o %s -ao %s %s", cgc_name, cmdFlags.Chars(), compile,  FastPrecision?"-fastprecision":"-nofastprecision", name, target, bin_name.Chars(), txt_name.Chars(), fxname);

	if (system(cmdline))
	{
		fprintf( stderr, "ERROR: sce-cgc execution failed.\n%s\n", cmdline );
		function->Microcode = NULL;
		return false;
	}

	// Read in microcode and add to APK.
	FILE* in = fopen(bin_name,"rb");
	if (!in)
	{
		printf("ERROR: fxc execution failed.\n" );
		function->Microcode = NULL;
		return false;
	}
	fseek(in,0,SEEK_END); int codesize = ftell(in); fseek(in,0,SEEK_SET);

	function->Microcode = new U8[codesize];
	function->cMicrocode = codesize;
	fread(function->Microcode,codesize,1,in);

	fclose(in);

	// Read in listing and generate bindings.
	in = fopen(txt_name,"rb");
	fseek(in,0,SEEK_END); codesize = ftell(in); fseek(in,0,SEEK_SET);

	char* list = (char*)malloc(codesize);
	fread(list,codesize,1,in);

	fclose(in);

	int nBindings = 0;

	const char* ch = list;
	while (*ch)
	{
		char name[LIST_FIELD_MAX], semantic[LIST_FIELD_MAX], used[LIST_FIELD_MAX];
		ListReadField(ch, name); // "//var"
		if (name[0] != '/' || name[1] != '/')
			break;
		ListReadField(ch, semantic); // semantic - interpolators are marked with $vin or $vout
		ListNextField(ch); // hw resource name
		ListNextField(ch); // no idea (0 or -1)
		ListReadField(ch, used); // seems to indicate whether it is used
		
		// in case of arrays, skip all but the [0] element.
		bool keep_array = true;
		if (strchr(name, '[') && !strstr(name, "[0]"))
			keep_array = false;
		
		if (strncmp(name, "//var ", 6) == 0 && strchr(semantic, '$') == 0 && atoi(used) == 1 && keep_array)
		{
//			if (strncmp(name, "//var sampler", 13) == 0)
//				function->NSamplerBindings++;
//			else
			nBindings++;
		}
		else
		{
			// handle vertex input semantics
			if (strncmp(name, "//var ", 6) == 0 && strncmp(semantic, " $vin.", 6) == 0 && atoi(used) == 1 )
			{
				if (!bPixelShader)
					AddVertexInputSemantic(effect, semantic+6);
			}
		}
		ListNextLine(ch);
	}

	function->Bindings.Resize(nBindings);

	int bindingidx = 0;
	int arrayidx = -1;

	ch = list;
	while (*ch)
	{
		char name[LIST_FIELD_MAX], semantic[LIST_FIELD_MAX], used[LIST_FIELD_MAX], reg[LIST_FIELD_MAX];
		ListReadField(ch, name); // "//var"
		if (name[0] != '/' || name[1] != '/')
			break;
		ListReadField(ch, semantic); // semantic - interpolators are marked with $vin or $vout
		ListReadField(ch, reg); // hw resource name
		ListNextField(ch); // no idea (0 or -1)
		ListReadField(ch, used); // seems to indicate whether it is used
		
		// in case of array elements, keep the first and use the rest to update the count.
		bool keep_array = true;
		if (strncmp(name, "//var ", 6) == 0 && strchr(semantic, '$') == 0 && atoi(used) == 1 && strchr(name, '['))
		{ 
			char* brace = strchr(name,'[');
			int element = strtol(brace+1, NULL, 0);
			if (element > 0)
			{
				assert(arrayidx != -1);
				int regcount = 1;
				char* count = strchr(reg, ',');
				if (count)
					regcount = atoi(count+1);
				function->Bindings[arrayidx].Count += regcount;
				keep_array = false;
			}
			else
				arrayidx = bindingidx;		
		}
		
		if (strncmp(name, "//var ", 6) == 0 && strchr(semantic, '$') == 0 && atoi(used) == 1 && keep_array)
		{
			FunctionBinding *binding = &function->Bindings[bindingidx++];

			// Determine the binding type.
			if (strncmp(name, "//var sampler", 13) == 0)
			{
				binding->Type = NGLFX_BIND_SAMPLER;
				// set handle as texture unit index

				if( 0 == strncmp( reg, " texunit", 8  ) )
				{
					int baseUnit = 0;//bPixelShader ? 0 : 16;
					binding->Handle = atoi( reg + 8 ) + baseUnit;
				}
				else
				{
					binding->Handle = GPU_INVALID_SAMPLER;
				}

				binding->Count = 1;
			}
			else
			{
				binding->Type = NGLFX_BIND_FLOAT;

				// Get the register count.
				int regcount = 1;
				char* count = strchr(reg, ',');
				if (count)
					regcount = atoi(count+1);

				// get the start register
				binding->Handle = GPU_INVALID_PARAM;
				char* const_reg_end = strrchr( reg, ']' );

				if( const_reg_end )
				{
					*const_reg_end = 0;
					char* const_reg_start = strrchr( reg, '[' );
					if( const_reg_start && ('c' == *(const_reg_start-1)) )
						binding->Handle = atoi( const_reg_start+1 );
				}

				binding->Count = regcount;
    		}

			// Get the actual name.
			char binding_name[LIST_FIELD_MAX];
			const char* name_ch = name;
			ListNextIdent(name_ch); // skip "//var "
			ListNextIdent(name_ch); // skip type
			ListReadIdent(name_ch, binding_name); // finally the name


			if( bPixelShader && (NGLFX_BIND_SAMPLER != binding->Type) )
			{
				binding->Name = new char [strlen(binding_name)+1];
				strcpy((char*)binding->Name, binding_name);
			}

			// When looking for the parameter, we need to get rid of the array suffix.
			// But, it's neccessary in order to do the binding at runtime, so it is kept above.
			int arrayindex = -1;
			char* brace = strchr(binding_name,'[');
			if (brace)
			{
				*brace++ = 0;
				char* num = brace;
				while( *brace != ']' ) ++brace;
				*brace = 0;

				arrayindex = atoi(num);
			}

			// get structure name
			char* psz = strchr(binding_name, '.');
			Util::C_String structname;
			if (psz)
				structname = Util::C_String(binding_name)(0, psz-binding_name);

			// Find parameter index.
			binding->ParameterIdx = 0xff;
			binding->Offset = 0;

			if( structname.Length()>0 )
			{
				// binding is structure member
				// todo: add support of arrays of structs

				for (int i = 0; i < effect->Parameters.Size(); i++)
				{
					Parameter& param = effect->Parameters[i];
					// match struct
					if ( !strcmp(param.Name, structname.Chars()))
					{
						// find offset in struct
						int offset = GetStructOffset(binding_name);

						if( offset >= 0 )
						{
							//if( arrayindex >= 0 )
							//	tlPrintf("found %s . %s [%d] : %d\n", structname, bindname, arrayindex, offset );
							//else
							//	tlPrintf("found %s . %s : %d\n", structname, bindname, offset );

							binding->Offset = offset;
							binding->ParameterIdx = i;
							break;
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < effect->Parameters.Size(); i++)
				{
					Parameter& param = effect->Parameters[i];
					// check for structs
					if ( !strcmp(param.Name, binding_name))
					{
						binding->ParameterIdx = i;
						break;
					}
				}
			}

			if( binding->ParameterIdx == 0xff )
			{
				printf("ERROR: Unable to bind shader input to a valid parameter!" );
				return false;
			}			
		}
		ListNextLine(ch);
	}

	if (!bPixelShader)
	{
		MergeBindings(effect, function);

		// remove default values from the CGB file's constant table. only leave internal values used for arithmetic or logic 
		StripDefaultValuesFromCGB(effect, function);
	}

	free(list);

//	_unlink(bin_name);
	_unlink(txt_name);

//	ReleaseSemaphore(hSemaphore,1,NULL);
//	CloseHandle(hSemaphore);

	return true;
}

} // namespace FX

ACTRESULT NglPs3Fx::SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char **ppszDefines, const char **ppszIncludesconst, const char *pszOutputFilename )
{
	return FX::SerialOutFx(pStream, fxname, ppszDefines, ppszIncludesconst, pszOutputFilename);
}

