#ifndef ATTK_PIXEL_FORMATS_H
#define ATTK_PIXEL_FORMATS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	struct C_PixelR8G8B8A8U;
	struct C_PixelA8B8G8R8U;
	struct C_PixelR8G8B8A8U;
	struct C_PixelA8R8G8B8U;
	struct C_PixelR8G8B8U;
	struct C_PixelB8G8R8A8U;
	struct C_PixelB8G8R8U;
	struct C_PixelLuminance8U;
	struct C_PixelLuminance16U;
	struct C_PixelR32G32B32A32F;
	struct C_PixelR32G32B32F;
	struct C_PixelB32G32R32F;
	struct C_PixelB32G32R32A32F;
	struct C_PixelA32R32G32B32F;
	struct C_PixelColourIndex8U;



	//Interchange format.
	struct C_PixelR32G32B32A32F
	{
		C_PixelR32G32B32A32F() {}

		C_PixelR32G32B32A32F(F32 r, F32 g, F32 b, F32 a) :
			red(r), green(g), blue(b), alpha(a) {}

		C_PixelR32G32B32A32F(const C_PixelA8B8G8R8U& rhs);

		C_PixelR32G32B32A32F(const C_PixelR8G8B8A8U& rhs);

		C_PixelR32G32B32A32F(const C_PixelA8R8G8B8U& rhs);

		C_PixelR32G32B32A32F(const C_PixelR8G8B8U& rhs);

		C_PixelR32G32B32A32F(const C_PixelB8G8R8A8U& rhs);

		C_PixelR32G32B32A32F(const C_PixelB8G8R8U& rhs);

		C_PixelR32G32B32A32F(const C_PixelLuminance8U& rhs);

		C_PixelR32G32B32A32F(const C_PixelLuminance16U& rhs);

		C_PixelR32G32B32A32F(const C_PixelR32G32B32A32F& rhs);

		C_PixelR32G32B32A32F(const C_PixelR32G32B32F& rhs);

		C_PixelR32G32B32A32F(const C_PixelB32G32R32F& rhs);
	
		C_PixelR32G32B32A32F(const C_PixelB32G32R32A32F& rhs);

		C_PixelR32G32B32A32F(const C_PixelA32R32G32B32F& rhs);

		C_PixelR32G32B32A32F(const C_PixelColourIndex8U& rhs, 
			const C_PixelR32G32B32A32F* pInterchangePalette);


		void Set(const C_PixelA8B8G8R8U& rhs);

		void Set(const C_PixelR8G8B8A8U& rhs);

		void Set(const C_PixelA8R8G8B8U& rhs);

		void Set(const C_PixelR8G8B8U& rhs);

		void Set(const C_PixelB8G8R8A8U& rhs);

		void Set(const C_PixelB8G8R8U& rhs);

		void Set(const C_PixelLuminance8U& rhs);

		void Set(const C_PixelLuminance16U& rhs);

		void Set(const C_PixelR32G32B32A32F& rhs);

		void Set(const C_PixelR32G32B32F& rhs);

		void Set(const C_PixelB32G32R32F& rhs);

		void Set(const C_PixelB32G32R32A32F& rhs);

		void Set(const C_PixelA32R32G32B32F& rhs);

		void Set(const C_PixelColourIndex8U& rhs, 
			const C_PixelR32G32B32A32F* pInterchangePalette);

		const C_PixelR32G32B32A32F operator * (F32 scalar)
		{
			C_PixelR32G32B32A32F rtn; 
			rtn.red = red * scalar;
			rtn.green = green * scalar;
			rtn.blue = blue * scalar;
			rtn.alpha = alpha * scalar;

			return rtn;
		}

		const C_PixelR32G32B32A32F operator + (const C_PixelR32G32B32A32F& rhs)
		{
			C_PixelR32G32B32A32F rtn; 
			rtn.red = red + rhs.red;
			rtn.green = green + rhs.green;
			rtn.blue = blue + rhs.blue;
			rtn.alpha = alpha + rhs.alpha;

			return rtn;
		}

		union
		{
			F32 data[4];

			struct
			{
				F32 red;
				F32 green;
				F32 blue;
				F32 alpha;
			};
		};

		typedef F32 ComponentType;
	};

	struct C_PixelA8B8G8R8U
	{
		union
		{
			U32 data;

			struct
			{
				U8 alpha;
				U8 blue;
				U8 green;
				U8 red;
			};
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			if (rhs.red > 1.f)
				red = 255;
			else
				red = (U8)(rhs.red * 255.f);

			if (rhs.green > 1.f)
				green = 255;
			else
				green = (U8)(rhs.green * 255.f);

			if (rhs.blue > 1.f)
				blue = 255;
			else 
				blue = (U8)(rhs.blue * 255.f);

			if (rhs.alpha > 1.f)
				alpha = 255;
			else
				alpha = (U8)(rhs.alpha * 255.f);
		}

		typedef U8 ComponentType;
	};


	struct C_PixelR8G8B8A8U
	{
		union
		{
			U32 data;

			struct
			{
				U8 red;
				U8 green;
				U8 blue;
				U8 alpha;
			};
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			if (rhs.red > 1.f)
				red = 255;
			else
				red = (U8)(rhs.red * 255.f);

			if (rhs.green > 1.f)
				green = 255;
			else
				green = (U8)(rhs.green * 255.f);

			if (rhs.blue > 1.f)
				blue = 255;
			else 
				blue = (U8)(rhs.blue * 255.f);

			if (rhs.alpha > 1.f)
				alpha = 255;
			else
				alpha = (U8)(rhs.alpha * 255.f);
		}

		typedef U8 ComponentType;
	};

	struct C_PixelLuminance8U
	{
		U8 luminance;

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			F32 luminanceF = (rhs.red + rhs.green + rhs.blue) / 3;
			if (luminanceF > 1.f)
				luminance = 255;
			else
				luminance = (U8)(luminanceF * 255.f);
		}

		typedef U8 ComponentType;
	};

	struct C_PixelLuminance16U
	{
		U16 luminance;

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			F32 luminanceF = (rhs.red + rhs.green + rhs.blue) / 3;
			if (luminanceF > 1.f)
				luminance = 65535;
			else
				luminance = (U16)(luminanceF * 65535.f);
		}

		typedef U16 ComponentType;
	};

	struct C_PixelA8R8G8B8U
	{
		union
		{
			U32 data;

			struct
			{
				U8 alpha;
				U8 red;
				U8 green;
				U8 blue;
			};
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			if (rhs.red > 1.f)
				red = 255;
			else
				red = (U8)(rhs.red * 255.f);

			if (rhs.green > 1.f)
				green = 255;
			else
				green = (U8)(rhs.green * 255.f);

			if (rhs.blue > 1.f)
				blue = 255;
			else 
				blue = (U8)(rhs.blue * 255.f);

			if (rhs.alpha > 1.f)
				alpha = 255;
			else
				alpha = (U8)(rhs.alpha * 255.f);
		}

		typedef U8 ComponentType;
	};

	struct C_PixelB8G8R8A8U
	{
		union
		{
			U32 data;

			struct
			{
				U8 blue;
				U8 green;
				U8 red;
				U8 alpha;
			};
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			if (rhs.red > 1.f)
				red = 255;
			else
				red = (U8)(rhs.red * 255.f);
			
			if (rhs.green > 1.f)
				green = 255;
			else
				green = (U8)(rhs.green * 255.f);

			if (rhs.blue > 1.f)
                blue = 255;
			else
				blue = (U8)(rhs.blue * 255.f);

			if (rhs.alpha > 1.f)
				alpha = 255;
			else
				alpha = (U8)(rhs.alpha * 255.f);
		}

		typedef U8 ComponentType;
	};


	struct C_PixelR8G8B8U
	{
		struct
		{
			U8 red;
			U8 green;
			U8 blue;
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			if (rhs.red > 1.f)
				red = 255;
			else
				red = (U8)(rhs.red * 255.f);

			if (rhs.green > 1.f)
				green = 255;
			else
				green = (U8)(rhs.green * 255.f);

			if (rhs.blue > 1.f)
				blue = 255;
			else
                blue = (U8)(rhs.blue * 255.f);
		}

		typedef U8 ComponentType;
	};

	struct C_PixelB8G8R8U
	{
		struct
		{
			U8 blue;
			U8 green;
			U8 red;
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			if (rhs.blue > 1.f)
				blue = 255;
			else
				blue = (U8)(rhs.blue * 255.f);

			if (rhs.green > 1.f)
				green = 255;
			else
				green = (U8)(rhs.green * 255.f);

			if (rhs.red > 1.f)
				red = 255;
			else
				red = (U8)(rhs.red * 255.f);
		}

		typedef U8 ComponentType;
	};


	struct C_PixelR32G32B32F
	{
		union
		{
			F32 data[3];

			struct
			{
				F32 red;
				F32 green;
				F32 blue;
			};
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			memcpy(data, rhs.data, sizeof(data));	
		}

		typedef F32 ComponentType;
	};

	struct C_PixelB32G32R32F
	{
		union
		{
			F32 data[3];

			struct
			{
				F32 blue;
				F32 green;
				F32 red;
			};
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			blue = rhs.blue;
			green = rhs.green;
			red = rhs.red;
		}

		typedef F32 ComponentType;
	};

	struct C_PixelB32G32R32A32F
	{
		union
		{
			F32 data[4];

			struct
			{
				F32 blue;
				F32 green;
				F32 red;
				F32 alpha;
			};
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			blue = rhs.blue;
			green = rhs.green;
			red = rhs.red;
			alpha = rhs.alpha;
		}

		typedef F32 ComponentType;
	};

	struct C_PixelA32R32G32B32F
	{
		union
		{
			F32 data[4];

			struct
			{
				F32 alpha;
				F32 red;
				F32 green;
				F32 blue;
			};
		};

		void Set(const C_PixelR32G32B32A32F& rhs)
		{
			alpha = rhs.alpha;
			red = rhs.red;
			green = rhs.green;
			blue = rhs.blue;
		}

		typedef F32 ComponentType;
	};
	

	struct C_PixelColourIndex8U
	{
		U8 index;

		//We cannot convert to a palettised image type.
	};


	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelA8B8G8R8U& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelR8G8B8A8U& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelA8R8G8B8U& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelR8G8B8U& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelB8G8R8A8U& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelB8G8R8U& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelLuminance8U& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelLuminance16U& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelR32G32B32A32F& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelR32G32B32F& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelB32G32R32F& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelB32G32R32A32F& rhs)
	{
		Set(rhs);
	}

	FORCE_INLINE C_PixelR32G32B32A32F::C_PixelR32G32B32A32F(const C_PixelA32R32G32B32F& rhs)
	{
		Set(rhs);
	}



	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelA8B8G8R8U& rhs)
	{
		alpha = (F32)rhs.alpha/255.f;
		red = (F32)rhs.red/255.f;
		green = (F32)rhs.green/255.f;
		blue = (F32)rhs.blue/255.f;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelR8G8B8A8U& rhs)
	{
		red = (F32)rhs.red/255.f;
		green = (F32)rhs.green/255.f;
		blue = (F32)rhs.blue/255.f;
		alpha = (F32)rhs.alpha/255.f;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelA8R8G8B8U& rhs)
	{
		alpha = (F32)rhs.alpha/255.f;
		red = (F32)rhs.red/255.f;
		green = (F32)rhs.green/255.f;
		blue = (F32)rhs.blue/255.f;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelR8G8B8U& rhs)
	{
		alpha = 1.0f;
		red = (F32)rhs.red/255.f;
		green = (F32)rhs.green/255.f;
		blue = (F32)rhs.blue/255.f;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelB8G8R8A8U& rhs)
	{
		blue = (F32)rhs.blue/255.f;
		green = (F32)rhs.green/255.f;
		red = (F32)rhs.red/255.f;
		alpha = (F32)rhs.alpha/255.f;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelB8G8R8U& rhs)
	{
		alpha = 1.0f;
		blue = (F32)rhs.blue/255.f;
		green = (F32)rhs.green/255.f;
		red = (F32)rhs.red/255.f;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelLuminance8U& rhs)
	{
		alpha = 1.0f;
		red = (F32)rhs.luminance/255.f;
		green = (F32)rhs.luminance/255.f;
		blue = (F32)rhs.luminance/255.f;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelLuminance16U& rhs)
	{
		alpha = 1.0f;
		red = (F32)rhs.luminance/65535.f;
		green = (F32)rhs.luminance/65535.f;
		blue = (F32)rhs.luminance/65535.f;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelR32G32B32A32F& rhs)
	{
		memcpy(data, rhs.data, sizeof(C_PixelR32G32B32A32F));
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelR32G32B32F& rhs)
	{
		alpha = 1.0f;
		red = rhs.red;
		green = rhs.green;
		blue = rhs.blue;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelB32G32R32F& rhs)
	{
		alpha = 1.0f;
		red = rhs.red;
		green = rhs.green;
		blue = rhs.blue;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelB32G32R32A32F& rhs)
	{
		alpha = rhs.alpha;
		red = rhs.red;
		green = rhs.green;
		blue = rhs.blue;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelA32R32G32B32F& rhs)
	{
		alpha = rhs.alpha;
		red = rhs.red;
		green = rhs.green;
		blue = rhs.blue;
	}

	FORCE_INLINE void C_PixelR32G32B32A32F::Set(const C_PixelColourIndex8U& rhs, 
		const C_PixelR32G32B32A32F* pInterchangePalette)
	{
		assert(!"TODO");
	}
}

#endif
