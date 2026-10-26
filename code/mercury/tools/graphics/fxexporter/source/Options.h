#ifndef OPTIONS_H
#define OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_Options
{
	Util::C_String inFile;
	Util::C_String outFile;
	Util::C_String target;
	Ut::Vector<const char *> defines;
	Ut::Vector<const char *> includes;
};

#endif
