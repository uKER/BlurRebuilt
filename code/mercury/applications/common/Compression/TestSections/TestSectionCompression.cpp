#include "stdafx.h"
#include "TestSectionCompression.h"
#include <Framework/Application/Application.h>
#include <Framework/Application/UserControls.h>
#include <Input/GameInput.h>

#if defined(PLATFORM_PS3)
static Scheduler::Policy g_DeflatePolicy[] = { 1, 1, 0, 0, 0, 0, 0, 0 };
static Scheduler::Policy g_InflatePolicy[] = { 0, 0, 1, 1, 1, 1, 1, 1 };
#else
static Scheduler::Policy g_DeflatePolicy[] = { 1, 1, 1, 1, 1, 1 };
static Scheduler::Policy g_InflatePolicy[] = { 1, 1, 1, 1, 1, 1 };
#endif

C_TestSectionCompression::C_TestSectionCompression(const Util::C_String& name) : Framework::C_TestSection(name)
{
	m_Deflate.SetPolicy(Scheduler::CreatePolicy(g_DeflatePolicy));
	m_Inflate.SetPolicy(Scheduler::CreatePolicy(g_InflatePolicy));

	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(2);

	m_Deflate.Create();
	m_Inflate.Create();
}

C_TestSectionCompression::~C_TestSectionCompression()
{
	m_Deflate.Destroy();
	m_Inflate.Destroy();

	Scheduler::Shutdown();
}

void C_TestSectionCompression::OnUpdate()
{
	if (Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm))
	{
		Deflate();
	}
	else if (Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuCancel))
	{
		Inflate();
	}
}

void C_TestSectionCompression::OnRender2DView()
{
	Display::FontRender()->Draw("Compression", 1024.f/2.f, 768.f/2.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
}

void C_TestSectionCompression::Deflate()
{
	// read a file into memory
	// choose the base skydome texture Sunny.dds
	Sys::I_File* pInFile = Sys::C_FileSystem::Instance().Open("Textures/Sunny.dds");
	if (!pInFile)
		return;

	U32 const nFileSize = pInFile->Length();
	U8* const pFileData = new U8[nFileSize];

	pInFile->Read(pFileData, nFileSize);

	// compress it
	U8* pCompressedData = new U8[nFileSize];
	U32 nCompressedSize = nFileSize;

	m_Deflate.Start(pFileData, nFileSize, pCompressedData, nCompressedSize);
	m_Deflate.Finish(nCompressedSize);

	// write to a file
	Sys::I_File* pOutFile = Sys::C_FileSystem::Instance().Open("User/Sunny.bzp", Sys::E_FileAccess_Write);
	if (pOutFile)
	{
		pOutFile->Write(pCompressedData, nCompressedSize);
	}

	// clean up
	delete pCompressedData;
	delete pFileData;
	delete pOutFile;
	delete pInFile;

	// set the file size
	m_FileSize = nFileSize;
}

void C_TestSectionCompression::Inflate()
{
	// read a file into memory
	// choose the base skydome texture Sunny.dds
	Sys::I_File* pInFile = Sys::C_FileSystem::Instance().Open("User/Sunny.bzp");
	if (!pInFile)
		return;

	U32 const nFileSize = pInFile->Length();
	U8* const pFileData = new U8[nFileSize];

	pInFile->Read(pFileData, nFileSize);

	// decompress it
	U8* pUncompressedData = new U8[nFileSize*10];
	U32 nUncompressedSize = nFileSize*10;

	m_Inflate.Start(pFileData, nFileSize, pUncompressedData, nUncompressedSize);
	m_Inflate.Finish();

	// write to a file
	Sys::I_File* pOutFile = Sys::C_FileSystem::Instance().Open("User/Sunny.dds", Sys::E_FileAccess_Write);
	if (pOutFile)
	{
		pOutFile->Write(pUncompressedData, m_FileSize);
	}

	// clean up
	delete pUncompressedData;
	delete pFileData;
	delete pOutFile;
	delete pInFile;
}
