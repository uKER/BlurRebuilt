#pragma once

class AlienBrainWrapper
{
public:
	static bool GetLatest(const C_Options& options, const Sys::C_XmlDom& dom, bool doSync);
};
