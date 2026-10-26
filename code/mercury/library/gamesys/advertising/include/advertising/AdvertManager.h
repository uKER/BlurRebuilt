#ifndef ADVERTMANAGER_H
#define ADVERTMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PC
	//#define ENABLE_MASSIVE
#endif


/*#ifdef PLATFORM_PS3
	#define ENABLE_IGA
	#include "radial.h"
#endif*/

#include <display.h>
#include <display/buffer/vertexdeclarations.h>

#include "world_render/MaterialParameter.h"

#include "advertising/AdvertResource.h"

#include <SystemProfile\SystemProfile.h>

namespace GameResource
{
	class C_LevelResource;
	class C_LevelInstance;
}

namespace Advertising
{
	class AdvertObject;
	// [Blur2PC] also declared in Advertising.h; needed here when no ad SDK is enabled
	typedef void JumpToZoneCallback(const char* zoneName);

	class AdvertDownload
	{
	public:
		Display::C_Texture2D			m_advertTexture;
		Display::C_Texture2D			m_placeholderTexture;

		WorldRender::MaterialParameter	m_materialParam;

		U8								m_advertIndex;
		U8								m_currentlyShowing	: 2;		// 0=none, 1=Placeholder, 2=Advert
		U8								m_killCount			: 6;		// Countdown timer if we're going to kill stuff
		bool							m_downloadedAdvert	: 1;
		bool							m_loadedPlaceholder	: 1;
		bool							m_visible			: 1;

	public:
		AdvertDownload() : m_currentlyShowing(0), m_killCount(0), m_downloadedAdvert(false), m_loadedPlaceholder(false), m_visible(false)
		{
		}

		void Destroy();

		bool IsMaterialValid() const		{ return m_materialParam.index != -1; }

		Btl::String16 Name(bool alternateName=false) const;

		void SetMaterialParam(WorldRender::MaterialParameter param)		{ m_materialParam = param; }
		const WorldRender::MaterialParameter& GetMaterialParam() const	{ return m_materialParam; }
	};

	class AdvertManager
	{
	protected:
		static AdvertManager s_instance;

		SysProfile::UserIdentifier	m_userIdentifier;

		MT::Lock	m_lock;		// Keep advert stuff nice and thread safe

		// Name of the current "zone" we're in - i.e. game level
		Btl::String	m_zoneName;
		Btl::String	m_levelName;

		// Strings to help identify the game to the advertising system
		Btl::String	m_gameSku;
		Btl::String	m_gameSkuVersion;

		GameResource::C_LevelInstance *m_pLevelInstance;

		Btl::Vector<AdvertDownload>	m_downloads;

		// IGA specific data
#ifdef ENABLE_IGA
		RADSESSION					session;

		Btl::Vector<RADCELL>		m_cells;

		Btl::String					m_cacheLocation;
#endif

		Btl::Vector<AdvertObject*>	m_adverts;
		AdvertResource				m_advertResource;

		Display::C_VertexBuffer		m_vertexBuffer;
		U32							m_vertexCount;
		Display::C_IndexBuffer		m_indexBuffer;
		U32							m_indexCount;

		Display::C_VertexShader		m_vertexShader;
		Display::C_PixelShader		m_pixelShader;

		JumpToZoneCallback*			m_jumpToZoneCallback;

		S32		m_currentRoute;
		U16		m_currentAdvert;

		U8		m_flushBeforeExit;
		bool	m_initialised			: 1;
		bool	m_insideZone			: 1;
		bool	m_advertsEnabled		: 1;	// We don't want them in split-screen - too small = no Floella's for us
		bool	m_systemEnabled			: 1;	// Advert system
		bool	m_validZoneName			: 1;	

		bool	m_errorWhileUpdating	: 1;

		bool	m_suspendMakingMoney	: 1;

#ifdef ENABLE_IGA
		bool	m_refreshCacheDone		: 1;
#endif
	
	protected:
		bool InitialiseSystem();

		void CreateRenderData();
		void DestroyRenderingData();

		void FixUpDownloadedTextures();

		int PrimeAdvertDownload(unsigned int aindex);

	public:
		AdvertManager() :
			m_errorWhileUpdating(false),
			m_systemEnabled(false),
			m_advertsEnabled(false),
			m_initialised(false),
			m_insideZone(false),
			m_suspendMakingMoney(false),
#ifdef ENABLE_IGA
			m_refreshCacheDone(false),
#endif
			m_flushBeforeExit(0),
			m_jumpToZoneCallback(NULL)
			{
			}

		bool IsInitialised() const					{ return m_initialised; }
		bool SystemEnabled() const					{ return m_systemEnabled; }
		void SetSystemEnabled(bool enable)			{ m_systemEnabled = enable; }
		bool ValidZone() const						{ return m_validZoneName; }

		void SetJumpToZoneCallback(JumpToZoneCallback* jtzCallback)	{ m_jumpToZoneCallback = jtzCallback; }

		JumpToZoneCallback* GetJumpToZoneCallback() const { return m_jumpToZoneCallback; }

		const AdvertDownload& GetDownloadTexture(int index) { return m_downloads[index]; }
		int GetNumDownloads() { return m_downloads.Size(); }

		/*
			Initialise the advertising system - called once at the start of the game

			360 - This must be done after logging into Live
		*/
		bool Initialise(const char* skuName, const char* skuVersion);

		// Shut down the advertising system
		void Shutdown();

		// Called when we log into the game and have a profile
		void LoggedIn();

		// We've signed out of our profile
		void LoggedOut();

		// Must be called once per frame to update the stats
		void Update(bool onLoadingScreen);

		// Must be called once per viewport so we can calculate impression information
		void Render(const Display::C_View* view);

		void EnterZone(bool quickRestart, const Btl::String& levelName, S32 route, bool enable);
		void EnterZone_PostLoad(GameResource::C_LevelResource *pLevelResource, GameResource::C_LevelInstance *pLevelInstance);
		void ExitZone(bool forceQuit);
		void RestartZone();
		void TidyZone();

		void ResumeMakingMoney();
		void SuspendMakingMoney();

		void LoadAdvertResource(Sys::I_File& file);

#if defined(ENABLE_IGA) || (defined(ENABLE_MASSIVE) && defined(PLATFORM_PC))
		bool TextureDownloaded(int index, const U8* data, size_t dataLength);
#endif

#if defined(ENABLE_IGA) && defined(PLATFORM_PS3)
		void SetCacheFolder(const Btl::String& cacheFolder);
#endif

#ifdef ENABLE_IGA
		void SetRefreshCacheDone() { m_refreshCacheDone = true; }
#endif

		bool AdvertsEnabled() const { return m_advertsEnabled; }

		const Btl::String& ZoneName() const { return m_zoneName; }

		void GotoAdvert();
		void NextAdvert();
		void PrevAdvert();

#ifdef ENABLE_IGA
		RADSESSION& Session()
		{
			return session;
		}
#endif

		static AdvertManager& Instance() { return s_instance; }
	};
}

#endif
