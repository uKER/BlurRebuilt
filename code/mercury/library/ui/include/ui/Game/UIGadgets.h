#ifndef UIGADGETS_H
#define UIGADGETS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

class UIGadgets
{
public:
	float XCoord_UI_to_Lua(float var);
	float XCoord_Lua_to_UI(float var);

	float YCoord_UI_to_Lua(float var);
	float YCoord_Lua_to_UI(float var);

	void Get4VarsFromString(char *str,float *a,float *b,float *c,float *d);
};

extern UIGadgets gUIGadgets;

}; // namespace UI
#endif // UIGADGETS_H
