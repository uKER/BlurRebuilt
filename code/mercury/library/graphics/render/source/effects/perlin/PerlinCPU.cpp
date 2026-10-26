#include "stdafx.h"
#include "Render/Effects/Perlin/PerlinCPU.h"

namespace PerlinCPU
{

Debug::Bool g_Show("Graphics.Render.PerlinCPU.Show", false);
Debug::Bool g_Regression("Graphics.Render.PerlinCPU.RegressionTest", false);
Debug::Bool g_Optimized("Graphics.Render.PerlinCPU.UseOptimizedVersion", true);

Debug::Int g_Octaves("Graphics.Render.PerlinCPU.Octaves", 4, Debug::Range(0, 8), Debug::Step(1));
Debug::Float g_AnimationSpeedPerLayer("Graphics.Render.PerlinCPU.PerLayerAnimationSpeed", 2.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.1f));
Debug::Float g_AnimationSpeed("Graphics.Render.PerlinCPU.AnimationSpeed", 1.5f, Debug::Range(0.0f, 10.0f), Debug::Step(0.1f));


C_Perlin::C_Perlin() : Display::I_Renderer("PerlinCPU")
{
	m_fNoiseFalloff = 0.607f;
	m_iCurrentNoise = 0;
	time = 0.0;
}

C_Perlin::~C_Perlin()
{
	DestroyRenderer();
}

bool C_Perlin::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("PerlinCPU");

	for(S32 a = 0; a < NUM_NOISE_TEXTURES; a++)
	{
		if (!Display::Device()->CreateTexture2D(np_size, np_size, 0, Display::E_Format_Linear, Display::E_Format_L16, &m_NoiseTexture[a]))
			return false;

		#ifndef CFG_GOLD
		m_NoiseTexture[a]->SetFileName("_PerlinNoise");
		#endif
	}

	// fill noise with default values
	InitialiseNoise();

	return true;
}

void C_Perlin::OnDestroyVolatile()
{
	RESOURCE_MEMORY_ID("PerlinCPU");

	for(S32 a = 0; a < NUM_NOISE_TEXTURES; a++)
	{
		Display::Device()->Destroy(&m_NoiseTexture[a]);
	}
}

// noise functions...

void C_Perlin::InitialiseNoise()
{
	// create noise (uniform)
	F32* tempnoise = new F32[n_size_sq * noise_frames];
	for(S32 i = 0; i < (n_size_sq * noise_frames); i++)
	{
		F32 temp = (F32) rand()/RAND_MAX;		
		tempnoise[i] = 4 * (temp - 0.5f);	
	}	

	for(S32 frame = 0; frame < noise_frames; frame++)
	{
		for(S32 v = 0; v < n_size; v++)
		{
			for(S32 u = 0; u < n_size; u++)
			{	
				S32 v0 = ((v - 1) & n_size_m1) * n_size,
					v1 = v * n_size,
					v2 = ((v + 1) & n_size_m1) * n_size,
					u0 = ((u - 1) & n_size_m1),
					u1 = u,
					u2 = ((u + 1) & n_size_m1),					
					f  = frame * n_size_sq;
				F32 temp = (1.0f/14.0f) * (	tempnoise[f + v0 + u0] + tempnoise[f + v0 + u1] + tempnoise[f + v0 + u2] +
										tempnoise[f + v1 + u0] + 6.0f*tempnoise[f + v1 + u1] + tempnoise[f + v1 + u2] +
										tempnoise[f + v2 + u0] + tempnoise[f + v2 + u1] + tempnoise[f + v2 + u2]);
									  
				noise[frame * n_size_sq + v * n_size + u] = (S32)(noise_magnitude * temp);
			}
		}
	}	

	delete [] tempnoise;
}

