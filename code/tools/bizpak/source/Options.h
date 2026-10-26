#ifndef OPTIONS_H
#define OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <PakFs/Impl/PakDefinitions.h>
#include <PakFs/Impl/PakBuildHelpers.h>

enum E_ReturnCode
{
	E_ReturnCode_Success = 0,
	E_ReturnCode_Warning = 1,
	E_ReturnCode_Fatal = 2,
	E_ReturnCode_CmdParseErr = 7,
	E_ReturnCode_Cancel = 255
};

#define DEF(e, str) e,
enum E_Command
{
	#include "Command.inl"
	E_Command_Count
};
#undef DEF

#define DEF(e, str) e,
enum E_CompressType
{
	#include "CompressTypes.inl"
	E_CompressType_Count
};
#undef DEF

struct C_Switches
{
	E_CompressType		compressType;
	bool				recurseSubdirectories;
	bool				bigEndian;
	bool				secure;
	bool				quiet;
	bool				timeOperation;
	Pak::E_UpdateType	updateType;
	U32					sectorSize;
	Btl::String			metadata;

	//Location for extracted files.
	Sys::C_FilePath	outputDirectory;
};

struct C_Options
{
	C_Options() : 
		command(E_Command_Unknown) {}

	E_Command		command;
	//Filepath of pak file.
	Sys::C_FilePath	pakFilePath;
	Sys::C_FilePath	pakRootPath;
	Pak::C_FilePathsToProcess toProcess;
	C_Switches		switches;
	char			pathSeparator;
};

const char* GetCommandStr(E_Command cmd);

const char* GetCompressTypeStr(E_CompressType type);

E_Command GetCommand(const char* cmd);

E_CompressType GetCompressType(const char* type);

#endif
