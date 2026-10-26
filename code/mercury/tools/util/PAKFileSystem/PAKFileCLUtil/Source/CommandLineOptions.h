#ifndef COMMAND_LINE_OPTIONS_H
#define COMMAND_LINE_OPTIONS_H

struct CommandLineOptions
{
	Util::C_String xmlFile;
	Util::C_String root;
	Util::C_String pak;
	Util::C_String mountPoint;
	bool compress;
	bool endianSwap;
	bool parseXml;
	U32 sectorSize;
};

#endif // COMMAND_LINE_OPTIONS_H
