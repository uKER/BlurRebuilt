real EvaluateHighlightIntensity(real3 vSample)
{
	return 0.1h * vSample.r * (1.0h + 7.0h * vSample.g);
}

real EvaluateHighlightIntensity(real3 vSample, real highlightPower)
{
	return 0.1h * vSample.r * (highlightPower + 2.0h) * 0.5h;
}

real EvaluateHighlightPower(real3 vSample)
{
#if defined(BLINN)
	return 1.0h + (vSample.g * 1600.0h);
#else
	return 4.0h + 8188.0h * vSample.g * vSample.g;
#endif
}

real EvaluateReflectionIntensity(real3 vSample)
{
	return vSample.r;
}

real EvaluateReflectionPower(real3 vSample)
{
	return 7.0h * (1.0h - vSample.g);
}

real EvaluateFresnelIntensity(real3 vSample)
{
	return vSample.r;
}

real EvaluateFresnelMidBrightness(real3 vSample)
{
	return vSample.b;
}
