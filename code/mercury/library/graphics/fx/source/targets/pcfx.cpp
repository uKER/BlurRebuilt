#include "stdafx.h"

#include <windows.h>
#include "d3d9.h"  // Make sure this is the PC version.
#include "d3d9types.h"

#include <core/core.h>
#include <core/containers/string.h>
#include <system/scheduler/core/tl_defs.h>
#include <system/serialise/I_BinaryStream.h>
#include <system/serialise/BinarySerialise.h>
#include <core/utility/endian.h>

//#include "ActToolsLib/ActToolsLib.h"
//#include "ActToolsLib/graphics/texture/Texture.h"
//using namespace ActTools;

#define NGL_GPU_D3D9 1							
#define NGL_API	
#define NGL_GPU_D3D_PC 1
#include "ngl/mercury_to_ngl.h"
#include "ngl/defs.h"
#include "ngl/system.h"
#include "ngl/dx/ngl_dx_gpu_types.h"

#include "source/fx_export.cpp"

#include "fx/targets/pcfx.h"

namespace FX
{

const char *g_pszPlatform = "PLATFORM_PC";

// Write out compiled microcode and parameter bindings (impelementation specific).
static bool CompileShader(bool bPixelShader, Effect *effect, Function *function, const char* fxname, const char* name, const char* target, const char* define, BOOL /*FastPrecision*/, Util::C_String cmdFlags)
{
	const char* DXSDK_DIR = getenv("DXSDK_DIR");
	if (!DXSDK_DIR)
	{
		printf( "ERROR: Need DXSDK_DIR environment variable to find fxc.\n" );
		function->Microcode = NULL;
		return false;
	}

	char fxc_name[_MAX_PATH];
	_snprintf(fxc_name, _MAX_PATH, "%s\\utilities\\bin\\x86\\fxc.exe", DXSDK_DIR);

	Util::C_String bin_name = GetTemporaryFilename();

	// todo- Introduce an option to allow /Zpc.
	char cmdline[4096];
	if (define)
		_snprintf(cmdline, sizeof(cmdline), " %s /D=%s /nologo /Zpr /T%s /E%s /Fo%s %s", cmdFlags.Chars(), define, target, name, bin_name.Chars(), fxname);
	else
		_snprintf(cmdline, sizeof(cmdline), " %s /nologo /Zpr /T%s /E%s /Fo%s %s", cmdFlags.Chars(), target, name, bin_name.Chars(), fxname);

	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi = { 0 };
	CreateProcess(fxc_name, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD code;
	GetExitCodeProcess(pi.hProcess, &code);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	if (code)
	{
		printf("ERROR: fxc execution failed.\n" );
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

	// Generate shader constant bindings.
	ID3DXConstantTable* table;
	D3DXGetShaderConstantTable((const DWORD*)function->Microcode, &table);
	if (!table)
	{
		printf("ERROR: Unable to read shader constant table.\n" );
		function->Microcode = NULL;
		return false;
	}

	// Get Vertex Shader input semantics
	if (!bPixelShader)
	{
		UINT nSemantics=0;
		D3DXGetShaderInputSemantics((const DWORD*)function->Microcode, NULL, &nSemantics);
		D3DXSEMANTIC *pSemantics = new D3DXSEMANTIC[nSemantics];
		D3DXGetShaderInputSemantics((const DWORD*)function->Microcode, pSemantics, &nSemantics);
		for (UINT i=0; i<nSemantics; i++)
		{
			// our output doesn't have POSITIONT, so compensate for that
			if (pSemantics[i].Usage >= D3DDECLUSAGE_COLOR && D3DDECLUSAGE_COLOR > 9)
				pSemantics[i].Usage -= D3DDECLUSAGE_COLOR - 9;

			AddVertexInputSemantic(effect, pSemantics[i].Usage, pSemantics[i].UsageIndex);
		}
		delete [] pSemantics;
	}

	//
	D3DXCONSTANTTABLE_DESC tabledesc;
	table->GetDesc(&tabledesc);
	function->Bindings.Resize(tabledesc.Constants);

	for (UINT i = 0; i < tabledesc.Constants; i++)
	{
		D3DXCONSTANT_DESC desc;
		UINT count = 1;
		table->GetConstantDesc(table->GetConstant(NULL, i), &desc, &count);

		FunctionBinding *binding = &function->Bindings[i];

		switch (desc.RegisterSet)
		{
		case D3DXRS_BOOL: binding->Type = NGLFX_BIND_BOOL; break;
		case D3DXRS_INT4: binding->Type = NGLFX_BIND_INT; break;
		case D3DXRS_FLOAT4: binding->Type = NGLFX_BIND_FLOAT; break;
		case D3DXRS_SAMPLER: binding->Type = NGLFX_BIND_SAMPLER; break;
		}

		// Find parameter index.
		binding->ParameterIdx = 0xff;
		binding->Offset = 0;
		for (int i = 0; i < effect->Parameters.Size(); i++)
			if (effect->Parameters[i].Name == desc.Name)
			{
				binding->ParameterIdx = i;
				binding->Handle = desc.RegisterIndex;
				binding->Count = desc.RegisterCount;
				break;
			}
		assert(binding->ParameterIdx != 0xff && "Unable to bind shader input to a valid parameter!");
	}

	_unlink(bin_name);

	return true;
}

static void FixupParameters(Effect *effect)
{
}

} // namespace FX

PCFX_API ACTRESULT NglPcFx::SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char **ppszDefines, const char **ppszIncludes, const char *pszOutputFilename )
{
	return FX::SerialOutFx(pStream, fxname, ppszDefines, ppszIncludes, pszOutputFilename);
}

