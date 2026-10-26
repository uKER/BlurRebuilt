#ifndef SHADER_COLOUROP_H
#define SHADER_COLOUROP_H

real Greyscale(real3 colour)
{
	return dot(colour, real3(0.30h, 0.59h, 0.11h));
}

real3 AdjustSaturation(real3 colour, real amount)
{
	real luminance = 0.2125h * colour.r + 0.7154h * colour.g + 0.0721h * colour.b;
	real3 result;
	result.r = (colour.x - luminance) * amount + luminance;
	result.g = (colour.y - luminance) * amount + luminance;
	result.b = (colour.z - luminance) * amount + luminance;

	return result;
}

real3 AdjustSaturation(real3 vColour, real3 vAmount)
{
	real fLuminance = dot(vColour, real3(0.2125h, 0.7154h, 0.0721h));

	return (vColour - fLuminance) * vAmount + fLuminance;
}

real3 ColourToGreyscale(real3 colour) 
{
	real grey = Greyscale(colour);
	return real3(grey, grey, grey);
}

real3 AdjustBrightness(real3 colour, real amount)
{
	return colour + float3(amount, amount, amount);
}

real3 AdjustContrast(real3 colour, real amount)
{
	real3 output = colour - 0.5h;
	output *= amount;
	output += 0.5h;
	return output;
}

real3 ApplySepia(real3 colour, real amount)
{
	// sepia matrix (values from wikipedia)
	real3x3 sepia;
	sepia[0] = real3(0.393h, 0.349h, 0.272h);
	sepia[1] = real3(0.769h, 0.686h, 0.534h);
	sepia[2] = real3(0.189h, 0.168h, 0.131h);
	
	return lerp(colour, mul(colour, sepia), amount);
}

real3 xyYToXYZ(real3 xyY)
{
	real3 XYZ;
	XYZ.x = (xyY.x / xyY.y) * xyY.z;
	XYZ.y = xyY.z;
	XYZ.z = ((1.0f - xyY.x - xyY.y) / xyY.y) * xyY.z;

	return XYZ;
}

real3 XYZToRGB(real3 XYZ)
{
	real3 RGB;
	RGB.r = dot(XYZ, float3(3.240479f, -1.537150f, -0.498535f));
	RGB.g = dot(XYZ, float3(-0.969256f, 1.875992f, -0.041556f));
	RGB.b = dot(XYZ, float3(0.055648f, -0.204043f, 1.057311f));

	return RGB;
}

real3 xyYToRGB(real xyY)
{
	return XYZToRGB(xyYToXYZ(xyY));
}

#endif
