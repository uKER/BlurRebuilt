#ifndef PROCESS_AFTKCREATELODSPRITES_H
#define PROCESS_AFTKCREATELODSPRITES_H

#ifdef _MSC_VER
#pragma once
#endif

class C_DataConversionOptions;

namespace CreateLodImposters
{
	bool Process(Aftk::C_Scene& scene, const C_DataConversionOptions& conversionOptions);
};

#endif