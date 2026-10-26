
float3 PointLight1(float3 worldPosition)
{
	float3 lightColour = g_PointLightColour.xyz;

	float3 lightPosition = g_PointLightPositions[0].xyz;
	float lightFalloffConstant = g_PointLightPositions[0].w;
	
	float3 lightPosToWorldPos = lightPosition - worldPosition;
	
	float lightDistance = dot(lightPosToWorldPos, lightPosToWorldPos);
	float lightFalloff = 1.f - saturate(lightDistance * lightFalloffConstant);

	float3 diffuseLight = lightColour * lightFalloff;
	return diffuseLight;
}

float3 PointLight1(float3 worldPosition, float3 worldNormal)
{
	float3 lightColour = g_PointLightColour.xyz;

	float3 lightPosition = g_PointLightPositions[0].xyz;
	float lightFalloffConstant = g_PointLightPositions[0].w;
	
	float3 lightPosToWorldPos = lightPosition - worldPosition;
	
	float lightDistance = dot(lightPosToWorldPos, lightPosToWorldPos);
	float lightFalloff = 1.f - saturate(lightDistance * lightFalloffConstant);

	float3 lightDirection = normalize(lightPosToWorldPos);
	float NdotL = saturate(dot(lightDirection, worldNormal));

	float3 diffuseLight = lightColour * lightFalloff * NdotL;
	return diffuseLight;
}

float3 PointLight4(float3 worldPosition)
{
	float4 lightFalloffConstants;
	lightFalloffConstants.x = g_PointLightPositions[0].w;
	lightFalloffConstants.y = g_PointLightPositions[1].w;
	lightFalloffConstants.z = g_PointLightPositions[2].w;
	lightFalloffConstants.w = g_PointLightPositions[3].w;
	
	float3 lightPosToWorldPos0 = g_PointLightPositions[0].xyz - worldPosition;
	float3 lightPosToWorldPos1 = g_PointLightPositions[1].xyz - worldPosition;
	float3 lightPosToWorldPos2 = g_PointLightPositions[2].xyz - worldPosition;
	float3 lightPosToWorldPos3 = g_PointLightPositions[3].xyz - worldPosition;
	
	float4 lightDistances;
	lightDistances.x = dot(lightPosToWorldPos0, lightPosToWorldPos0);
	lightDistances.y = dot(lightPosToWorldPos1, lightPosToWorldPos1);
	lightDistances.z = dot(lightPosToWorldPos2, lightPosToWorldPos2);
	lightDistances.w = dot(lightPosToWorldPos3, lightPosToWorldPos3);
	
	float4 lightIntensities = 1.f - saturate(lightDistances * lightFalloffConstants);
	
	// when doing 4 lights each light colour is stored across 3 vectors to reduce instruction count
	float3 diffuse;
	diffuse.r = dot(lightIntensities, g_PointLightColoursRed);
	diffuse.g = dot(lightIntensities, g_PointLightColoursGreen);
	diffuse.b = dot(lightIntensities, g_PointLightColoursBlue);
	
	return diffuse;
}

float3 PointLight4(float3 worldPosition, float3 worldNormal)
{
	float4 lightFalloffConstants;
	lightFalloffConstants.x = g_PointLightPositions[0].w;
	lightFalloffConstants.y = g_PointLightPositions[1].w;
	lightFalloffConstants.z = g_PointLightPositions[2].w;
	lightFalloffConstants.w = g_PointLightPositions[3].w;
	
	float3 lightPosToWorldPos0 = g_PointLightPositions[0].xyz - worldPosition;
	float3 lightPosToWorldPos1 = g_PointLightPositions[1].xyz - worldPosition;
	float3 lightPosToWorldPos2 = g_PointLightPositions[2].xyz - worldPosition;
	float3 lightPosToWorldPos3 = g_PointLightPositions[3].xyz - worldPosition;
	
	float4 lightDistances;
	lightDistances.x = dot(lightPosToWorldPos0, lightPosToWorldPos0);
	lightDistances.y = dot(lightPosToWorldPos1, lightPosToWorldPos1);
	lightDistances.z = dot(lightPosToWorldPos2, lightPosToWorldPos2);
	lightDistances.w = dot(lightPosToWorldPos3, lightPosToWorldPos3);
	
	float4 lightFalloffs = 1.f - saturate(lightDistances * lightFalloffConstants);
	
	float4x3 lightDirections;
	lightDirections[0] = normalize(lightPosToWorldPos0);
	lightDirections[1] = normalize(lightPosToWorldPos1);
	lightDirections[2] = normalize(lightPosToWorldPos2);
	lightDirections[3] = normalize(lightPosToWorldPos3);
	
	float4 NdotL = saturate(mul(lightDirections, worldNormal));
	float4 lightIntensities = NdotL * lightFalloffs;

	float3 diffuse;
	diffuse.r = dot(lightIntensities, g_PointLightColoursRed);
	diffuse.g = dot(lightIntensities, g_PointLightColoursGreen);
	diffuse.b = dot(lightIntensities, g_PointLightColoursBlue);
	
	return diffuse;
}


