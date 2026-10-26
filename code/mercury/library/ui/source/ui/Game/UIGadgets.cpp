#include "stdafx.h"

#include "ui/Game/UIGadgets.h"
#include "ui/Graphics/Simple2d.h"

namespace UI
{

UIGadgets gUIGadgets;

float UIGadgets::XCoord_Lua_to_UI(float var)
{
#if 0
	var/=UI_VSCREEN_SX;
	var-=0.5f;
#endif
	return var;
}

float UIGadgets::XCoord_UI_to_Lua(float var)
{
#if 0
	var=((var*UI_VSCREEN_SX)+UI_SCREEN_POS_X)+(UI_VSCREEN_SX/2.0f);
#endif
	return var;
}

float UIGadgets::YCoord_Lua_to_UI(float var)
{
#if 0
	var/=UI_VSCREEN_SY;
	var-=0.5f;
#endif
	return var;
}

float UIGadgets::YCoord_UI_to_Lua(float var)
{
#if 0
	var=((var*UI_VSCREEN_SY)+UI_SCREEN_POS_Y)+(UI_VSCREEN_SY/2.0f);
#endif
	return var;
}

void UIGadgets::Get4VarsFromString(char *c,float *var1,float *var2,float *var3,float *var4)
{
	if(c==NULL) 
	{
		if(var1) *var1=0;
		if(var2) *var2=0;
		if(var3) *var3=0;
		if(var4) *var4=0;
		return;
	}

	char tempc[6];
	int a,b;

	a=b=0;
	while(c[a] != 0x20)
	{
		tempc[b]=c[a];
		a++;
		b++;
	}
	a++;
	tempc[b]=0;
	*var1=float(atof(tempc));
	
	b=0;
	while(c[a] != 0x20)
	{
		tempc[b]=c[a];
		a++;
		b++;
	}
	a++;
	tempc[b]=0;
	*var2=float(atof(tempc));

	b=0;
	while(c[a] != 0x20)
	{
		tempc[b]=c[a];
		a++;
		b++;
	}
	a++;
	tempc[b]=0;
	*var3=float(atof(tempc));

	b=0;
	while(c[a] != 0x20 && c[a])
	{
		tempc[b]=c[a];
		a++;
		b++;
	}
	a++;
	tempc[b]=0;
	*var4=float(atof(tempc));
}

}; // namespace UI
