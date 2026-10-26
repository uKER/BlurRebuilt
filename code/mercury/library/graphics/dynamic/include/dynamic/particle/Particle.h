#ifndef RENDER_DYNAMICEFFECT_PARTICLE_PARTICLE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_PARTICLE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{
namespace Particle
{

const static U8 PARTICLE_COLLISION_NONE		= 1<<0;		// free - particles don't collide with anything
const static U8 PARTICLE_COLLISION			= 1<<1;		// particle collided with something on last update
const static U8 PARTICLE_COLLISION_FIRST	= 1<<2;		// particle collided with something for first time on last update
const static U8 PARTICLE_COLLISION_ONCE		= 1<<3;		// if particle has collided at least once
const static U8 PARTICLE_COLLISION_SLEEP	= 1<<4;		// if particle is sleeping
class C_Emitter;

enum E_ParticleElement
{
	// specific types
	E_ParticleElement_Position3,
	E_ParticleElement_Life,
	E_ParticleElement_Velocity3,
	E_ParticleElement_Variant,
	E_ParticleElement_Flags,
	E_ParticleElement_GameData,
	E_ParticleElement_Size,
	E_ParticleElement_LifeModifier,
	E_ParticleElement_Colour,
	E_ParticleElement_Rotation,
	E_ParticleElement_RotationRate,
	E_ParticleElement_RotationQuaternion,
	E_ParticleElement_AngularVelocity,
	E_ParticleElement_CollisionPlane,

	// generic types for padding or whatever
	E_ParticleElement_Byte,
	E_ParticleElement_Byte2,
	E_ParticleElement_Byte4,
	E_ParticleElement_Float,
	E_ParticleElement_Float2,
	E_ParticleElement_Float4
};

INLINE U32 ParticleElementSize(E_ParticleElement element)
{
	switch(element)
	{
	case E_ParticleElement_Position3:
		return sizeof(MathsStorage::Float3);
	case E_ParticleElement_Life:
		return sizeof(F32);
	case E_ParticleElement_Velocity3:
		return sizeof(MathsStorage::Float3);
	case E_ParticleElement_Variant:
		return sizeof(U8);
	case E_ParticleElement_Flags:
		return sizeof(U8);
	case E_ParticleElement_GameData:
		return sizeof(U8);
	case E_ParticleElement_Size:
		return sizeof(F32);
	case E_ParticleElement_LifeModifier:
		return sizeof(F32);
	case E_ParticleElement_Colour:
		return sizeof(MathsStorage::Float4);
	case E_ParticleElement_Rotation:
		return sizeof(F32);
	case E_ParticleElement_RotationRate:
		return sizeof(F32);
	case E_ParticleElement_RotationQuaternion:
		return sizeof(MathsStorage::Float4);
	case E_ParticleElement_AngularVelocity:
		return sizeof(MathsStorage::Float4);
	case E_ParticleElement_CollisionPlane:
		return sizeof(MathsStorage::Float4);
	case E_ParticleElement_Byte:
		return sizeof(U8);
	case E_ParticleElement_Byte2:
		return sizeof(U8) * 2;
	case E_ParticleElement_Byte4:
		return sizeof(U8) * 4;
	case E_ParticleElement_Float:
		return sizeof(F32);
	case E_ParticleElement_Float2:
		return sizeof(F32) * 2;
	case E_ParticleElement_Float4:
		return sizeof(F32) * 4;
	default:
		assert(0);
		return 0;
	}
}

struct ParticleElement
{
	E_ParticleElement	type;
	U32					offset;
};

class C_ParticleFormat
{
public:
	C_ParticleFormat() {}
	~C_ParticleFormat() {}

