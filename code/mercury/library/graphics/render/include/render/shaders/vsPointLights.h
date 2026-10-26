
float4	 g_PointLightPositions[4]	: register(CCV_RENDER_LIGHT_POSITIONS);
float4	 g_PointLightColour			: register(CCV_RENDER_LIGHT_COLOUR);
float4	 g_PointLightColoursRed		: register(CCV_RENDER_LIGHTS_COLOURS_RED);
float4	 g_PointLightColoursGreen	: register(CCV_RENDER_LIGHTS_COLOURS_GREEN);
float4	 g_PointLightColoursBlue	: register(CCV_RENDER_LIGHTS_COLOURS_BLUE);

#include "PointLights.h"



