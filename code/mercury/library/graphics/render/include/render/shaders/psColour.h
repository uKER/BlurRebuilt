float Greyscale(float3 colour)
{
	return colour.r * 0.30f + colour.g * 0.59f + colour.b * 0.11f;
}

float3 AdjustSaturation(float3 colour, float amount)
{
	float luminance = 0.2125f * colour.r + 0.7154f * colour.g + 0.0721f * colour.b;
	float3 result;
	result.r = (colour.x - luminance) * amount + luminance;
	result.g = (colour.y - luminance) * amount + luminance;
	result.b = (colour.z - luminance) * amount + luminance;

	return result;
}

float3 AdjustSaturation(float3 vColour, float3 vAmount)
{
	float fLuminance = dot(vColour, float3(0.2125f, 0.7154f, 0.0721f));

	return (vColour - fLuminance) * vAmount + fLuminance;
}

float3 ColourToGreyscale(float3 colour) 
{
	float grey = Greyscale(colour);
	return float3(grey, grey, grey);
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

float3 xyYToXYZ(float3 xyY)
{
	float3 XYZ;
	XYZ.x = (xyY.x / xyY.y) * xyY.z;
	XYZ.y = xyY.z;
	XYZ.z = ((1.0f - xyY.x - xyY.y) / xyY.y) * xyY.z;

	return XYZ;
}

float3 XYZToRGB(float3 XYZ)
{
	float3 RGB;
	RGB.r = dot(XYZ, float3(3.240479f, -1.537150f, -0.498535f));
	RGB.g = dot(XYZ, float3(-0.969256f, 1.875992f, -0.041556f));
	RGB.b = dot(XYZ, float3(0.055648f, -0.204043f, 1.057311f));

	return RGB;
}

float3 xyYToRGB(float3 xyY)
{
	return XYZToRGB(xyYToXYZ(xyY));
}
