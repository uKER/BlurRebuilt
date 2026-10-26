#ifndef OPTIONS_H
#define OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_Options
{
	Util::C_String inFile;
	Util::C_String outFile;
	Util::C_String outFile2;
	Util::C_String format;
	Util::C_String flag;
};

#endif