	void							AddElement(E_ParticleElement element);
	S32								FindElement(E_ParticleElement element);
	S32								GetNumElements() const { return m_Elements.Size(); }
	const ParticleElement&			GetElement(S32 index) const { return m_Elements[index]; }
	U32								GetElementOffset(S32 index) const { return m_Elements[index].offset; }
	U32								GetElementSize() const { return m_Size; }

private:
	Btl::Vector<ParticleElement>	m_Elements;
	U32								m_Size;
};

class Particle3D;
class C_Particle3D;
class C_Particle3D_R1D;
class C_Particle3D_R1D_CP;
class C_Particle3D_R1D_P;
class C_Particle3D_R3D;
class C_Particle3D_RV3D;
class C_Particle3D_RV3D_CP;
class C_Particle3D_P;
class C_Particle3D_CP;

template<class Particle>
void CreateParticleFormat(C_ParticleFormat& format)
{
}

template<>
INLINE void CreateParticleFormat<Particle3D>(C_ParticleFormat& format)
{
	format.AddElement(E_ParticleElement_Position3);
	format.AddElement(E_ParticleElement_Life);
	format.AddElement(E_ParticleElement_Velocity3);
	format.AddElement(E_ParticleElement_Variant);
	format.AddElement(E_ParticleElement_Flags);
	format.AddElement(E_ParticleElement_GameData);
	format.AddElement(E_ParticleElement_GameData);
	format.AddElement(E_ParticleElement_Float2);
	format.AddElement(E_ParticleElement_Size);
	format.AddElement(E_ParticleElement_LifeModifier);
	format.AddElement(E_ParticleElement_Colour);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D>(C_ParticleFormat& format)
{
	CreateParticleFormat<Particle3D>(format);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D_R1D>(C_ParticleFormat& format)
{
	CreateParticleFormat<Particle3D>(format);

	format.AddElement(E_ParticleElement_Rotation);
	format.AddElement(E_ParticleElement_RotationRate);
	format.AddElement(E_ParticleElement_Float2);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D_R1D_CP>(C_ParticleFormat& format)
{
	CreateParticleFormat<C_Particle3D_R1D>(format);

	format.AddElement(E_ParticleElement_CollisionPlane);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D_R1D_P>(C_ParticleFormat& format)
{
	CreateParticleFormat<C_Particle3D_R1D>(format);

	format.AddElement(E_ParticleElement_Position3);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D_R3D>(C_ParticleFormat& format)
{
	CreateParticleFormat<Particle3D>(format);

	format.AddElement(E_ParticleElement_RotationQuaternion);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D_RV3D>(C_ParticleFormat& format)
{
	CreateParticleFormat<C_Particle3D_R3D>(format);

	format.AddElement(E_ParticleElement_AngularVelocity);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D_RV3D_CP>(C_ParticleFormat& format)
{
	CreateParticleFormat<C_Particle3D_RV3D>(format);

	format.AddElement(E_ParticleElement_CollisionPlane);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D_P>(C_ParticleFormat& format)
{
	CreateParticleFormat<Particle3D>(format);

	format.AddElement(E_ParticleElement_Position3);
}

template<>
INLINE void CreateParticleFormat<C_Particle3D_CP>(C_ParticleFormat& format)
{
	CreateParticleFormat<Particle3D>(format);

	format.AddElement(E_ParticleElement_CollisionPlane);
}

// particle class just contains a pointer to an emitter and salt, used to manage traileffects etc
class C_Particle_E
{
public:
	C_Emitter*		pEmitter;
	U32				iSalt;
};

class Particle2D
{
public:
	MathsStorage::Float2	vPosition;
	MathsStorage::Float2	vVelocity;
	MathsStorage::Float3	vLifeSizeLife;				// x = life, y = size, z = life modifier
	U8						iVariant;
	U8						iFlags;
	U8						iGameData0;	
	U8						iGameData1;	
	MathsStorage::Float4	vColour;
};

// 64 bytes
class Particle3D
{
public:
	MathsStorage::Float3	vPosition;					// current position
	F32						fLife;						// current life of particle
	MathsStorage::Float3	vVelocity;					// current velocity
	U8						iVariant;					// for non-mesh particles this is the texture from the sheet to use (0-N).
														// otherwise this is an index into the array of possible mesh variants.
	U8						iFlags;						// collision flags
	U8						iGameData0;					// additional index, eg material type for collisions
	U8						iUV;						// for uvs
	MathsStorage::Float4A16	vForceSizeLife;				// x = force factor, y = mass, z = size, w = life modifier				
	MathsStorage::Float4A16 vColour;					// xyz = rgb, w = a
};

// 80 bytes with 1D rotation
class ALIGN16 C_Particle3D_R1D : public Particle3D
{
public:
	MathsStorage::Float4A16	vRotation;

	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D_R1D>(format); }
	static C_ParticleFormat		format;
};

// 80 bytes with 1D rotation
class ALIGN16 C_Particle3D_R1D_CP : public C_Particle3D_R1D
{
public:
	MathsStorage::Float3	vPlanePosition;			
	MathsStorage::HenDN3	vPlaneNormal;

	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D_R1D_CP>(format); }
	static C_ParticleFormat		format;
};



// position, velocity, life, flags, position2
// 96 bytes
class ALIGN16 C_Particle3D_R1D_P : public C_Particle3D_R1D
{
public:
	MathsStorage::Float4A16	vPosition2;

	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D_R1D_P>(format); }
	static C_ParticleFormat		format;
};

// 80 bytes with 3D rotation/orientation
class ALIGN16 C_Particle3D_R3D : public Particle3D
{
public:
	MathsStorage::Float4A16	qRotation;					// rotation quaternion

	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D_R3D>(format); }
	static C_ParticleFormat		format;
};


// 96 bytes with 3D rotation/orientation and angular velocity
class ALIGN16 C_Particle3D_RV3D : public C_Particle3D_R3D
{
public:
	MathsStorage::Float4A16 vAngularVelocity;			// xyz = angular velocity

	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D_RV3D>(format); }
	static C_ParticleFormat		format;
};

// 112 bytes with 3D rotation/orientation, angular velocity, and collision plane
class ALIGN16 C_Particle3D_RV3D_CP : public C_Particle3D_RV3D
{
public:
	MathsStorage::Float3	vPlanePosition;			
	MathsStorage::HenDN3	vPlaneNormal;

	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D_RV3D_CP>(format); }
	static C_ParticleFormat		format;
};

// position, velocity, life, flags
// 64 bytes
class ALIGN16 C_Particle3D : public Particle3D
{
public:
	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D>(format); }
	static C_ParticleFormat		format;
};

// position, velocity, life, flags, position2
// 80 bytes
class ALIGN16 C_Particle3D_P : public C_Particle3D
{
public:
	MathsStorage::Float4A16	vPosition2;

	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D_P>(format); }
	static C_ParticleFormat		format;
};

// 80 bytes
// allows particles to collide against a per particle plane, keep collisions accurate
class ALIGN16 C_Particle3D_CP : public Particle3D
{
public:
	MathsStorage::Float3	vPlanePosition;			
	MathsStorage::HenDN3	vPlaneNormal;

	static void CreateFormat() { if(format.GetNumElements() == 0) CreateParticleFormat<C_Particle3D_CP>(format); }
	static C_ParticleFormat		format;
};

class ALIGN16 C_Particle2D : public Particle2D
{
public:
};



} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_PARTICLE_H
