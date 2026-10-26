#ifndef RD_DATA_TYPES_H
#define RD_DATA_TYPES_H
#ifdef _MSC_VER
#pragma once
#endif

#include "misc_shared\Types\float16.h"

namespace RenderingData
{
	enum IndexType
	{
		INDEX_TYPE_U16,
		INDEX_TYPE_U32,
	};

	enum VertexElementType
	{
		// Basic vertex types.
		VERTEX_ELEMENT_TYPE_S16_2,
		VERTEX_ELEMENT_TYPE_S16_4,
		VERTEX_ELEMENT_TYPE_S16_2N,
		VERTEX_ELEMENT_TYPE_S16_4N,
		VERTEX_ELEMENT_TYPE_F32_1,
		VERTEX_ELEMENT_TYPE_F32_2,
		VERTEX_ELEMENT_TYPE_F32_3,
		VERTEX_ELEMENT_TYPE_F32_4,
		VERTEX_ELEMENT_TYPE_F16_2,
		VERTEX_ELEMENT_TYPE_F16_4,
		VERTEX_ELEMENT_TYPE_U8_4,
		VERTEX_ELEMENT_TYPE_U8_4N,

		// Extended types (not supported on all platforms).
		VERTEX_ELEMENT_TYPE_SHHD_3N, // 360, PS3
		VERTEX_ELEMENT_TYPE_SDDD_3N, // 360, PC

		VERTEX_ELEMENT_TYPE_MAX
	};

	enum MaterialElementType
	{
		// Built-in types.
		MATERIAL_ELEMENT_TYPE_S8,
		MATERIAL_ELEMENT_TYPE_U8,
		MATERIAL_ELEMENT_TYPE_S16,
		MATERIAL_ELEMENT_TYPE_U16,
		MATERIAL_ELEMENT_TYPE_S32,
		MATERIAL_ELEMENT_TYPE_U32,
		MATERIAL_ELEMENT_TYPE_F32,

		MATERIAL_ELEMENT_TYPE_BASIC_MAX,

		// Special types.
		MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX = MATERIAL_ELEMENT_TYPE_BASIC_MAX, // S32 index into scene textures, becomes S32 index into render list textures on target.
		MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX, // S32 index into scene materials, becomes MaterialIndex on target.
		MATERIAL_ELEMENT_TYPE_DATA_INDEX, // S32 index to the material data.
		MATERIAL_ELEMENT_TYPE_TEXTURE_PTR, // pointer to gpu texture (dynamic materials only).

		MATERIAL_ELEMENT_TYPE_MAX
	};

	// Types that can be stored in vertex / index buffers or materials,
	// in addition to built-in types (S32 etc).
	namespace DataTypes
	{
		class S16_2
		{
		public:
			typedef S16 DataType;

			S16_2() {}
			S16_2(S16 x_, S16 y_) : x(x_), y(y_) {} 

			S16 X() const {return x;}
			S16 Y() const {return y;}

			void X(S16 v) {x = v;}
			void Y(S16 v) {y = v;}

		private:
			S16 x, y;
		};

		class S16_4 
		{
		public:
			typedef S16 DataType;

			S16_4() {}
			S16_4(S16 x_, S16 y_, S16 z_, S16 w_) : x(x_), y(y_), z(z_), w(w_) {} 

			S16 X() const {return x;}
			S16 Y() const {return y;}
			S16 Z() const {return z;}
			S16 W() const {return w;}

			void X(S16 v) {x = v;}
			void Y(S16 v) {y = v;}
			void Z(S16 v) {z = v;}
			void W(S16 v) {w = v;}

		private:
			S16 x, y, z, w;
		};
		
		class S16_2N 
		{
		public:
			typedef float DataType;

			S16_2N() {}
			S16_2N(float x_, float y_) : x(S16(x_ * 65535.f)), y(S16(y_ * 65535.f)) {} 

			float X() const {return x / 65535.f;}
			float Y() const {return y / 65535.f;}

			void X(float v) {x = S16(v * 65535.f);}
			void Y(float v) {y = S16(v * 65535.f);}

		private:
			S16 x, y;
		};

		class S16_4N 
		{
		public:
			typedef float DataType;

			S16_4N() {}
			S16_4N(float x_, float y_, float z_, float w_) : x(S16(x_ * 65535.f)), y(S16(y_ * 65535.f)), z(S16(z_ * 65535.f)), w(S16(w_ * 65535.f)) {} 

			float X() const {return x / 65535.f;}
			float Y() const {return y / 65535.f;}
			float Z() const {return z / 65535.f;}
			float W() const {return w / 65535.f;}

			void X(float v) {x = S16(v * 65535.f);}
			void Y(float v) {y = S16(v * 65535.f);}
			void Z(float v) {z = S16(v * 65535.f);}
			void W(float v) {w = S16(v * 65535.f);}

		private:
			S16 x, y, z, w;
		};

		class F32_1
		{
		public:
			typedef float DataType;

			F32_1() {}
			F32_1(float x_) : x(x_) {} 

			float X() const {return x;}

			void X(float v) {x = v;}

		private:
			float x;
		};

		class F32_2 
		{
		public:
			typedef float DataType;

			F32_2() {}
			F32_2(float x_, float y_) : x(x_), y(y_) {} 

			float X() const {return x;}
			float Y() const {return y;}

			void X(float v) {x = v;}
			void Y(float v) {y = v;}

		private:
			float x, y;
		};
		
		class F32_3 
		{
		public:
			typedef float DataType;

			F32_3() {}
			F32_3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {} 

			float X() const {return x;}
			float Y() const {return y;}
			float Z() const {return z;}

			void X(float v) {x = v;}
			void Y(float v) {y = v;}
			void Z(float v) {z = v;}