void C_Perlin::UpdateNoise(const F32 fTime)
{
	PERF_EVENT("PerlinCPU::UpdateNoise");

	octaves = g_Octaves;	

	// calculate the strength of each octave
	F32 sum = 0.0f;
	F32 f_multitable[max_octaves];	// aleksger: Moved from class to stack
	for(S32 i = 0; i < octaves; i++)
	{
		f_multitable[i] = powf(m_fNoiseFalloff, 1.0f * i);
		sum += f_multitable[i];
	}

	// aleksger: calculate the inverse of the sum with some other stuff to remove
	// some extra calculation.
	F32 invSum = ((F32)scale_magnitude) / (sum * 1.5f);
	for(int i=0; i<octaves; ++i)
	{
		f_multitable[i] *= invSum;
	}

	F32 fAnimSpeed = g_AnimationSpeed;

	if(fAnimSpeed == 0.0f)
		fAnimSpeed = 0.5f;

	time = fTime * fAnimSpeed;

	F64	r_timemulti = 1.0;
	// aleksger: Moved out of the loop
	U32 image[3];
	S32 amount[3], iImage;
	F64 dImage, fraction;

	// aleksger, mruete: zero out all of o_noise since we're going to completely overwrite it.
	// saves some memory reads.
	// NOTE: It's possible we might be doing too much at once up front.

	for(S32 o = 0; o < octaves; o++)
	{
		fraction = modf(time*r_timemulti,&dImage);
		iImage = (S32)dImage;
		
		amount[0] = (S32)(f_multitable[o] * pow(sin((fraction + 2) * Maths::Pi/3), 2));
		amount[1] = (S32)(f_multitable[o] * pow(sin((fraction + 1) * Maths::Pi/3), 2));
		amount[2] = (S32)(f_multitable[o] * pow(sin((fraction) * Maths::Pi/3), 2));
		image[0] = (iImage) & noise_frames_m1;
		image[1] = (iImage + 1) & noise_frames_m1;
		image[2] = (iImage + 2) & noise_frames_m1;
		{	
			for(S32 i = 0; i < n_size_sq; i++)
			{
				o_noise[i + n_size_sq*o] =	(	((amount[0] * noise[i + n_size_sq * image[0]]) >> scale_decimalbits) + 
					((amount[1] * noise[i + n_size_sq * image[1]]) >> scale_decimalbits) + 
					((amount[2] * noise[i + n_size_sq * image[2]]) >> scale_decimalbits));
			}
		}

		r_timemulti *= g_AnimationSpeedPerLayer;
	}

	// mruete, aleksger: New optimized code.
	if(g_Optimized)
	{
		S32 index = 0,  vmask;
		S32 uStart, vStart;

		// NOTE: We took out the outer octave loop, which means this code is now hard-coded for 4 octaves.
		// It shouldn't be too hard to add the loop back in and change octaves 0,1,2,3 to o, o+1, o+2, and o+3.
		// But we can't see having the performance overhead for that, and haven't tested it that way.

		// Use the data cache block zero intrinsic since we're over-writing all of p_noise.
		// NOTE: It's possible we might be doing too much at once up front.

		// The inner set of loops samples a single 8x8 block of p_noise.
		// So this loop steps by 8 rows.
		for ( vStart = 0; vStart <= np_size-8; vStart += 8 )
		{
			// This loop steps by 8 columns
			for ( uStart = 0; uStart <= np_size-8; uStart += 8)
			{
				// Analysis of the original mapsample function shows that each 8x8 block of p_noise
				// uses the same 4 samples from octave 0 of o_noise, so read them once and store them.
				o_noise_samples samp_oct_0((vStart>>3)&n_size_m1, 
					(uStart>>3)&n_size_m1,
					((vStart>>3)+1)&n_size_m1, 
					((uStart>>3)+1)&n_size_m1,
					o_noise);

				// This loop steps through the rows of a single 8x8 block.
				// This loop is counted using the fv variable that used to be computed inside
				// the original call to "mapsample( u, v, 3, o);".  Analysis of values from
				// one call to the next showed that it incremented by one for each call, in the
				// range 0..7, so it made a perfect loop counter.
				for (S32 fv_0 = 0; fv_0 < 8; ++fv_0)
				{ 
					// Index of beginning of current line within the 8x8 block.  Precompute here
					// instead of repeatedly computing it in the inner loop.
					index = (vStart+fv_0) * np_size + uStart;
					// Take a bunch of processing out of the inner loop.
					vmask = ((vStart+fv_0)&n_size_m1)*n_size + 3 * n_size_sq;
 					
 					// This loop steps through 8 samples on a single row of the 8x8 block.
 					// fu_0 is its loop counter, but it's incremented in the inner most loop.
 					// That means this loop only executes twice.
 					for (S32 fu_0 = 0; fu_0 <8;)
					{
						// Analysis shows that these samples don't change for 4 p_noise samples, so cache them
						o_noise_samples samp_oct_1(((vStart+fv_0)>>2)&n_size_m1, 
							((uStart+fu_0)>>2)&n_size_m1,
							(((vStart+fv_0)>>2)+1)&n_size_m1, 
							(((uStart+fu_0)>>2)+1)&n_size_m1,
							o_noise+n_size_sq*1);

						// This loop steps through the first 4 samples on a single row.
						// fu_1 is its loop counter, but it's incremented in the inner most loop,
						// so this loop will only execute twice.
						for (S32 fu_1 = 0; fu_1<4;)
						{
							// Analysis shows that these samples don't change for 2 p_noise samples, so cache them
							o_noise_samples samp_oct_2(((vStart+fv_0)>>1)&n_size_m1, 
								((uStart+fu_0)>>1)&n_size_m1,
								(((vStart+fv_0)>>1)+1)&n_size_m1, 
								(((uStart+fu_0)>>1)+1)&n_size_m1,
								o_noise+n_size_sq*2);
								
							// This loop steps through 2 samples in a single row. We assume the compiler
							// is unrolling it (haven't verified, though).  It increments all the fu_* variables
							// here, as well as the index in p_noise being written to.
							for (S32 fu_2 = 0; fu_2<2; ++fu_2,++fu_1,++fu_0, ++index)
							{
								p_noise[index] = o_noise[vmask + ((uStart+fu_0)&n_size_m1)]
									+ mapsample_power<3>(samp_oct_0,fu_0, fv_0)
									+ mapsample_power<2>(samp_oct_1,fu_1, fv_0 & 3)
									+ mapsample_power<1>(samp_oct_2,fu_2, fv_0 & 1);
							}
						}
					}
				}
			}
		}
	
		// consistency check.  Uses old code to verify that the new code generated correct p_noise samples.
		// Once the check fails, we'll stop checking, so the assert can only fire once.
		static bool bCheckFailed = false;
		if ( g_Regression && !bCheckFailed )
		{
			S32 octavepack = 0;
			S32 chk;
			for(S32 o=0; o<octaves; o+=n_packsize)
			{
				for(S32 v=0; v<np_size; v++)
					for(S32 u=0; u<np_size; u++)
					{
						chk = 	o_noise[(o+3)*n_size_sq + (v&n_size_m1)*n_size + (u&n_size_m1)];
						chk += mapsample( u, v, 3, o);
						chk += mapsample( u, v, 2, o+1);
						chk += mapsample( u, v, 1, o+2);	
						if (chk != p_noise[v*np_size+u+octavepack*np_size_sq])
						{
							assert(0 && "Noise sampling consistency check failed.  This is safely continuable.");
							goto bail;
						}
					}
					++octavepack;
			}
			bail:;	// Check failed
		}
	}
	else
	{
		// ORIGINAL

		S32 octavepack = 0;
		for(S32 o=0; o<octaves; o+=n_packsize)
		{
			for(S32 v=0; v<np_size; v++)
				for(S32 u=0; u<np_size; u++)
				{
					p_noise[v*np_size+u+octavepack*np_size_sq] = o_noise[(o+3)*n_size_sq + (v&n_size_m1)*n_size + (u&n_size_m1)];
					p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 3, o);
					p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 2, o+1);
					p_noise[v*np_size+u+octavepack*np_size_sq] += mapsample( u, v, 1, o+2);	
				}
				++octavepack;
		}
	}
}

