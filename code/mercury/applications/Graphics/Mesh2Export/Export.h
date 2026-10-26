#ifndef EXPORT_H
#define EXPORT_H

#pragma once

struct C_ExportFile
{
	C_ExportFile();
	C_ExportFile(const Util::C_String& sourceFileExt, const Util::C_String& sourceFileDir, const Util::C_String& sourceFileName, const Util::C_String& targetFileDir);

	Util::C_String	m_SourceFileName;
	Util::C_String	m_SourceFilePath;
	Util::C_String	m_TargetFileDir;
};

struct C_ExportList
{
	~C_ExportList();

	void ClearFiles();
	void AddFile(const Util::C_String& sourceFileExt, const Util::C_String& sourceFileDir, const Util::C_String& sourceFileName, const Util::C_String& targetFileDir);

	Btl::Vector<C_ExportFile> m_Files;
};

#endif // EXPORT_H
