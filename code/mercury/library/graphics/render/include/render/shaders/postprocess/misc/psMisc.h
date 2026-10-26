float3 ApplyStatic(float3 colour, float2 uv, float amount) 
{
	float noise = uv.x * 128.f + uv.y * 720.f * 128.f;
	noise *= noise;
	noise *= 1.0f / (7907.0f + amount * 109.f);
	noise -= floor(noise);
	float3 vNoise = float3(noise, noise, noise);
	return colour + (vNoise - colour) * amount;
}

float3 AdjustBrightness(float3 colour, float amount)
{
	return colour + float3(amount, amount, amount);
}

float3 AdjustContrast(float3 colour, float amount)
{
	float3 pointFive = float3(0.5f, 0.5f, 0.5f);
	float3 output = colour - pointFive;
	output *= amount;
	output += pointFive;
	return output;
}

float3 ApplySepia(float3 colour, float amount)
{
	// sepia matrix (values from wikipedia)
	float3x3 sepia;
	sepia[0] = float3(0.393f, 0.349f, 0.272f);
	sepia[1] = float3(0.769f, 0.686f, 0.534f);
	sepia[2] = float3(0.189f, 0.168f, 0.131f);
	
	return lerp(colour, mul(colour, sepia), amount);
	
}

