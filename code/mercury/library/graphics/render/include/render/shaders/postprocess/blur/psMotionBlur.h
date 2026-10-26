
float4x4 g_mMatrix		: register(CCP_RENDER_POSTPROCESS_PARAMETERS_0);
float4 g_vDepthScale	: register(CCP_RENDER_POSTPROCESS_PARAMETERS_4);
float4 g_vStepScale		: register(CCP_RENDER_POSTPROCESS_PARAMETERS_5);
float4 g_vPixelScale	: register(CCP_RENDER_POSTPROCESS_PARAMETERS_6);

float GetDepth(float z)
{
	return g_vDepthScale.x / (g_vDepthScale.y - z);
}

float4 GetStep(float depth, float2 uv)
{
	// get position in uv * depth space
	float4 pos = float4(uv.x * depth, uv.y * depth, depth, 1);
	
	// get previous uvs
	pos = mul(pos, g_mMatrix);	
	// and convert resulting texture * depth coords back to texture coords
	pos.xy /= pos.w;
	
	float2 delta = (pos.xy - uv);
	return delta.xyxy * g_vStepScale;
}

float4 GetSamples(int samples, sampler2D textureMap, float2 startuv, float4 step)
{
	float4 accum = tex2D(textureMap, startuv);

	// get start positions
	float4 uv = startuv.xyxy + step;

	int max = (samples - 1) / 2;

	for ( int i = 0; i < max; i++ )
	{
		float4 tap;
		
		tap = tex2D(textureMap, uv.xy);
		accum += tap;
		
		tap = tex2D(textureMap, uv.zw);
		accum += tap;
		
		uv += step;
	}
	
	return float4(accum.rgb / (1 + max*2), 1.0f);
}

float4 GetSamplesAlphaMask(int samples, sampler2D textureMap, float2 startuv, float4 step)
{
	float4 accum0 = tex2D(textureMap, startuv);
	if(accum0.a < 1.0f)
	{
		float4 tap1 = tex2D(textureMap, startuv + float2(g_vPixelScale.x, 0.0f));
		float4 tap2 = tex2D(textureMap, startuv + float2(0.0f, g_vPixelScale.y));
		float4 tap3 = tex2D(textureMap, startuv + float2(-g_vPixelScale.x, 0.0f));
		float4 tap4 = tex2D(textureMap, startuv + float2(0.0f, -g_vPixelScale.y));
		//float4 tap5 = tex2D(textureMap, startuv + float2(-g_vPixelScale.x, 0.0f));
		//float4 tap6 = tex2D(textureMap, startuv + float2(-g_vPixelScale.x, -g_vPixelScale.y));
		//float4 tap7 = tex2D(textureMap, startuv + float2(0.0f, -g_vPixelScale.y));
		//float4 tap8 = tex2D(textureMap, startuv + float2(g_vPixelScale.x, -g_vPixelScale.y));

		//float tap1ceil = ceil(tap1.a);
		//float tap2ceil = ceil(tap2.a);
		//float tap3ceil = ceil(tap3.a);
		//float tap4ceil = ceil(tap4.a);
		//float tap5ceil = ceil(tap5.a);
		//float tap6ceil = ceil(tap6.a);
		//float tap7ceil = ceil(tap7.a);
		//float tap8ceil = ceil(tap8.a);

		//float weight = 5.0f - (tap1ceil + tap2ceil + tap3ceil + tap4ceil);// + tap5ceil + tap6ceil + tap7ceil + tap8ceil);

		accum0 += tap1;// * (1.0f - tap1ceil);
		accum0 += tap2;// * (1.0f - tap2ceil);
		accum0 += tap3;// * (1.0f - tap3ceil);
		accum0 += tap4;// * (1.0f - tap4ceil);
		//accum0 += tap5 * (1.0f - tap5ceil);
		//accum0 += tap6 * (1.0f - tap6ceil);
		//accum0 += tap7 * (1.0f - tap7ceil);
		//accum0 += tap8 * (1.0f - tap8ceil);
		
		return accum0 / 5.0f;
	}
	else
	{
		accum0.a = 1.0f;
		float4 accum = accum0;	

		// get start positions
		float4 uv = startuv.xyxy + step;

		for ( int i = 0; i < ((samples - 1) / 2); i++ )
		{
			float4 tap;
			
			tap = tex2D(textureMap, uv.xy);
			tap.a = floor(tap.a);
			tap *= tap.a;
			accum += tap;
			
			tap = tex2D(textureMap, uv.zw);

			tap.a = floor(tap.a);
			tap *= tap.a;
			accum += tap;
			
			uv += step;
		}
		
		return float4(accum.rgb / accum.a, 1.0f);
	}
}

