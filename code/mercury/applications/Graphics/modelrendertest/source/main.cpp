#include "stdafx.h"
#include "modelrenderer.h"

int main()
{
	ModelRendererApp app;

	if (app.Create())
	{
		for (;;)
		{
			app.Update();
			app.Render();
		}
	}

	return 0;
}
