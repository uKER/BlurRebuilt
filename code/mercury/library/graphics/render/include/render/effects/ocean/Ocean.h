#ifndef RENDER_OCEAN_H
#define RENDER_OCEAN_H

#ifdef _MSC_VER
#pragma once
#endif



namespace Ocean
{

	void RegenerateWaves(void *pParam);
	
	struct C_Waves
	{
		float	m_Heights[8];
		float	m_Frequencies[8];
		float	m_DirectionsX[8];
		float	m_DirectionsY[8];
		float	m_Offsets[8];
		float	m_Speeds[8];
		float	m_Choppiness[8];
	};

	struct C_PositionConstants
	{
		MathsStorage::Float4	m_BottomLeft;
		MathsStorage::Float4	m_TopLeft;
		MathsStorage::Float4	m_TopRight;
		MathsStorage::Float4	m_BottomRight;
	};

	struct C_PSConstants
	{
		MathsStorage::Float4	m_Specular;		// .x = intensity, .y = power, .z = dull intensity, .w = dull power
		MathsStorage::Float4	m_Reflect;		// .x = specular reflection, .y = specular blur factor
		MathsStorage::Float4	m_Scales;		// .x = normal scale, .y = diffuse scale
		MathsStorage::Float4	m_WaveColour;
		MathsStorage::Float4	m_DeepColour;
		MathsStorage::Float4	m_ObjSunDir;
		MathsStorage::Float4	m_SunCol;
		MathsStorage::Float4	m_SkyCol;
		MathsStorage::Float4	m_FogColour;
		MathsStorage::Float4	m_FogInfo;
		MathsStorage::Float4	m_Animation;
	};

	extern Debug::Float g_MedianWaveLength;
	extern Debug::Float g_MedianAmplitude;
	extern Debug::Float g_Choppiness;
	extern Debug::Vector g_WindDirection;
	extern Debug::Vector g_ScrollingDirection0;
	extern Debug::Vector g_ScrollingDirection1;
	extern Debug::Float g_ScrollingSize0;
	extern Debug::Float g_ScrollingSize1;
	extern Debug::Float g_ScrollingSpeed;
	extern Debug::Float g_SpecularPower;
	extern Debug::Float g_SpecularIntensity;
	extern Debug::Float g_DullSpecularPower;
	extern Debug::Float g_DullSpecularIntensity;
	extern Debug::Float g_SpecularBlurFactor;
	extern Debug::Float g_SpecularReflection;
	extern Debug::Float g_FogMultiplier;
	extern Debug::Colour g_WaveColour;
	extern Debug::Colour g_DeepColour;
	extern Debug::String g_NormalMap0;
	extern Debug::String g_NormalMap1;
	extern Debug::String g_EnvironmentMap;

}; // end namespace Ocean



class C_Ocean : public Display::I_Renderer
{

public:

	C_Ocean(const F32 height);
	~C_Ocean();

	bool	OnCreate();
	void	OnDestroy();
	void	Update(const Maths::C_TimeS& timeStep);
	void	Render();
	void	Reset();

#ifdef PLATFORM_PS3
	void	PatchPixelShader();
#endif

	F32		Height() const	{ return m_Height; }

	void	SetSunDir(const Maths::C_Normal &sunDir)				{ m_SunDir = sunDir; }
	void	SetSunColour(const Maths::C_RGBA_F32 &sunColour)		{ m_SunColour = sunColour; }
	void	SetSkyColour(const Maths::C_RGBA_F32 &skyColour)		{ m_SkyColour = skyColour; }
	void	SetFogColour(const Maths::HVec4 &fogColour)				{ m_FogColour = fogColour; }
	void	SetFogInfo(const Maths::HVec4 &fogInfo)					{ m_FogInfo = fogInfo; }

private:
	
	void	CalculateProjectiveQuad();
	void	UploadConstants();
	void	UploadBoundaries(const Maths::HVec4 &bottomLeft, const Maths::HVec4 &topLeft, const Maths::HVec4 &topRight, const Maths::HVec4 &bottomRight);

private:

	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader[2];

	Display::C_VertexBuffer			m_VertexBuffer;
	Display::C_IndexBuffer			m_IndexBuffer;

	Display::C_VertexDeclaration *	m_pDecl;
	U32								m_VertexStride;

	U32								m_VertexCount;
	U32								m_IndexCount;

	Display::C_Texture2D			m_Normal0Map;
	Display::C_Texture2D			m_Normal1Map;
	Display::C_TextureCube			m_EnvironmentMap;

	Display::C_PixelShaderParameter	m_psLuminance[2];

#ifdef PLATFORM_PS3
	Display::C_PixelShaderParameter	m_Settings_m_Specular[2];
	Display::C_PixelShaderParameter	m_Settings_m_Reflect[2];	
	Display::C_PixelShaderParameter	m_Settings_m_Scales[2];	
	Display::C_PixelShaderParameter	m_Settings_m_WaveColour[2];
	Display::C_PixelShaderParameter	m_Settings_m_DeepColour[2];
	Display::C_PixelShaderParameter	m_Settings_m_ObjSunDir[2];
	Display::C_PixelShaderParameter	m_Settings_m_SunCol[2];
	Display::C_PixelShaderParameter	m_Settings_m_SkyCol[2];
	Display::C_PixelShaderParameter	m_Settings_m_FogColour[2];
	Display::C_PixelShaderParameter	m_Settings_m_FogInfo[2];
	Display::C_PixelShaderParameter	m_Settings_m_Animation[2];
	Display::C_PixelShaderParameter	m_Settings_m_Damping[2];
#endif

	Maths::C_Normal					m_SunDir;
	Maths::C_RGBA_F32				m_SunColour;
	Maths::C_RGBA_F32				m_SkyColour;
	Maths::HVec4					m_FogColour;
	Maths::HVec4					m_FogInfo;

	F32								m_Time;

	F32								m_Height;
	Maths::C_Plane					m_Plane;
	Maths::HVec4					m_ProjectiveCoords[5];
	U32								m_TessellatedEdges;
	bool							m_IsVisible;

	static F32						s_WaveHeights[8];
	static F32						s_WaveFrequencies[8];
	static F32						s_WaveDirectionsX[8];
	static F32						s_WaveDirectionsY[8];
	static F32						s_WaveSpeeds[8];
	static F32						s_WaveOffsets[8];
	static F32						s_WaveChoppiness[8];

};



#endif