void C_Perlin::UploadNoise()
{
	PERF_EVENT("PerlinCPU::UploadNoise");

	Display::C_LockedRect locked;
	U16 *data;
	S32 *tempdata = (S32*) alloca(np_size_sq * sizeof(S32));

	if (!tempdata)
	{
		return;
	}

	m_iCurrentNoise += 1;
	if(m_iCurrentNoise >= NUM_NOISE_TEXTURES)
		m_iCurrentNoise = 0;

	S32 offset = np_size_sq * 0;

	if(m_NoiseTexture[m_iCurrentNoise].IsCreated())
	{
		locked = m_NoiseTexture[m_iCurrentNoise].Lock(0);
		if(locked.pBits)
		{
			data = (U16*)locked.pBits;
			for(S32 i = 0; i < np_size_sq; i++)
				data[i] = 32768 + p_noise[i + offset];

			m_NoiseTexture[m_iCurrentNoise].Unlock(0);
			
			S32 c = m_NoiseTexture[m_iCurrentNoise].GetLevels();

			// calculate the second level, and upload it
			locked = m_NoiseTexture[m_iCurrentNoise].Lock(1);
			
			data = (U16*)locked.pBits;		
			S32 pitch1 = (locked.pitch) >>1 ;
			S32 sz = np_size >> 1;
			for(S32 v = 0; v < sz; v++)
			{
				for(S32 u = 0; u < sz; u++)
				{				
					tempdata[v * np_size + u] = (p_noise[((v << 1)) * np_size + (u << 1)+offset] + p_noise[((v << 1)) * np_size + (u << 1) + 1 + offset] +
											   p_noise[((v << 1) + 1) * np_size + (u << 1)+offset] + p_noise[((v << 1)+1) * np_size + (u << 1) + 1 + offset]) >> 2;
					data[v * pitch1 + u] = 32768 + tempdata[v * np_size + u];
				}
			}

			m_NoiseTexture[m_iCurrentNoise].Unlock(1);

			for(S32 j = 2; j < c; j++)
			{
				locked = m_NoiseTexture[m_iCurrentNoise].Lock(j);
				data = (U16*)locked.pBits;
				S32 pitch2 = (locked.pitch) >> 1;
				sz = np_size >> j;			
				for(S32 v=0; v<sz; v++)
				{
					for(S32 u=0; u<sz; u++)
					{
						tempdata[v*np_size + u] =	(tempdata[((v<<1))*np_size + (u<<1)] + tempdata[((v<<1))*np_size + (u<<1) + 1] +
													tempdata[((v<<1)+1)*np_size + (u<<1)] + tempdata[((v<<1)+1)*np_size + (u<<1) + 1])>>2;
						data[v*pitch2+u] = 32768+tempdata[v*np_size + u];
					}
				}		
				m_NoiseTexture[m_iCurrentNoise].Unlock(j);
			}
		}
	}
}

void C_Perlin::Update(const F32 fTime)
{
	UpdateNoise(fTime);
	UploadNoise();
}

void C_Perlin::Render() const
{
}

void C_Perlin::Show() const
{
	if(g_Show)
	{
		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Quad::DrawFullScreen(&m_NoiseTexture[m_iCurrentNoise]);
	}
}

} // namespace Perlin
