#include "stdafx.h"

#include <windows.h>
#include "d3d9.h"  // Make sure this is the Xenon version.
#include "d3d9types.h"
#include "Xgraphics.h"
#define NGL_TOOL 1

#include "system/scheduler/core/tl_defs.h"

#include "texture_export/Texture.h"
using namespace ActTools;

#define NGL_GPU_D3D 1
#define NGL_GPU_D3D9 1
#define NGL_GPU_D3D_XENON 1
#define NGL_API
#include "texture_export/ngl/dx/ngl_dx_gpu_types.h"

#include "source/fx.cpp"

#include "texture_export/targets/xetex.h"
#include "texture_export/targets/xefx.h"

// ensure we're using the XDK D3D
#if D3D_SDK_VERSION != 0
#pragma error("Using the wrong version of D3D - should be using the XDK version")
#endif


// Write out compiled microcode and parameter bindings (impelementation specific).
static void CompileShader(Effect *effect, Function *function, const char* fxname, const char* name, const char* target, const char* define, BOOL /*FastPrecision*/)
{
	const char* XEDK = getenv("XEDK");
	if (!XEDK)
	{
		LOGERROR( TextureExport::g_Log, "ERROR: Need XEDK environment variable to find fxc.\n" );
		function->Microcode = NULL;
		return;
	}

	char fxc_name[_MAX_PATH];
	_snprintf(fxc_name, _MAX_PATH, "%s\\bin\\win32\\fxc.exe", XEDK);

	char tmp_path[_MAX_PATH];
	GetTempPath(_MAX_PATH, tmp_path);

	char bin_name[_MAX_PATH];
	GetTempFileName(tmp_path, name, 0, bin_name);
	_unlink(bin_name);

	// todo- Introduce an option to allow /Zpc.
	char cmdline[4096];
	char inc[]=".";
	if (define)
		_snprintf(cmdline, sizeof(cmdline), "\"%s\"  /DNGL_XENON=1 /nologo /Zpr /D%s=1 /I%s /XOautoz /T%s /E%s /Fo%s %s", fxc_name, define, inc, target, name, bin_name, fxname);
	else
		_snprintf(cmdline, sizeof(cmdline), "\"%s\"  /DNGL_XENON=1 /nologo /Zpr /I%s /XOautoz /T%s /E%s /Fo%s %s", fxc_name, inc, target, name, bin_name, fxname);
//	printf("%s\n",cmdline);

	if (system(cmdline))
	{
		LOGERROR( TextureExport::g_Log, "fxc execution failed.\n" );
		function->Microcode = NULL;
		return;
	}

	// Read in microcode and add to APK.
	FILE* in = fopen(bin_name,"rb");
	fseek(in,0,SEEK_END); int codesize = ftell(in); fseek(in,0,SEEK_SET);

	function->Microcode = new U8[codesize];
	fread(function->Microcode,codesize,1,in);

	fclose(in);

	// Generate shader constant bindings.
	ID3DXConstantTable* table;
	HRESULT hr = D3DXGetShaderConstantTable((const DWORD*)function->Microcode, &table);
	if (!table)
	{
		LOGERROR( TextureExport::g_Log, "ERROR: Unable to read shader constant table.\n" );
		function->Microcode = NULL;
		return;
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
		binding->ParameterIdx = -1;
		for (int i = 0; i < effect->Parameters.Size(); i++)
			if (effect->Parameters[i].Name == desc.Name)
			{
				binding->ParameterIdx = i;
				binding->Handle = desc.RegisterIndex;
				binding->Count = desc.RegisterCount;
				break;
			}
		assert(binding->ParameterIdx != -1 && "Unable to bind shader input to a valid parameter!");
	}

	_unlink(bin_name);
}

XEFX_API ACTRESULT NglXeFx::SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char* flags )
{
	strcpy(CompileFlags, flags);
	strcat(CompileFlags, " -DNGL_XENON");
	strcat(CompileFlags, " -DTARGET_SET=1 -DTARGET_NGL3_XENON=1");

	ACTRESULT r = ::SerialOutFx(pStream, fxname, flags);

	// Add all the generated procedural textures to the APK file.
//	NglXeTex xetex;
	for (int i = 0; i < NOutputTex; i++)
	{
		assert(0); // TODO: put this back in
//		xetex.SerialOutTexture( pStream, OutputTex[i] );
//		delete OutputTex[i];
	}
	NOutputTex = 0;

	return r;
}

static void FixupParameters(Effect *effect)
{
}
