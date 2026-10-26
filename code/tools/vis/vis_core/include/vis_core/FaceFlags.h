#pragma once

#define UNINITIALISED_FACE_ID 0x00ffffff
#define FACE_ID_MASK 0x00ffffff
#define FACE_FLAG_MASK 0xff000000

struct FaceFlags
{
	FaceFlags()
	{
		//Zero the top 8 bits and set our ID to max;
		bits = 0x00ffffff;
	}

	FaceFlags(const FaceFlags& rhs)
	{
		*this = rhs;
	}

	const FaceFlags& operator=(const FaceFlags& rhs)
	{
		bits = rhs.bits;
		return *this;
	}

	union
	{
		U32 bits;
		struct
		{
			U32 id : 24;
			U32 unused : 5;
			//Will be rendered with backface culling in wireframe.
			U32 alpha : 1;
			//For us in conjunction with the above to render without backface culling in wireframe.
			U32 doubleSided : 1;
			//use occlusion query
			U32 occlusionquery : 1;
		};
	};
};
