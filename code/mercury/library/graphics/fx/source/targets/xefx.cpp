#include "stdafx.h"

#include <windows.h>
#include "d3d9.h"  // Make sure this is the Xenon version.
#include "d3d9types.h"
#include "Xgraphics.h"
#define NGL_TOOL 1

#include "system/scheduler/core/tl_defs.h"

#define NGL_GPU_D3D 1
#define NGL_GPU_D3D9 1
#define NGL_GPU_D3D_XENON 1
#define NGL_API
#include "ngl/mercury_to_ngl.h"
#include "ngl/defs.h"
#include "ngl/system.h"
#include "ngl/dx/ngl_dx_gpu_types.h"

#include "source/fx_export.cpp"

#include "fx/targets/xefx.h"

// ensure we're using the XDK D3D
#if D3D_SDK_VERSION != 0
#pragma error("Using the wrong version of D3D - should be using the XDK version")
#endif

namespace FX
{

const char *g_pszPlatform = "PLATFORM_XENON";

// Write out compiled microcode and parameter bindings (impelementation specific).
static bool CompileShader(bool bPixelShader, Effect *effect, Function *function, const char* fxname, const char* name, const char* target, const char* define, BOOL /*FastPrecision*/, Util::C_String cmdFlags)
{
	const char* XEDK = getenv("XEDK");
	if (!XEDK)
	{
		printf("ERROR: Need XEDK environment variable to find fxc.\n" );
		function->Microcode = NULL;
		return false;
	}

	char fxc_name[_MAX_PATH];
	_snprintf(fxc_name, _MAX_PATH, "%s\\bin\\win32\\fxc.exe", XEDK);

	Util::C_String bin_name = GetTemporaryFilename();

	// todo- Introduce an option to allow /Zpc.
	char cmdline[4096];
	if (define)
		_snprintf(cmdline, sizeof(cmdline), " /nologo %s /Zpr /XZi /D%s=1 /XOautoz /T%s /E%s /Fo%s %s", cmdFlags.Chars(), define, target, name, bin_name.Chars(), fxname);
	else
		_snprintf(cmdline, sizeof(cmdline), " /nologo %s /Zpr /XZi /XOautoz /T%s /E%s /Fo%s %s", cmdFlags.Chars(), target, name, bin_name.Chars(), fxname);
//	printf("%s\n",cmdline);

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
	HRESULT hr = D3DXGetShaderConstantTable((const DWORD*)function->Microcode, &table);
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
			AddVertexInputSemantic(effect, pSemantics[i].Usage, pSemantics[i].UsageIndex);
		delete [] pSemantics;
	}

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

		// Get the actual name.
//		binding->Name = (const char*)(void*)entry->AppendData(image_index, NULL, tl_align(strlen(desc.Name)+1, 4), 4);
//		entry->AddFixup(&binding->Name, (void*)binding->Name);
//		strcpy((char*)binding->Name, desc.Name);

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
		// if it asserts here and the parameter starts with a '$', that means it's a 'uniform' variable, which is not supported
		assert(binding->ParameterIdx != 0xff && "Unable to bind shader input to a valid parameter!");
	}

	_unlink(bin_name);

	return true;
}

static void FixupParameters(Effect *effect)
{
}

} // namespace FX

XEFX_API ACTRESULT NglXeFx::SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char **ppszDefines, const char **ppszIncludes, const char *pszOutputFilename )
{
	return FX::SerialOutFx(pStream, fxname, ppszDefines, ppszIncludes, pszOutputFilename);
}

