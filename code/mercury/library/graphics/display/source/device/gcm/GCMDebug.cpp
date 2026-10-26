#include "stdafx.h"
#include "Display/Device/GCM/GCMDebug.h"

namespace Display
{
namespace GCM
{

void DebugOutput(CellCgbProgram* pProgram)
{
	if(!pProgram)
		return;

	LOGINFO(g_Log, "CellCgbProgram : ");
	for(U32 i=0; i<CELL_CGB_PROGRAM_STRUCTURE_SIZE; i++)
	{
		LOGINFO(g_Log, "%c", pProgram->data[i]);
	}
	LOGINFO(g_Log, "\n");
}

void DebugOutput(CellCgbVertexProgramConfiguration* pConfig)
{
	if(!pConfig)
		return;

	LOGINFO(g_Log, "CellCgbVertexProgramConfiguration : instructionSlot '%i' instructionCount '%i' attributeInputMask '%i' registerCount '%i'\n",
		pConfig->instructionSlot,
		pConfig->instructionCount,
		pConfig->attributeInputMask,
		pConfig->registerCount);
}

void DebugOutput(CellCgbFragmentProgramConfiguration* pConfig)
{
	if(!pConfig)
		return;

	LOGINFO(g_Log, "CellCgbVertexProgramConfiguration : offset '%i' attributeInputMask '%i' texCoordsInputMask '%i' texCoords2D '%i' texCoordsCentroid '%i' fragmentControl '%i' registerCount '%i'\n",
		pConfig->offset,
		pConfig->attributeInputMask,
		pConfig->texCoordsInputMask,
		pConfig->texCoords2D,
		pConfig->texCoordsCentroid,
		pConfig->fragmentControl,
		pConfig->registerCount);
}

}; // namespace GCM
}; // namespace Display
