#pragma once


class SubversionWrapper
{
public:
	static bool GetLatest(const C_Options& options, const Tools::Shim::C_SvnFolder& svnFolder);
};
