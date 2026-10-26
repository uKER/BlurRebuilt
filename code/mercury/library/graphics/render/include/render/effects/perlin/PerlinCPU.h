#ifndef RENDER_PERLINCPU_H
#define RENDER_PERLINCPU_H

#ifdef _MSC_VER
#pragma once
#endif


namespace PerlinCPU
{

const S32 n_bits			= 5;
const S32 n_size			= (1<<(n_bits-1));
const S32 n_size_m1			= (n_size - 1);
const S32 n_size_sq			= (n_size*n_size);
const S32 n_size_sq_m1		= (n_size_sq - 1);


const S32 n_packsize		= 4;

const S32 np_bits			= (n_bits+n_packsize-1);
const S32 np_size			= (1<<(np_bits-1));
const S32 np_size_m1		= (np_size-1);
const S32 np_size_sq		= (np_size*np_size);
const S32 np_size_sq_m1		= (np_size_sq-1);

const S32 n_dec_bits		= 12;
const S32 n_dec_magn		= 4096;
const S32 n_dec_magn_m1		= 4095;

const S32 max_octaves		= 32;

const S32 noise_frames		= 8;
const S32 noise_frames_m1	= (noise_frames-1);

const S32 noise_decimalbits	= 15;
const S32 noise_magnitude	= (1<<(noise_decimalbits-1));

const S32 scale_decimalbits	= 15;
const S32 scale_magnitude	= (1<<(scale_decimalbits-1));

const S32 NUM_NOISE_TEXTURES = 2;

class C_Perlin : public Display::I_Renderer
{
public:
	C_Perlin();
	~C_Perlin();

	bool	OnCreateVolatile();
	void	OnDestroyVolatile();

	void	Update(const F32 fTime);
	void	Show() const;
	void	Render() const;

	const Display::C_Texture2D*		GetTexture() const { return &m_NoiseTexture[m_iCurrentNoise]; }

private:
	void	InitialiseNoise();
	void	UpdateNoise(const F32 fTime);
	void	UploadNoise();

	Display::C_Texture2D			m_NoiseTexture[NUM_NOISE_TEXTURES];
	S32								m_iCurrentNoise;
	
	F32								m_fNoiseFalloff;

	// noise stuff....
	// mruete: these need to be 128-byte aligned so we can use _dcbz on them.
	// also consider reducing max_octaves to 4, since that's all that's ever used.
	// That would recover some space.  Also, if they were made small enough,
	// it might be possible to move o_noise to the stack in CalculateNoise, which
	// could help things a bit.
	__declspec(align(128)) S32		noise[n_size_sq*noise_frames];
	__declspec(align(128)) S32		o_noise[n_size_sq*max_octaves];
	__declspec(align(128)) S32		p_noise[np_size_sq*(max_octaves>>(n_packsize-1))];	
	S32*							r_noise;
	S32								octaves;
	F64								time;

	// aleksger - removed f_multitable and multitable as they weren't really used elsewhere.
	
	// mruete: o_noise_samples is designed to take the 4 samples from a
	// single octave of o_noise required for mapsample.  Storing them in a structure,
	// and sampling them in the constructor makes the code in CalculateNoise cleaner
	// and easier to understand.  The samples are re-used across multiple calls to mapsample,
	// (and multiple writes to p_noise).  This drastically reduces redundant indexing work
	// and memory reads from o_noise.  See comments in CalculateNoise.
	struct o_noise_samples
	{
		// These are named kind of backwards to maintain consistency with the rest of
		// the code that actually uses v before u.  Just makes it easier to read
		// and keep track of.
		S32 o_vu;	// sample at u, v
		S32 o_vu1;	// sample at u+1, v
		S32 o_v1u;	// sample at u, v+1
		S32 o_v1u1; // sample at u+1, v+1
		
		// This function requires that the caller ensure that the sample coords are
		// in-bounds
		__forceinline o_noise_samples( S32 v, S32 u, S32 v1, S32 u1, S32 o_noise[] ) :
			o_vu(   o_noise[v*n_size+u] ),
			o_vu1(  o_noise[v*n_size+u1] ),
			o_v1u(  o_noise[v1*n_size+u] ),
			o_v1u1( o_noise[v1*n_size+u1] )
		{
	#ifdef _DEBUG
			// Consistency checks.  Unfortunately, we've already messed up if they fail.
			// This is just for uber-safety, though.
			assert( u >= 0 && u < n_size );
			assert( v >= 0 && u < n_size );
			assert( u1 >= 0 && u < n_size );
			assert( v1 >= 0 && u < n_size );
	#endif // _DEBUG
		}
	};

