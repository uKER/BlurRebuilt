#include "StdAfx.h"
#include "Export.h"

C_ExportFile::C_ExportFile()
{
	m_SourceFileName	= "";
	m_SourceFilePath	= "";
	m_TargetFileDir		= "";
}

C_ExportFile::C_ExportFile(const Util::C_String& sourceFileExt, const Util::C_String& sourceFileDir, const Util::C_String& sourceFileName, const Util::C_String& targetFileDir)
{
	m_SourceFileName	= sourceFileName;
	m_SourceFilePath	= sourceFileDir + sourceFileName + "." + sourceFileExt;
	m_TargetFileDir		= targetFileDir;
}

C_ExportList::~C_ExportList()
{
	ClearFiles();
}

void C_ExportList::ClearFiles()
{
	m_Files.Clear();
}

void C_ExportList::AddFile(const Util::C_String& sourceFileExt, const Util::C_String& sourceFileDir, const Util::C_String& sourceFileName, const Util::C_String& targetFileDir)
{
	m_Files.PushBack(C_ExportFile(sourceFileExt, sourceFileDir, sourceFileName, targetFileDir));
}
