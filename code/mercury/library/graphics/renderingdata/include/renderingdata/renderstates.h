#ifndef RD_RENDER_STATES_H
#define RD_RENDER_STATES_H
#ifdef _MSC_VER
#pragma once
#endif

namespace RenderingData
{
	enum BlendFactor
	{
		BLEND_FACTOR_ZERO,
		BLEND_FACTOR_ONE,
		BLEND_FACTOR_SRCCOLOUR,
		BLEND_FACTOR_INVSRCCOLOUR,
		BLEND_FACTOR_SRCALPHA,
		BLEND_FACTOR_INVSRCALPHA,
		BLEND_FACTOR_DSTALPHA,
		BLEND_FACTOR_INVDSTALPHA,
		BLEND_FACTOR_DSTCOLOUR,
		BLEND_FACTOR_INVDSTCOLOUR,
	};

	enum BlendOp
	{
		BLEND_OP_ADD,
		BLEND_OP_SUBTRACT,
		BLEND_OP_MIN,
		BLEND_OP_MAX,
		BLEND_OP_REVSUBTRACT,
	};

	enum CullMode
	{
		CULL_MODE_NONE,
		CULL_MODE_NORMAL,
		CULL_MODE_REVERSED,
	};

	class BlendState
	{
	public:
		BlendState() :  srcBlend(BLEND_FACTOR_ONE),
						destBlend(BLEND_FACTOR_INVSRCALPHA),
						blendOp(BLEND_OP_ADD),
						alphaBlendEnable(0),
						alphaTestEnable(0),
						alphaRef(0),
						alphaToMaskEnable(0),
						cullMode(CULL_MODE_NORMAL),
						zBias(0),
						invalidate(0) {}

		BlendFactor GetSrcBlend() const;
		BlendFactor GetDestBlend() const;
		BlendOp GetBlendOp() const;
		bool GetAlphaBlendEnable() const;
		bool GetAlphaTestEnable() const;
		U8 GetAlphaRef() const;
		bool GetAlphaToMaskEnable() const;
		CullMode GetCullMode() const;
		S32 GetZBias() const;

		void SetSrcBlend(BlendFactor p);
		void SetDestBlend(BlendFactor p);
		void SetBlendOp(BlendOp p);
		void SetAlphaBlendEnable(bool p);
		void SetAlphaTestEnable(bool p);
		void SetAlphaRef(U8 p);
		void SetAlphaToMaskEnable(bool p);
		void SetCullMode(CullMode p);
		void SetZBias(S32 p); // 0-15

		void SetInvalid();

		friend U32 Hash(const BlendState& blendState, U32 initval);	
		friend bool operator == (const BlendState& b1, const BlendState& b2);
		friend bool operator != (const BlendState& b1, const BlendState& b2);
		friend bool operator < (const BlendState& b1, const BlendState& b2);

	private:
		union
		{
			U32 data;

			struct
			{
				U32 srcBlend : 4;
				U32 destBlend : 4;
				U32 blendOp : 4;
				U32 alphaBlendEnable : 1;
				U32 alphaTestEnable : 1;
				U32 alphaRef : 8;
				U32 alphaToMaskEnable : 1;
				U32 cullMode : 2;
				U32 zBias : 4;
				U32 invalidate : 1; 
			};
		};
	};

	enum TextureAddress
	{
		TEXTURE_ADDRESS_WRAP,
		TEXTURE_ADDRESS_MIRROR,
		TEXTURE_ADDRESS_CLAMP,
		TEXTURE_ADDRESS_BORDER,
	};

	enum TextureFilter
	{
		TEXTURE_FILTER_POINT,
		TEXTURE_FILTER_LINEAR,
		TEXTURE_FILTER_ANISOTROPIC,
	};

	class SamplerState
	{
	public:
		SamplerState() :	addressU(TEXTURE_ADDRESS_WRAP),
							addressV(TEXTURE_ADDRESS_WRAP),
							addressW(TEXTURE_ADDRESS_WRAP),
							magFilter(TEXTURE_FILTER_LINEAR),
							minFilter(TEXTURE_FILTER_LINEAR),
							mipFilter(TEXTURE_FILTER_LINEAR),
							maxAnisotropy(1),
							maxMipLevel(0),
							mipMapLodBias(0),
							sRGB(0) {}

		TextureAddress GetAddressU() const;
		TextureAddress GetAddressV() const;
		TextureAddress GetAddressW() const;
		TextureFilter GetMagFilter() const;
		TextureFilter GetMinFilter() const;
		TextureFilter GetMipFilter() const;
		S32 GetMaxAnisotropy() const;
		S32 GetMaxMipLevel() const;
		S32 GetMipMapLodBias() const;
		U32 GetSRGB() const;

		void SetAddressU(TextureAddress p);
		void SetAddressV(TextureAddress p);
		void SetAddressW(TextureAddress p);
		void SetMagFilter(TextureFilter p);
		void SetMinFilter(TextureFilter p);
		void SetMipFilter(TextureFilter p);
		void SetMaxAnisotropy(S32 p);
		void SetMaxMipLevel(S32 p);
		void SetMipMapLodBias(S32 p);
		void SetSRGB(U32 p);

		friend bool operator == (const SamplerState& p1, const SamplerState& p2);
		friend bool operator != (const SamplerState& p1, const SamplerState& p2);
		friend bool operator < (const SamplerState& p1, const SamplerState& p2);
		friend U32 Hash(const SamplerState& samplerState);

	private:
		union
		{
			U32 data;

			struct
			{
				U32 addressU : 3;
				U32 addressV : 3;
				U32 addressW : 3;
				U32 magFilter : 2;
				U32 minFilter : 2;
				U32 mipFilter : 2;
				U32 maxAnisotropy : 4;
				S32 maxMipLevel : 4;
				S32 mipMapLodBias : 8;
				U32 sRGB : 1;
			};
		};
	};
}

#endif
