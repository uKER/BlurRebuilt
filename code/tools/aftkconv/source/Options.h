#ifndef OPTIONS_H
#define OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

enum E_ReturnCode
{
	E_ReturnCode_Success = 0,
	E_ReturnCode_Warning = 1,
	E_ReturnCode_Fatal = 2,
	E_ReturnCode_CmdParseErr = 7,
	E_ReturnCode_MetaMismatch = 254,
	E_ReturnCode_Cancel = 255
};

#define DEF(e, str) e,
enum E_Command
{
	#include "Command.inl"
	E_Command_Count
};
#undef DEF

struct C_Options
{
	C_Options() {}

	E_Command						command;
	Btl::Vector<Sys::C_FilePath>	inputs;
	Sys::C_FilePath					output;
	bool							quiet;
	bool							forceConversion;
	Btl::String						inputArgs;
	Btl::String						outputArgs;
	
	//Conversion options
	Sys::C_FilePath					conversionInput;

	//Merge options
	Sys::C_FilePath					mergeWorkFile;
};

const char* GetCommandStr(E_Command cmd);

E_Command GetCommand(const char* cmd);

#endif