		private:
			float x, y, z;
		};
		
		class F32_4 
		{
		public:
			typedef float DataType;

			F32_4() {}
			F32_4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {} 

			float X() const {return x;}
			float Y() const {return y;}
			float Z() const {return z;}
			float W() const {return w;}

			void X(float v) {x = v;}
			void Y(float v) {y = v;}
			void Z(float v) {z = v;}
			void W(float v) {w = v;}

		private:
			float x, y, z, w;
		};
		
		class F16_2 
		{
		public:
			typedef float DataType;

			F16_2() {}
			F16_2(float x_, float y_) : x(x_), y(y_) {}

			float X() const {return x;}
			float Y() const {return y;}

			void X(float v) {x = v;}
			void Y(float v) {y = v;}

		private:
			Half::F16 x, y;
		};
		
		class F16_4 
		{
		public:
			typedef float DataType;

			F16_4() {}
			F16_4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {} 

			float X() const {return x;}
			float Y() const {return y;}
			float Z() const {return z;}
			float W() const {return w;}

			void X(float v) {x = v;}
			void Y(float v) {y = v;}
			void Z(float v) {z = v;}
			void W(float v) {w = v;}

		private:
			Half::F16 x, y, z, w;
		};
		
		class U8_4 
		{
		public:
			typedef U8 DataType;

			U8_4() {}

			U8_4(U8 x, U8 y, U8 z, U8 w) 
			{
				data = w << 24 | z << 16 | y << 8 | x;
			}

			U8_4(U32 xyzw) : data(xyzw) {}

			U8 W() const {return (U8)((data & 0xff000000) >> 24);}
			U8 Z() const {return (U8)((data & 0x00ff0000) >> 16);}
			U8 Y() const {return (U8)((data & 0x0000ff00) >> 8);}
			U8 X() const {return (U8)(data & 0x000000ff);}

			void W(U8 v) {data = (data & 0x00ffffff) | (v << 24);}
			void Z(U8 v) {data = (data & 0xff00ffff) | (v << 16);}
			void Y(U8 v) {data = (data & 0xffff00ff) | (v << 8);}
			void X(U8 v) {data = (data & 0xffffff00) | (v);}

		private:
			U32 data;
		};

		class U8_4N 
		{
		public:
			typedef float DataType;

			U8_4N() {}

			U8_4N(U8 r, U8 g, U8 b, U8 a) 
			{
				data = a << 24 | b << 16 | g << 8 | r;
			}

			U8_4N(float r, float g, float b, float a) 
			{
				data = int(a * 255.f) << 24 | int(b * 255.f) << 16 | int(g * 255.f) << 8 | int(r * 255);
			}

			U8_4N(U32 rgba) : data(rgba) {}

			float A() const {return ((data & 0xff000000) >> 24) / 255.f;}
			float B() const {return ((data & 0x00ff0000) >> 16) / 255.f;}
			float G() const {return ((data & 0x0000ff00) >> 8) / 255.f;}
			float R() const {return (data & 0x000000ff) / 255.f;}

			void A(float v) {assert(v >= 0.f); data = (data & 0x00ffffff) | (int(v * 255.f) << 24);}
			void B(float v) {assert(v >= 0.f); data = (data & 0xff00ffff) | (int(v * 255.f) << 16);}
			void G(float v) {assert(v >= 0.f); data = (data & 0xffff00ff) | (int(v * 255.f) << 8);}
			void R(float v) {assert(v >= 0.f); data = (data & 0xffffff00) | (int(v * 255.f));}

			void W(float v) {A(v);}
			void Z(float v) {B(v);}
			void Y(float v) {G(v);}
			void X(float v) {R(v);}

			float W() const {return A();}
			float Z() const {return B();}
			float Y() const {return G();}
			float X() const {return R();}

		private:
			U32 data;
		};

		class SHHD_3N 
		{
		public:
			typedef float DataType;

			SHHD_3N() {}

			SHHD_3N(float x_, float y_, float z_) : data(0),
													x((S32) (x_ * 1023.0f)), 
													y((S32) (y_ * 1023.0f)), 
													z((S32) (z_ * 511.0f)) {}

			float X() const {return ((float) x) / 1023.0f;}
			float Y() const {return ((float) y) / 1023.0f;}
			float Z() const {return ((float) z) / 511.0f;}

			void X(float v) {x = (S32) (v * 1023.0f);}
			void Y(float v) {y = (S32) (v * 1023.0f);}
			void Z(float v) {z = (S32) (v * 511.0f);}

		private:
			union
			{
				S32 data;

				// to do: bitfield order wrong on PPC
				// (okay on PC for export)
				struct
				{
					S32 x : 11;
					S32 y : 11;
					S32 z : 10;
				};
			};
		};

		class SDDD_3N 
		{
		public:
			typedef float DataType;

			SDDD_3N() {}

			SDDD_3N(float x_, float y_, float z_) : data(0),
													x((S32) (x_ * 511.0f)), 
													y((S32) (y_ * 511.0f)), 
													z((S32) (z_ * 511.0f)) {}

			float X() const {return ((float) x) / 511.0f;}
			float Y() const {return ((float) y) / 511.0f;}
			float Z() const {return ((float) z) / 511.0f;}

			void X(float v) {x = (S32) (v * 511.0f);}
			void Y(float v) {y = (S32) (v * 511.0f);}
			void Z(float v) {z = (S32) (v * 511.0f);}

		private:
			union
			{
				S32 data;

				// to do: bitfield order wrong on PPC
				// (okay on PC for export)
				struct
				{
					S32 x : 10;
					S32 y : 10;
					S32 z : 10;
				};
			};
		};
	}
}

#endif