	// mruete, aleksger: New templated, inlined mapsample replacement.
	// Takes the o_noise samples and fu and fv as parameters so they
	// can be calculated more efficiently in the caller.
	template<S32 power>
	__forceinline S32 mapsample_power(const o_noise_samples &s, S32 fu, S32 fv)
	{
		const S32 magnitude = 1<<power;
		return ((magnitude-fu)*	(magnitude-fv)	* s.o_vu +
				fu *	(magnitude-fv)			* s.o_vu1 + 
				(magnitude-fu)*	fv				* s.o_v1u +
				fu	*	fv						* s.o_v1u1)
				>>(power+power);
	}

	// original (slow) mapsample function
	__forceinline S32 mapsample(S32 u, S32 v, S32 upsamplepower, S32 octave)
	{
		S32 magnitude = 1<<upsamplepower;
		S32 pu = u >> upsamplepower;
		S32 pv = v >> upsamplepower;	
		S32 fu = u & (magnitude-1);
		S32 fv = v & (magnitude-1);
		S32 fu_m = magnitude - fu;
		S32 fv_m = magnitude - fv;

#ifdef _DEBUG
		// aleksger: intermediate sample values from o_noise.  Helps to figure out
		// where things are going wrong if the consistency check loop fails in CalculateNoise.
		S32 uv = o_noise[octave*n_size_sq + ((pv)&n_size_m1)*n_size + ((pu)&n_size_m1)];
		S32 vu1 = o_noise[octave*n_size_sq + ((pv)&n_size_m1)*n_size + ((pu+1)&n_size_m1)];
		S32 v1u = o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu)&n_size_m1)];
		S32 v1u1=o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu+1)&n_size_m1)];
#endif
		S32 o = fu_m*fv_m*o_noise[octave*n_size_sq + ((pv)&n_size_m1)*n_size + ((pu)&n_size_m1)] +
				fu*fv_m*o_noise[octave*n_size_sq + ((pv)&n_size_m1)*n_size + ((pu+1)&n_size_m1)] +
				fu_m*fv*o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu)&n_size_m1)] +
				fu*fv*o_noise[octave*n_size_sq + ((pv+1)&n_size_m1)*n_size + ((pu+1)&n_size_m1)];

		return o >> (upsamplepower+upsamplepower);
	}
	
	S32 readtexel_linear_dual(S32 u, S32 v, S32 o)
	{
		S32 iu, iup, iv, ivp, fu, fv;
		iu = (u>>n_dec_bits)&np_size_m1;
		iv = ((v>>n_dec_bits)&np_size_m1)*np_size;
		
		iup = ((u>>n_dec_bits) + 1)&np_size_m1;
		ivp = (((v>>n_dec_bits) + 1)&np_size_m1)*np_size;
		
		fu = u & n_dec_magn_m1;
		fv = v & n_dec_magn_m1;
			
		S32 ut01 = ((n_dec_magn-fu)*r_noise[iv + iu] + fu*r_noise[iv + iup])>>n_dec_bits;
		S32 ut23 = ((n_dec_magn-fu)*r_noise[ivp + iu] + fu*r_noise[ivp + iup])>>n_dec_bits;
		S32 ut = ((n_dec_magn-fv)*ut01 + fv*ut23) >> n_dec_bits;
		return ut;
	}

	float get_height_dual(S32 u, S32 v)
	{	
		S32 value=0;	
		r_noise = p_noise;	// poS32er to the current noise source octave
		S32 hoct = octaves / n_packsize;
		for(S32 i=0; i<hoct; i++)
		{		
			value += readtexel_linear_dual(u,v,0);
			u = u << n_packsize;
			v = v << n_packsize;
			r_noise += np_size_sq;
		}		
		return (float)value;
	}
};

} // namespace Perlin

#endif // RENDER_PERLINCPU_H
