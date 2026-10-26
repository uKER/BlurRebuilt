#ifndef CREATE_FILE_LIST_H
#define CREATE_FILE_LIST_H

#ifdef _MSC_VER
#pragma once
#endif

class C_FontConverter
{
public:
	static bool Process(
		Tools::FileCopy::C_FileCopyList& uberCopyList,
		const C_Options& options);
};

#endif
