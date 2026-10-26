#ifndef RANDOM_H
#define RANDOM_H

#ifdef _MSC_VER
#pragma once
#endif

#define RAND_N 624

namespace Util
{

class C_Random
{
public:
                        C_Random();
                        ~C_Random();

    
    /// Seeds the random number generator                        
    void                Seed(const U32 seed);
    
    /// Returns a random integer in the range 0 to 65535  
    const U32           Rand();
    
    /// Returns a random integer in the range 0 to range
    const U32           Rand(const U32 range);
    
    /// Returns a random integer in the range -32767 to 32767  
    const S32           SignedRand();
    
    /// Returns a random integer in the range -range to range
    const S32           SignedRand(const U32 range);
    
    /// Returns a random float in the range 0 to 1
    const F32           FRand();
    
    /// Returns a random float in the range 0 to range
    const F32           FRand(const F32 range);
    
    /// Returns a random float in the range min to max
    const F32           FRand(const F32 theMin, const F32 theMax);
    
    /// Returns a random float in the range -1 to 1
    const F32           SignedFRand();
    
    /// Returns a random float in the range -range to range
    const F32           SignedFRand(const F32 range);
    
    /// Returns a random float in the range 0 to 2.0 * PI
    const F32           AngRand(); 
    
    /// Returns a random float in the range -PI to PI
    const F32           SignedAngRand(); 
    
    /// Returns a random vector in the range 0 to 1 for xy
    const Maths::HVec2  RectRand();
    
    /// Returns a random vector in the range 0 to range for xy
    const Maths::HVec2  RectRand(const F32 range);
    
    /// Returns a random vector in the range -1 to 1 for xy
    const Maths::HVec2  SignedRectRand();
    
    /// Returns a random vector in the range -range to range for xy
    const Maths::HVec2  SignedRectRand(const F32 range);
    
    /// Returns a random vector in the range 0 to 1 for xyz
    const Maths::HVec3  CubeRand();
    
    /// Returns a random vector in the range 0 to range for xyz
    const Maths::HVec3  CubeRand(const F32 range);
    
    /// Returns a random vector in the range -1 to 1 for xyz
    const Maths::HVec3  SignedCubeRand();
    
    /// Returns a random vector in the range -range to range for xyz
    const Maths::HVec3  SignedCubeRand(const F32 range);
    
    /// Returns a random point on a sphere of radius 1
    const Maths::HVec3  SphereRand();

private:   
    void		        UpdateSeed();

public:

	unsigned long mt[RAND_N]; /* the array for the state vector  */
	int		mti; /* mti==N+1 means mt[N] is not initialized */
	U32		m_Value;
};

}; // namespace Util

#endif
