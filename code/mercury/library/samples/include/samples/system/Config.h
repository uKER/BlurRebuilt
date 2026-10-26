#ifndef SAMPLES_CONFIG_H
#define SAMPLES_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Samples
{

class C_Config
{
public:

	C_Config();
	~C_Config();

	bool Open(const Util::C_String& fileName);
	void Close();

public:

	std::fstream m_Stream;
};

}; // namespace Samples

#endif // SAMPLES_CONFIG_H
