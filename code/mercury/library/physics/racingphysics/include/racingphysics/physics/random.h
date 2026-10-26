////////////////////////////////////////////////////////////
// Random
////////////////////////////////////////////////////////////

#pragma once

namespace RacingPhysics 
{

// x > 0 returns random float 0 to x
// x < 0 returns random float -x to x
inline float Random(float x = 1.f) 
{
  if (x > 0) return x * (((float)rand())/RAND_MAX);
  else return x * (((float)(rand() - RAND_MAX / 2))/ (RAND_MAX / 2));
}

// random integer up to x - 1
inline int Randomi(int x) 
{
	if (x > 0)
	{
		int nRandomNumber = rand();

		return nRandomNumber % x;
	}
	else return 0;
}

// float 0 to 1 which is noise function of x
inline float Noise(int x)
{
   x = (x<<13) ^ x;
   return (1.f - (float) ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.f);
}

inline int Noisei(int x)
{
   x = (x<<13) ^ x;
   return x * (x * x * 15731 + 789221) + 1376312589;
}

}
