#ifndef SPEED_TREE_H
#define SPEED_TREE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
}

class C_DataConversionOptions;

bool ExecuteSpeedTree( Aftk::C_Scene& aftk, const C_DataConversionOptions& conversionOptions );

#endif




