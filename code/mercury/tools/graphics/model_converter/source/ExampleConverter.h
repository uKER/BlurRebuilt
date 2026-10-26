#ifndef EXAMPLE_CONVERTER_H
#define EXAMPLE_CONVERTER_H

#ifdef _MSC_VER
#pragma once
#endif

class C_ExampleConverter
{
public:
	static bool Convert(
		C_ExampleExportData& output,
		Aftk::C_Scene& aftkScene,
		const C_Options& options);

private:
};

#endif
