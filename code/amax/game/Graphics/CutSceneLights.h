#ifndef AMAX_GRAPHICS_CUTSCENELIGHTS_H
#define AMAX_GRAPHICS_CUTSCENELIGHTS_H

namespace CutScene
{
	MathsStorage::Float4 GetLightColour(const U32 iIndex);
	MathsStorage::Float4 GetLightDirection(const U32 iIndex);
	void SetLight(const U32 iIndex, const MathsStorage::Float3& vDirection, const MathsStorage::Float3& vColour, const F32 fWrap);
}; // namespace CutScene

#endif // AMAX_GRAPHICS_CUTSCENELIGHTS_H