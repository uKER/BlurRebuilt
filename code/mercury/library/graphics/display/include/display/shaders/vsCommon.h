float4 TransformPosition2D(float2 position, float4 transform)
{
	return float4((position * transform.xy) + transform.zw, 1.f, 1.f);
}
