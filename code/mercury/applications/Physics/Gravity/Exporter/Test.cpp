#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//This file provides definitions that are required by the Aftk. You could also provide these yourself of course
//as opposed to including this header.
#include <core/core.h>
#include <core/maths.h>
#include "Aftk/Aftk.h"
#include <tools_debug/DebugManager.h>

//This includes the Aftk itself.
#include "Aftk/Aftk.h"


#include "gravity/sharedphysics/sortedarray.h"
#include "gravity/sharedphysics/sortedarray.inl"
#include "gravity/treebuilder/aabbtree.h"
#include "gravity/treebuilder/aabbtree.inl"
#include "system/serialise/C_FileSystem.h"
#include "system/serialise/I_BinaryStream.h"
//#include "gravity/narrowphase/shape.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/shapedescriptors.inl"
#include "gravity/exporter/ExportPrimitivesList.h"
//#include "gravity/exporter/ExportPrimitivesList.inl"
#include "gravity/importer/Importer.h"

#include "ConvertA3D.h"


int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		std::cout <<"Error - please provide a file to read from and write to!" << std::endl;
		return 0;
	}

	char* sourceFile(argv[1]);
	char* destFile(argv[2]);


	Gravity::C_DefaultPolyInform polyInform;
	Gravity::C_DefaultNodeInform nodeInform;
	Gravity::C_Converter convert(&polyInform, &nodeInform);
	
	if(convert.ConvertA3DToGravity(sourceFile, destFile))
		return 0;
	/*if(ConvertA3DToGravity2(sourceFile, destFile))
		return 0;*/
	return -1;
}

