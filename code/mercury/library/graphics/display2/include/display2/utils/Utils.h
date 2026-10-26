#ifndef DISPLAY2_UTILS
#define DISPLAY2_UTILS

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Display2Defines.h"

#if DEVICE_D3D11_PC
#include "Display2/Device/Device.h" // [Blur1PCDX11] Native resource types.
#elif DEVICE_GCM
#include "Display2/Utils/GCMUtils.h"
#endif

#include <Core/Threads/Mutex.h>
#include <Btl/Allocators/Pool.h>
#include <Btl/Containers/OpenHashMap.h>
#include <Core/Threads/ThreadLocalStorage.h>

namespace Display2
{

U64 GetFileTime(const Util::C_String& fileName);

bool GetFileData(const Util::C_String& fileName, U32& fileSize, U8** ppFileData);

//------------------------------------------------------------------------------
// Shaders

extern Util::C_String g_VertexShaderFileExt;

extern Util::C_String g_PixelShaderFileExt;

Util::C_String GetVertexShaderFilePathFromName(const Util::C_String& fileName);

Util::C_String GetPixelShaderFilePathFromName(const Util::C_String& fileName);

U64 GetShaderFileTimeFromPath(const Util::C_String& filePath);

U64 GetPixelShaderFileTimeFromName(const Util::C_String& fileName);

U64 GetVertexShaderFileTimeFromName(const Util::C_String& fileName);

bool LoadShaderFromFilePath(const Util::C_String& filePath, U32& size, U8** ppData);

bool LoadVertexShaderFromFileName(const Util::C_String& fileName, U32& size, U8** ppData);

bool LoadPixelShaderFromFileName(const Util::C_String& fileName, U32& size, U8** ppData);

bool CreateVertexShaderFromFunc(const Util::C_String& fileName, const U32 flags, C_VertexShader* pVertexShader, const U32 size, const U8* pData);

bool CreateVertexShaderFromFile(const Util::C_String& fileName, const U32 flags, C_VertexShader* pVertexShader);

bool CreatePixelShaderFromFunc(const Util::C_String& fileName, const U32 flags, C_PixelShader* pPixelShader, const U32 size, const U8* pData);

bool CreatePixelShaderFromFile(const Util::C_String& fileName, const U32 flags, C_PixelShader* pPixelShader);

void RefreshShaders(bool refreshVertexShaders = true, bool refreshPixelShaders = true);

//------------------------------------------------------------------------------
// Textures

struct C_TextureParams
{
	C_TextureParams();

	U32					m_Width;
	U32					m_Height;
	U32					m_Depth;
	U32					m_Levels;
	U32					m_Flags;
	E_TextureFormat		m_Format;
	U32					m_MipOffset;
	U32					m_MipStart;
	U32					m_MipEnd;
	U32					m_FileSize;
	U8*					m_pFileData;
};

bool LoadDDSTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams);

bool LoadTGATextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams);

bool LoadBMPTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams);

bool LoadJPGTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams);

bool CreateTexture2DFromFile(const Util::C_String& fileName, C_Texture* pTexture);

bool CreateTexture3DFromFile(const Util::C_String& fileName, C_Texture* pTexture);

bool CreateTextureCubeFromFile(const Util::C_String& fileName, C_Texture* pTexture);

bool CloneTexture2D(C_Texture* pSrc, C_Texture* pDst);

//------------------------------------------------------------------------------
// Timings

enum E_Timing
{
	E_Timing_Curr = 0,
	E_Timing_Prev,
	E_Timing_Count,
};

struct C_Timing
{
	C_Timing(const Util::C_String& name);

	bool Enabled() const;
	void Enabled(const bool enabled);

	void Update();

	F32 GetTime() const;

	bool									m_Enabled;
	Util::C_String							m_Name;
	U64										m_Time[E_Timing_Count];

	#if PLATFORM_PS3	
	U32										m_TimestampRead;
	U32										m_TimestampWrite;
	static const U32						m_TimestampCount = 4;
	U32										m_TimestampIndex[m_TimestampCount];
	#endif

};

typedef Btl::OpenHashMap<U32, C_Timing* > C_TimingMap;

class C_TimingManager
{
public:

	~C_TimingManager();

	void BeginFrame();
	void EndFrame();
	void Update();

	static C_TimingManager& GetInstance() { return s_Instance; }

public:

	MT::Mutex								m_Mutex;
	C_TimingMap								m_Timings;

private:

	static C_TimingManager					s_Instance;
};

struct C_TimingString
{
	C_TimingString(const Util::C_String80& name, const S32 level);
	~C_TimingString();

	U32										m_Key;
	const Util::C_String80					m_Name;
	S32										m_Level;
};

struct C_TimingEvent
{
	C_TimingEvent(C_TimingString* pString);
	~C_TimingEvent();

	bool Enabled() const;

	void GetName(Util::C_String& name);
	void GetKey(U32& key);

	S32										m_Depth;
	C_Timing*								m_pTiming;
	C_TimingEvent*							m_pParent;
	C_TimingString*							m_pString;
	static MT::ThreadLocalStorage			m_Tls;

	#if PLATFORM_PS3
	U32										m_TimestampIndex;
	#endif
};

} // namespace Display2

#endif // DISPLAY2_UTILS
