#include "AdvertisingPch.h"

#include "Advertising\Advertising.h"
#include "Advertising\AdvertResource.h"
#include "Advertising\AdvertManager.h"
#include "Advertising\AdvertObject.h"


#ifdef ENABLE_MASSIVE
#include "XMassiveAdClient.h"
#endif

#ifdef ENABLE_IGA
#include <cell/cell_fs.h>
#endif

#if defined(ENABLE_IGA) || (defined(ENABLE_MASSIVE) && defined(PLATFORM_PC))
#include "Display/Texture/DDS/DDSTextureUtils.h"
#endif

#include "display\view\view.h"

#include <System/debug/I_DebugRender.h>

#include <camera2.h>
#include <framework/application/ApplicationBase.h>

#include <game_resource/LevelResource.h>
#include <game_resource/LevelSceneManager.h>
#include "world_render/RenderLevel.h"

#include <SystemProfile/SystemProfilemanager.h>
#include "ui/Game/Profiles/ProfileInterface.h"

//#pragma optimize("", off)

#ifdef ENABLE_MASSIVE
	DEBUG_ENUM(MassiveAdClient3::MAD_LOG_LEVEL)
	DE_OPTION(MassiveAdClient3::LOG_LEVEL_SEVERE,		"Severe")
	DE_OPTION(MassiveAdClient3::LOG_LEVEL_ERROR,		"Error")
	DE_OPTION(MassiveAdClient3::LOG_LEVEL_WARNING,		"Warning")
	DE_OPTION(MassiveAdClient3::LOG_LEVEL_INFO,			"Info")
	DE_OPTION(MassiveAdClient3::LOG_LEVEL_DEBUG,		"Debug")
	DE_OPTION(MassiveAdClient3::LOG_LEVEL_IMPRESSION,	"Impression")
	DE_OPTION(MassiveAdClient3::LOG_LEVEL_TRACE,		"Trace")
	DE_OPTION(MassiveAdClient3::LOG_LEVEL_ALL,			"All")
	END_DEBUG_ENUM
#endif

namespace Advertising
{
#ifdef ENABLE_MASSIVE
	using namespace MassiveAdClient3;

	Debug::Enum<MAD_LOG_LEVEL> g_MassiveLogLevelDebugger("System.Advertising.MassiveLogLevelDebugger", LOG_LEVEL_WARNING, Debug::Persist());
	Debug::Enum<MAD_LOG_LEVEL> g_MassiveLogLevelFile("System.Advertising.MassiveLogLevelFile2", LOG_LEVEL_ERROR, Debug::Persist());

	Debug::Bool g_ShowMassiveCalculatedExtents("System.Advertising.ShowMassiveCalculatedExtents", true, Debug::Persist());
#endif

	Debug::Bool g_ShowMoneyMaker("System.Advertising.ShowMoneyMakers", false, Debug::Persist());
	Debug::Bool g_ShowStats("System.Advertising.ShowStats", false, Debug::Persist());
	Debug::Bool g_ShowExtents("System.Advertising.ShowExtents", false, Debug::Persist());
	Debug::Bool g_bShowOcclusionQueries("System.Advertising.ShowOcclusionQueries", false);

// IGA data
#ifdef ENABLE_IGA
	char		heap_buffer[RAD_MEMHEAPSIZE_RECOMMENDED];
#endif

AdvertManager AdvertManager::s_instance;

Debug::Logger &gLog( Debug::GetLogger("advertising") );

#ifdef ENABLE_MASSIVE
#include "Advertising\Advert_Massive.inl"
#endif

#ifdef ENABLE_IGA
#include "Advertising\Advert_IGA.inl"
#endif

// Amax		skuName = atvi_racing_09_x360
//			TitleID = 0x41560845
bool AdvertManager::Initialise(const char* skuName, const char* skuVersion)
{
	m_gameSku = skuName;
	m_gameSkuVersion = skuVersion;

	return true;
}

bool AdvertManager::InitialiseSystem()
{
	MEMORY_TRACKER_CONTEXT("advertising");

	m_lock.Enter();

	int primaryPadNumber = SysProfile::C_Manager::Instance().GetPrimaryPadNo();

	SysProfile::FullProfile fullProfile;
	bool getProfile = (primaryPadNumber != -1) && SysProfile::C_Manager::Instance().GetProfile(primaryPadNumber, fullProfile);
	if (getProfile)
	{
		if (m_userIdentifier == fullProfile)
		{
			LOGDEBUG(gLog, "AdvertManager::InitialiseSystem - Same profile we're already using - keep using");
			m_lock.Exit();
			return true;
		}
	}

	if (m_initialised)
		Shutdown();

	LOGDEBUG(gLog, "AdvertManager::InitialiseSystem");

	if ((primaryPadNumber != -1) && UI::C_ProfileInterface::Instance().PadProfileOnline(primaryPadNumber))
	{
		LOGDEBUG(gLog, "AdvertManager::InitialiseSystem - Signed In - Initialising AdvertSystem");

		m_userIdentifier = fullProfile;

#ifdef ENABLE_MASSIVE
		// Specify Massive Callbacks for Teleport Functionality
		XMassiveAdClient::SetTeleportToZoneFunction(CallbackTeleportToZone);
		XMassiveAdClient::SetTeleportWithinZoneFunction(CallbackTeleportWithinZone);

		XMassiveAdClient::SetCustomMemoryFunctions(MassiveAlloc, MassiveFree);

		// Initialize our connection to the Massive server
		// Setup the Massive Initialization Structure
		MAD_INIT_PARAMS InitStruct;
		memset(&InitStruct, 0, sizeof(MAD_INIT_PARAMS));

		// The unique SKU identifier for this title
		InitStruct.szSku = m_gameSku.Chars();

		// Version info
		InitStruct.szSkuVersion = m_gameSkuVersion.Chars();

		// Library config flags.  Xbox Live guarantees a broadband connection, so we can
		// use the MAD_NETWORK_FAST flag to double the packet size.
		InitStruct.usLibraryConfigFlags = MAD_NETWORK_FAST;

		// Set the debug logfile verboseness
		InitStruct.eLogLevelFile = g_MassiveLogLevelFile;

		// Set the debug output window verboseness
		InitStruct.eLogLevelDebugger = g_MassiveLogLevelDebugger;

		// Specify how long to wait between impression flushes
		InitStruct.ulImpressionFlushTime = 60000;

		// Set the Version of the headers for AdClientHeader
		// Preventing Headers from mismatching the Massive Library
		InitStruct.szHeader = MAD_HEADER;

		// Init the Massive client core system
		// and connect to the Massive servers.
		DWORD result = XMassiveAdClient::Initialize(&InitStruct);

		if (result == MAD_INTERNAL_SUCCESS)
		{
			LOGDEBUG(gLog, "XMassiveAdClient::Initialize - OK");
			m_systemEnabled = true;
		}
		else
		{
			LOGERROR(gLog, "XMassiveAdClient::Initialize - %d - 0x%x", result, result);
		}
#endif

#ifdef ENABLE_IGA
		LOGDEBUG(gLog, "+ SDK Version: [%x]", RAD_VERSION);
		LOGDEBUG(gLog, "+ Platform ID [%s]", PlatformPS3::g_ps3_titleId);

		memset(&session, 0, sizeof(session));

		session.manual_script_load		= 0;
		session.thread_priority			= 2;
		session.impression_cache_size	= 999;
		session.cache_dir				= m_cacheLocation.Chars();
		session.cache_size				= 5*1024*1024;
		session.debugfunc				= my_debugfunc;
		session.debug_level				= RAD_DEBUG_VERBOSE|/*RAD_DEBUG_MEMORY|*/RAD_DEBUG_DOWNLOAD|RAD_DEBUG_CELLTRACK|RAD_DEBUG_CACHE|RAD_DEBUG_CONTENT|RAD_DEBUG_IMPRESSION|RAD_DEBUG_HTTP|RAD_DEBUG_SDK|RAD_DEBUG_SCRIPT|RAD_DEBUG_HEAP;

		session.title_id				= PlatformPS3::g_ps3_titleId;
		session.reference_server		= NULL;
		session.heap_buffer				= heap_buffer;
		session.heap_size				= sizeof(heap_buffer);
		session.lean_heap				= 1;
		strcpy_s(session.game_version, lengthof(session.game_version), m_gameSkuVersion.Chars());

		// start new session
		LOGDEBUG(gLog, "Advertising: calling rad_start_session() - cache(%s)", session.cache_dir);
		int ret = rad_start_session(&session, my_startfunc, (void*)"start_session_userdata");

		if (ret == RAD_SUCCESS)
		{
			m_systemEnabled = true;
		}
		else
		{
			LOGERROR(gLog, "Advertising: rad_start_session(): failed %d", ret);
		}
#endif
	}
	else
	{
		LOGDEBUG(gLog, "AdvertManager::InitialiseSystem - Not Signed In");
		m_userIdentifier = SysProfile::UserIdentifier();
	}

	m_initialised = true;

	m_lock.Exit();

    return true;
}

void AdvertManager::Shutdown()
{
	m_lock.Enter();

	MEMORY_TRACKER_CONTEXT("advertising");

	LOGDEBUG(gLog, "AdvertManager::Shutdown");

	ExitZone(true);

	if (m_initialised)
	{
#ifdef ENABLE_MASSIVE
		if (m_systemEnabled)
		{
			DWORD result = XMassiveAdClient::Shutdown(5000);

			if (result == MAD_INTERNAL_SUCCESS)
			{
				LOGDEBUG(gLog, "AdvertManager::Shutdown - XMassiveAdClient::Shutdown");
			}
			else
			{
				LOGERROR(gLog, "AdvertManager::Shutdown - XMassiveAdClient::Shutdown - %d", result);
			}
		}
#endif

#ifdef ENABLE_IGA
		rad_stop_session(&session);
#endif

		m_systemEnabled = false;
		m_initialised = false;
	}

	// Clear the user identifier 
	m_userIdentifier = SysProfile::UserIdentifier();

	m_lock.Exit();
}

void AdvertManager::LoggedIn()
{
	InitialiseSystem();
}

#if defined(ENABLE_IGA) && defined(PLATFORM_PS3)

void AdvertManager::SetCacheFolder(const Btl::String& cacheFolder)
{
	m_cacheLocation.Printf("%s/igaCacheFolder", cacheFolder.Chars());;
}

#endif

void AdvertManager::Update(bool onLoadingScreen)
{
	m_lock.Enter();

	if (!onLoadingScreen)
	{
		FixUpDownloadedTextures();
	}

#ifdef ENABLE_MASSIVE
	if (m_flushBeforeExit > 0)
	{
		if (--m_flushBeforeExit)
		{
			LOGDEBUG(gLog, "AdvertManager::Update - Manual XMassiveAdClient::FlushImpressions");

			// Manually flush impression data to avoid leaking memory
			DWORD result = XMassiveAdClient::FlushImpressions();

			if (result != MAD_INTERNAL_SUCCESS)
			{
				LOGERROR(gLog, "XMassiveAdClient::FlushImpressions - %d", result);
			}
		}
		else
		{
			// Exit the zone finally
			BIZ_ASSERT(m_zoneName.Length() > 0);
			DWORD result = XMassiveAdClient::ExitZone(m_zoneName.Chars());
			if (result == MAD_INTERNAL_SUCCESS)
			{
				LOGDEBUG(gLog, "AdvertManager::Update - XMassiveAdClient::ExitZone - '%s' - Delayed", m_zoneName.Chars());
			}
			else
			{
				LOGERROR(gLog, "AdvertManager::Update - XMassiveAdClient::ExitZone - %d (%s)", result, m_zoneName.Chars());
			}
			m_zoneName.Clear();
		}
	}
#endif

	if (m_initialised && m_systemEnabled)
	{
#ifdef ENABLE_MASSIVE
		if (onLoadingScreen || !m_advertsEnabled)
		{
			XMassiveAdClient::Update(NULL);
		}
#endif

#ifdef ENABLE_IGA
		// This has to be called every frame to keep IGA happy
		rad_async_pump(&session);
#endif
	}

#ifdef ENABLE_IGA
	if (m_refreshCacheDone)
	{
		for (Btl::Vector<AdvertObject*>::ConstIterator it=m_adverts.Begin(); it != m_adverts.End(); it++)
		{
			AdvertObject& ao = *(*it);

			ao.ReadyToLoad();
		}

		m_refreshCacheDone = false;
	}
#endif

	m_lock.Exit();
}

void AdvertManager::Render(const Display::C_View* view)
{
	m_lock.Enter();

	PERF_EVENT("Advertising:Render");

	const bool isGamePaused = Framework::C_ApplicationBase::GetInstance()->IsPaused();

	// Rendering to the screen is at 1024x768 - we still work internally to the real viewport size
	float realScreenWidth = (float)view->m_Viewport.GetW();
	float realScreenHeight = (float)view->m_Viewport.GetH();

	// Massive wants to calculate all the values for themselves now, so tell them
#ifdef ENABLE_MASSIVE
	if (m_initialised && m_advertsEnabled && m_systemEnabled)
	{
		if (!isGamePaused && m_insideZone && !m_suspendMakingMoney)
		{
			static MAD_RECT viewport;
	        
			viewport.left	= view->m_Viewport.GetX();
			viewport.right	= view->m_Viewport.GetX() + view->m_Viewport.GetW();
			viewport.top	= view->m_Viewport.GetY();
			viewport.bottom	= view->m_Viewport.GetY() + view->m_Viewport.GetH();

			static MAD_UPDATE_PARAMS updateParams;

			const Display::C_Matrix& projMatrix = view->m_ViewSettings3D.GetProjMatrix();
			
#if _XDK_VER >= 11164
			updateParams.fFOVYRadians	= 2.f * atan(1.f / projMatrix._22);
#else
			updateParams.fFOVY			= 2.f * atan(1.f / projMatrix._22);
#endif

			updateParams.fZFar			= view->m_ViewSettings3D.GetFarClip();
			updateParams.fZNear			= view->m_ViewSettings3D.GetNearClip();

			updateParams.pRectViewport	= &viewport;

			const Maths::HVec3 cameraUp		= view->m_ViewSettings3D.GetCameraToWorld().AY();
			const Maths::HVec3 cameraDir	= view->m_ViewSettings3D.GetCameraToWorld().AZ();
			const Display::C_Float4& camPos	= view->m_ViewSettings3D.GetViewPosition();

			MAD_VECTOR3	cdir, cup, cpos;

			cdir.x = cameraDir.X();
			cdir.y = cameraDir.Y();
			cdir.z = cameraDir.Z();

			cup.x = cameraUp.X();
			cup.y = cameraUp.Y();
			cup.z = cameraUp.Z();

			cpos.x = camPos.x;
			cpos.y = camPos.y;
			cpos.z = camPos.z;

			updateParams.pvCameraDir	= &cdir;
			updateParams.pvCameraPos	= &cpos;
			updateParams.pvCameraUp		= &cup;

			updateParams.usScreenHeight = (USHORT)realScreenHeight;
			updateParams.usScreenWidth	= (USHORT)realScreenWidth;

			DWORD result = XMassiveAdClient::Update(&updateParams);

			if (result != MAD_INTERNAL_SUCCESS)
			{
				if (!m_errorWhileUpdating)
				{
					LOGERROR(gLog, "XMassiveAdClient::Update - %d\n", result);
					m_errorWhileUpdating = true;
				}
			}
		}
		else // Game paused - do non-impression update
		{
			DWORD result = XMassiveAdClient::Update(NULL);
			
			if (result != MAD_INTERNAL_SUCCESS)
			{
				if (!m_errorWhileUpdating)
				{
					LOGERROR(gLog, "XMassiveAdClient::Update - %d\n", result);
					m_errorWhileUpdating = true;
				}
			}
		}
	}
#endif

	// Let's render them in debug
	{
#ifndef CFG_GOLD
		Maths::RGBA blue(0, 0, 128);
		Maths::RGBA red(128, 0, 0);
		Maths::RGBA white(128, 128, 128);

		Maths::RGBA alphaSelectGreen(0, 128, 0, 100);
		Maths::RGBA alphaSelectRed(128, 0, 0, 100);
#endif

		for (Btl::Vector<AdvertObject*>::ConstIterator it=m_adverts.Begin(); it != m_adverts.End(); it++)
		{
			AdvertObject& ao = *(*it);

			ao.Update(view, isGamePaused || m_suspendMakingMoney);

			// Keep track of which textures are on-screen so we know when it's safe to switch
			if (ao.IsOnScreen() && (ao.Angle() > 0.f))
			{
				m_downloads[ao.GetDownloadIndex()].m_visible = true;
			}

#ifndef CFG_GOLD
			const float angle = ao.Angle();

			if (angle < 0.f)
				continue;

			const AdvertPosition& ape = ao.Extents();

			const float size = ao.Size();

			Maths::HVec3 cen	= ao.Centre();
			Maths::HVec3 nv		= ao.Normal();
			Maths::HVec3 cennv	= cen + nv * 4;

			if (m_initialised && g_ShowMoneyMaker)
			{
				DbUtil::DefaultDebugRenderSys()->Deferred()->SetBlendMode(DbUtil::E_BlendMode_Alpha);

				Maths::RGBA overlayColour = ao.MakingMoney() ? alphaSelectGreen : alphaSelectRed;

				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(ape.lowerLeft, ape.upperLeft, ape.upperRight, overlayColour);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(ape.lowerLeft, ape.upperRight, ape.lowerRight, overlayColour);

				// Show normal
				DbUtil::DefaultDebugRenderSys()->Deferred()->SetBlendMode(DbUtil::E_BlendMode_Solid);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(cen, cennv, blue);
			}

			DbUtil::DefaultDebugRenderSys()->Deferred()->SetBlendMode(DbUtil::E_BlendMode_Solid);

			if (m_initialised && g_ShowExtents && m_advertsEnabled && m_systemEnabled)
			{
				// TODO: Can remove this once Massive fixes up their extent code
#ifdef ENABLE_MASSIVE
				if (g_ShowMassiveCalculatedExtents)
				{
					XMassiveAdClientSubscriber3D* pmso = ao.GetMassiveSubscriber();

					if (pmso)
					{
						MAD_EXTENTS ex;
						pmso->GetExtents(&ex);

						const MAD_VECTOR2* ssmin = pmso->GetScreenSpaceMin();
						const MAD_VECTOR2* ssmax = pmso->GetScreenSpaceMax();

						float widthScale = 1024.f / realScreenWidth;
						float heightScale = 768.f / realScreenHeight;

						Maths::HVec2 c1 = Maths::HVec2(widthScale * ssmin->x, heightScale * ssmin->y);
						Maths::HVec2 c2 = Maths::HVec2(widthScale * ssmax->x, heightScale * ssmin->y);
						Maths::HVec2 c3 = Maths::HVec2(widthScale * ssmax->x, heightScale * ssmax->y);
						Maths::HVec2 c4 = Maths::HVec2(widthScale * ssmin->x, heightScale * ssmax->y);

						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(c1, c2, white);
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(c2, c3, white);
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(c3, c4, white);
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(c4, c1, white);
					}
				}
				else
#endif // ENABLE_MASSIVE
				{
					AdvertScreenExtents screenExtents;
					ao.ScreenExtents(&screenExtents);

					Maths::HVec2 ac1 = Maths::HVec2(screenExtents.min.x, screenExtents.min.y);
					Maths::HVec2 ac2 = Maths::HVec2(screenExtents.max.x, screenExtents.min.y);
					Maths::HVec2 ac3 = Maths::HVec2(screenExtents.max.x, screenExtents.max.y);
					Maths::HVec2 ac4 = Maths::HVec2(screenExtents.min.x, screenExtents.max.y);

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(ac1, ac2, white);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(ac2, ac3, white);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(ac3, ac4, white);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine(ac4, ac1, white);
				}
			}

			if (g_ShowStats)
			{
				const AdvertDownload& ad = m_downloads[ao.GetDownloadIndex()];

				if (m_initialised && m_advertsEnabled && m_systemEnabled && ad.m_currentlyShowing == 2)
				{
					AdvertScreenExtents screenExtents;
					ao.ScreenExtents(&screenExtents);

#ifdef ENABLE_MASSIVE
					XMassiveAdClientSubscriber3D*	pmso = ao.GetMassiveSubscriber();

					if (pmso)
					{
						MAD_EXTENTS ex;
						pmso->GetExtents(&ex);

						const MAD_VECTOR2* ssmin = pmso->GetScreenSpaceMin();
						const MAD_VECTOR2* ssmax = pmso->GetScreenSpaceMax();

						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(cen+Maths::HVec3(0,-1,0), Util::C_String::Format("ID(%d)\n%s\nIV(%s) A(%.1f) S(%.1f)\nMIN(%.1f,%.1f)\nMAX(%.1f,%.1f)\nOC(%s)",
							pmso->GetInvElementID(),
							ao.Name().Chars(),
							pmso->IsCalculatedInView() ? "Y" : "N", angle, size,
							ssmin->x, ssmin->y,
							ssmax->x, ssmax->y,
							ao.GetOcclusion() ? "[vis]" : "[not vis]"));
					}
					else
					{
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(cen+Maths::HVec3(0,-1,0), ao.Name().Chars());
					}
#else // ENABLE_MASSIVE
					float widthScale = realScreenWidth / 1024.f;
					float heightScale = realScreenHeight / 768.f;

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(cen+Maths::HVec3(0,-1,0), Util::C_String::Format("%s\nA(%.1f) S(%.1f)\nMIN(%.1f,%.1f)\nMAX(%.1f,%.1f)\nOC(%s)",
						ao.Name().Chars(), angle, size, widthScale * screenExtents.min.x, heightScale * screenExtents.min.y, widthScale * screenExtents.max.x, heightScale * screenExtents.max.y,
						ao.GetOcclusion() ? "[vis]" : "[not vis]"));
#endif // ENABLE_MASSIVE
				}
				else
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(cen+Maths::HVec3(0,-1,0), ao.Name().Chars());
				}
			}
#endif // CFG_GOLD
		}


		Display::BasicRender()->BindColoured3D();
		if (!g_bShowOcclusionQueries)
			Display::Device()->SetColourWriteEnable(0);

		for (Btl::Vector<AdvertObject*>::ConstIterator it=m_adverts.Begin(); it != m_adverts.End(); it++)
		{
			AdvertObject& ao = *(*it);

			ao.RenderOcclusionQuery();
		}

		Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
	}

	m_lock.Exit();
}

void AdvertManager::EnterZone(bool quickRestart, const Btl::String& levelName, S32 route, bool enabled)
{
	m_lock.Enter();

	m_validZoneName = true;

	m_suspendMakingMoney = false;

#ifdef ENABLE_MASSIVE
	if (m_flushBeforeExit > 0)
	{
		LOGDEBUG(gLog, "AdvertManager::Update - Delayed XMassiveAdClient::ExitZone");

		// Exit the zone finally
		BIZ_ASSERT(m_zoneName.Length() > 0);
		DWORD result = XMassiveAdClient::ExitZone(m_zoneName.Chars());
		if (result != MAD_INTERNAL_SUCCESS)
		{
			LOGERROR(gLog, "XMassiveAdClient::ExitZone - %d", result);
		}
		m_zoneName.Clear();

		m_flushBeforeExit = 0;
	}
#endif

	m_errorWhileUpdating = false;

	// Showroom in the UI is a special case zone that we want to ignore
	if (quickRestart || !stricmp(levelName.Chars(), "showroom"))
	{
		m_lock.Exit();
		return;
	}

	m_advertsEnabled = enabled;

	if (!m_initialised)
	{
		LOGWARN(gLog, "AdvertManager::EnterZone - System not initialised as profile has not been logged in");
	}

	m_currentRoute = route;
	m_currentAdvert = 0;

	MEMORY_TRACKER_CONTEXT("advertising");

	BIZ_ASSERT(m_zoneName.Length() == 0);
	m_zoneName.Printf("%s_%02d", levelName.Lower().Chars(), route);
	m_levelName = levelName;

	LOGDEBUG(gLog, "AdvertManager::EnterZone - Zone '%s'", m_zoneName.Chars());

	BIZ_ASSERT(m_adverts.Size() == 0);

	if (m_initialised && m_advertsEnabled && m_systemEnabled)
	{
#ifdef ENABLE_MASSIVE
		DWORD result = XMassiveAdClient::EnterZone(m_zoneName.Chars());

		if (result == MAD_INTERNAL_SUCCESS)
		{
			LOGDEBUG(gLog, "AdvertManager::EnterZone - XMassiveAdClient::EnterZone(%d)", result);
		}
		else
		{
			if (result == MAD_INTERNAL_ZONE_INVALID_NAME)
			{
				m_validZoneName = false;
			}

			LOGERROR(gLog, "XMassiveAdClient::EnterZone - %d\n", result);
		}
#endif
	}

#if 0
	float xOffset = 0.f;
	float zOffset = 0.f;
	float radius = 60.f;
	int step = 360 / 10;

	float ox = xOffset + radius * cos(0.f);
	float oz = zOffset + radius * sin(0.f);

	float height = 8.f;

	for (int iang=step; iang <= 360; iang+=step)
	{
		float cx2 = xOffset + radius * cos(float(iang) / 57.3f);
		float cz2 = zOffset + radius * sin(float(iang) / 57.3f);

		AdvertPosition apos;
		apos.lowerLeft	= Maths::HVec3(ox, -height, oz);
		apos.upperLeft	= Maths::HVec3(ox, height, oz);
		apos.upperRight	= Maths::HVec3(cx2, height, cz2);
		apos.lowerRight	= Maths::HVec3(cx2, -height, cz2);

		AdvertObject *ad1 = new AdvertObject("test_256x128", &apos);
		m_adverts.PushBack(ad1);

		ox = cx2;
		oz = cz2;
		height += .4f;
	}

	//{
	//	AdvertPosition apos;
	//	apos.lowerLeft	= Maths::HVec3(xOffset + 20.f, -10.f, zOffset + -20.f);
	//	apos.upperLeft	= Maths::HVec3(xOffset + 20.f, 10.f, zOffset + -20.f);
	//	apos.upperRight	= Maths::HVec3(xOffset + -10.f, 10.f, zOffset + 0.f);
	//	apos.lowerRight	= Maths::HVec3(xOffset + -10.f, -10.f, zOffset + 0.f);

	//	AdvertObject *ad1 = new AdvertObject("test_256x128", &apos);
	//	m_adverts.PushBack(ad1);
	//}
	//xOffset += 60.f;
	//{
	//	AdvertPosition apos;
	//	apos.lowerLeft	= Maths::HVec3(xOffset + 20.f, -10.f, zOffset + -40.f);
	//	apos.upperLeft	= Maths::HVec3(xOffset + 20.f, 10.f, zOffset + -40.f);
	//	apos.upperRight	= Maths::HVec3(xOffset + -10.f, 10.f, zOffset + 0.f);
	//	apos.lowerRight	= Maths::HVec3(xOffset + -10.f, -10.f, zOffset + 0.f);

	//	AdvertObject *ad1 = new AdvertObject("test_256x128", &apos);
	//	m_adverts.PushBack(ad1);
	//}
	//xOffset += 60.f;
	//{
	//	AdvertPosition apos;
	//	apos.lowerLeft	= Maths::HVec3(xOffset + 10.f, -10.f, zOffset + 0.f);
	//	apos.upperLeft	= Maths::HVec3(xOffset + 10.f, 10.f, zOffset + 0.f);
	//	apos.upperRight	= Maths::HVec3(xOffset + -10.f, 10.f, zOffset + 0.f);
	//	apos.lowerRight	= Maths::HVec3(xOffset + -10.f, -10.f, zOffset + 0.f);

	//	AdvertObject *ad1 = new AdvertObject("test_128x128", &apos);
	//	m_adverts.PushBack(ad1);
	//}
	//xOffset += 60.f;
	//{
	//	AdvertPosition apos;
	//	apos.lowerLeft	= Maths::HVec3(xOffset + 10.f, -10.f, zOffset + 0.f);
	//	apos.upperLeft	= Maths::HVec3(xOffset + 10.f, 10.f, zOffset + 0.f);
	//	apos.upperRight	= Maths::HVec3(xOffset + -10.f, 10.f, zOffset + 0.f);
	//	apos.lowerRight	= Maths::HVec3(xOffset + -10.f, -10.f, zOffset + 0.f);

	//	AdvertObject *ad1 = new AdvertObject("test_512x256", &apos);
	//	m_adverts.PushBack(ad1);
	//}
	//xOffset += 60.f;
	//{
	//	AdvertPosition apos;
	//	apos.lowerLeft	= Maths::HVec3(xOffset + 20.f, -10.f, zOffset + -20.f);
	//	apos.upperLeft	= Maths::HVec3(xOffset + 20.f, 10.f, zOffset + -20.f);
	//	apos.upperRight	= Maths::HVec3(xOffset + -10.f, 10.f, zOffset + 0.f);
	//	apos.lowerRight	= Maths::HVec3(xOffset + -10.f, -10.f, zOffset + 0.f);

	//	AdvertObject *ad1 = new AdvertObject("test_256x128", &apos);
	//	m_adverts.PushBack(ad1);
	//}
	//xOffset += 60.f;
	//{
	//	AdvertPosition apos;
	//	apos.lowerLeft	= Maths::HVec3(xOffset + 20.f, -10.f, zOffset + 0.f);
	//	apos.upperLeft	= Maths::HVec3(xOffset + 20.f, 10.f, zOffset + 0.f);
	//	apos.upperRight	= Maths::HVec3(xOffset + -10.f, 10.f, zOffset + -20.f);
	//	apos.lowerRight	= Maths::HVec3(xOffset + -10.f, -10.f, zOffset + -20.f);

	//	AdvertObject *ad1 = new AdvertObject("test_256x128", &apos);
	//	m_adverts.PushBack(ad1);
	//}
	//xOffset += 60.f;
	//{
	//	AdvertPosition apos;
	//	apos.lowerLeft	= Maths::HVec3(xOffset, -10.f, zOffset + 20.f);
	//	apos.upperLeft	= Maths::HVec3(xOffset, 10.f, zOffset + 20.f);
	//	apos.upperRight	= Maths::HVec3(xOffset, 10.f, zOffset + -10.f);
	//	apos.lowerRight	= Maths::HVec3(xOffset, -10.f, zOffset + -10.f);

	//	AdvertObject *ad1 = new AdvertObject("test_256x128", &apos);
	//	m_adverts.PushBack(ad1);
	//}	
	//xOffset += 60.f;
	//{
	//	AdvertPosition apos;
	//	apos.lowerLeft	= Maths::HVec3(xOffset, -10.f, zOffset + -20.f);
	//	apos.upperLeft	= Maths::HVec3(xOffset, 10.f, zOffset + -20.f);
	//	apos.upperRight	= Maths::HVec3(xOffset, 10.f, zOffset + 10.f);
	//	apos.lowerRight	= Maths::HVec3(xOffset, -10.f, zOffset + 10.f);

	//	AdvertObject *ad1 = new AdvertObject("test_256x128", &apos);
	//	m_adverts.PushBack(ad1);
	//}	

	CreateRenderData();

#endif

	m_insideZone = true;
	
	m_lock.Exit();
}

void AdvertManager::EnterZone_PostLoad(GameResource::C_LevelResource *pLevelResource, GameResource::C_LevelInstance *pLevelInstance)
{
	m_lock.Enter();


	m_pLevelInstance = pLevelInstance;

	bool noMatch(false);

	for (Btl::Vector<AdvertDownload>::Iterator it=m_downloads.Begin(); it != m_downloads.End(); ++it)
	{
		const Btl::String16 adName = it->Name();

		// Store down the material information we'll need to swap the texture into the scene later
		WorldRender::MaterialParameter mp = pLevelResource->GetRenderLevel()->GetParameter(adName.Chars());

		if (mp.index == -1)
		{
			mp = pLevelResource->GetRenderLevel()->GetParameter(it->Name(true).Chars());

			if (mp.index == -1)
			{
				LOGERROR(gLog, "AdvertManager::EnterZone_PostLoad - Can't find material for '%s' or '%s'", adName.Chars(), it->Name(true).Chars());
				noMatch = true;
			}
		}

		it->SetMaterialParam(mp);

		// If we didn't download an advert yet, we'll need to put something there in it's place, no benjamin's for us just yet alas :(
		if (!it->m_downloadedAdvert && !it->m_loadedPlaceholder)
		{
			Util::C_String placeholderName;
			placeholderName.Printf("graphics/textures/%s/%s.texture", m_levelName.Chars(), adName.Chars());

			if (Display::CreateTexture2DFromFile(placeholderName, &it->m_placeholderTexture))
			{
				it->m_loadedPlaceholder = true;
				LOGDEBUG(gLog, "AdvertManager::EnterZone_PostLoad - Loaded placeholder (%s)", adName.Chars());
			}
			else
			{
				LOGERROR(gLog, "AdvertManager::EnterZone_PostLoad - Failed to load placeholder (%s)", adName.Chars());
			}
		}
	}


#ifndef CFG_GOLD
	if (noMatch)
	{
		pLevelResource->GetRenderLevel()->DumpParameters();
	}
#endif

	FixUpDownloadedTextures();

	m_lock.Exit();
}

void AdvertManager::FixUpDownloadedTextures()
{
	m_lock.Enter();

	for (Btl::Vector<AdvertDownload>::Iterator it=m_downloads.Begin(); it != m_downloads.End(); ++it)
	{
		// Check if we've got a placeholder to delete
		if ((it->m_killCount > 0) && (--it->m_killCount == 0))
		{
			LOGDEBUG(gLog, "AdvertManager::FixUpDownloadedTextures - Deleting placeholder for '%s'", it->Name().Chars());

			if (it->m_placeholderTexture.IsCreated())
				Display::Device()->DestroyTexture2D(&it->m_placeholderTexture);
		}

		// Are we already showing the advert - or an advert is on-screen and we can't switch it
		if (it->m_currentlyShowing != 2 && !it->m_visible)
		{
			const WorldRender::MaterialParameter& mp = it->GetMaterialParam();

			// If we've got the advert but aren't showing it, or as a fallback loaded the placeholder and haven't set it yet
			if ((mp.index != -1) && (it->m_downloadedAdvert || (it->m_loadedPlaceholder && it->m_currentlyShowing == 0)))
			{
				void *pTex;

				if (it->m_downloadedAdvert)
				{
					pTex = it->m_advertTexture.GetGPUTexture();

					// We'll delete the placeholder in 3 update's time (to ensure it's not being used)
					if (it->m_loadedPlaceholder && it->m_placeholderTexture.IsCreated())
						it->m_killCount = 3;

					it->m_currentlyShowing = 2;
				}
				else
				{
					pTex = it->m_placeholderTexture.GetGPUTexture();
					it->m_currentlyShowing = 1;
				}

				BIZ_ASSERT(pTex != NULL);

				m_pLevelInstance->SetParameter(mp, &pTex);
			}
		}

		it->m_visible = false;
	}
	
	m_lock.Exit();
}

void AdvertManager::RestartZone()
{
	m_lock.Enter();

	// Need to reset the flag which controls if we insert back into the graphical mesh when we restart - don't shut anything down
	for (Btl::Vector<AdvertDownload>::Iterator it=m_downloads.Begin(); it != m_downloads.End(); ++it)
	{
		it->m_currentlyShowing = 0;
		it->m_visible = false;
		it->SetMaterialParam(WorldRender::MaterialParameter());
	}
	
	m_lock.Exit();
}

void AdvertManager::TidyZone()
{
	m_lock.Enter();

	// Tidy up the advert resources
	for (Btl::Vector<AdvertDownload>::Iterator it=m_downloads.Begin(); it != m_downloads.End(); ++it)
	{
		it->Destroy();
	}
	m_downloads.Destroy();

	m_lock.Exit();
}

// Massive - Memory leak in currently library if we ExitZone before the impressions are flushed
void AdvertManager::ExitZone(bool forceQuit)
{
	if (!m_insideZone)
		return;

	m_lock.Enter();

	MEMORY_TRACKER_CONTEXT("advertising");

	BIZ_ASSERT(m_zoneName.Length() > 0);
	
	LOGDEBUG(gLog, "AdvertManager::ExitZone - Zone '%s'", m_zoneName.Chars());

	// Destroy the advert placements (On the PS3 this also commit's the impression information - must be before submit)
	for (int i=0; i < m_adverts.Size(); i++)
	{
		delete m_adverts[i];
	}
	m_adverts.Destroy();

	if (m_initialised && m_advertsEnabled && m_systemEnabled)
	{
#ifdef ENABLE_MASSIVE
		if (m_validZoneName)
		{
			if (forceQuit)
			{
				DWORD result = XMassiveAdClient::ExitZone(m_zoneName.Chars());
				if (result == MAD_INTERNAL_SUCCESS)
				{
					LOGDEBUG(gLog, "XMassiveAdClient::ExitZone - '%s'", m_zoneName.Chars());
				}
				else
				{
					LOGERROR(gLog, "XMassiveAdClient::ExitZone - %d (%s)", result, m_zoneName.Chars());
				}
				m_zoneName.Clear();
			}
			else
			{
				// Workaround for memory leak - we delay the ExitZone and manually flush in the mean time
				m_flushBeforeExit = 10;
			}
		}
		else
		{
			m_zoneName.Clear();
		}
#endif

#ifdef ENABLE_IGA
		if (m_cells.Size() > 0)
		{
			int rci = rad_commit_impressions(&AdvertManager::Instance().Session(), m_cells.Size(), &m_cells[0]);
			if (rci == RAD_SUCCESS)
			{
				LOGDEBUG(gLog, "Advertising: rad_commit_impressions");
			}
			else
			{
				LOGERROR(gLog, "Advertising: rad_commit_impressions: failed ('%s' - %d)", IGAErrorToString(rci), rci);
			}

			int rc = rad_send_impressions(&session, my_sendcompletefunc, (void*)"myuserdata");
			if (rc == RAD_SUCCESS)
			{
				LOGDEBUG(gLog, "AdvertManager::ExitZone - rad_send_impressions");
			}
			else
			{
				LOGERROR(gLog, "Advertising: rad_send_impressions: failed ('%s' - %d)", IGAErrorToString(rc), rc);
			}
		}
		m_zoneName.Clear();
#endif
	}
	else if (!m_advertsEnabled || !m_systemEnabled)
	{
		m_zoneName.Clear();
	}

#ifdef ENABLE_IGA
	m_cells.Destroy();
#endif
	
	m_advertResource.quadArray.Destroy();

	DestroyRenderingData();

	m_insideZone = false;
	m_advertsEnabled = false;
	m_validZoneName = false;

	m_lock.Exit();
}

void AdvertManager::ResumeMakingMoney()
{
	m_suspendMakingMoney = false;
}

void AdvertManager::SuspendMakingMoney()
{
	m_suspendMakingMoney = true;
}

/*
	Create the data we use to render the visual adverts on the screen
*/
void AdvertManager::CreateRenderData()
{
	if (!m_adverts.Size())
		return;

	m_vertexCount	= m_adverts.Size() * 4;		// 4 corners per advert
	m_indexCount	= m_adverts.Size() * 2 * 3;	// 2 triangles per advert

	if (!Display::Device()->CreateVertexBuffer(m_vertexCount * sizeof(Display::Vertex_XYZ_C_UV), 0, &m_vertexBuffer))
		return;

	volatile Display::Vertex_XYZ_C_UV * pVertices = (Display::Vertex_XYZ_C_UV*) m_vertexBuffer.Lock();
	
	if (pVertices)
	{
		Btl::Vector<AdvertObject*>::ConstIterator it = m_adverts.Begin();

		for (; it != m_adverts.End(); it++)
		{
			const AdvertPosition& ape = (*it)->Extents();

			memcpy((void*)&pVertices->x, &ape.lowerLeft, 12);
#if 0 | defined(PLATFORM_PC)
			pVertices->c = 0xff888888;
#else
			pVertices->c = 0x888888ff;
#endif
			pVertices->u = 0.f;
			pVertices->v = 1.f;
			pVertices++;
			memcpy((void*)&pVertices->x, &ape.upperLeft, 12);
#if 0 | defined(PLATFORM_PC)
			pVertices->c = 0xff888888;
#else
			pVertices->c = 0x888888ff;
#endif
			pVertices->u = 0.f;
			pVertices->v = 0.f;
			pVertices++;
			memcpy((void*)&pVertices->x, &ape.upperRight, 12);
#if 0 | defined(PLATFORM_PC)
			pVertices->c = 0xff888888;
#else
			pVertices->c = 0x888888ff;
#endif
			pVertices->u = 1.f;
			pVertices->v = 0.f;
			pVertices++;
			memcpy((void*)&pVertices->x, &ape.lowerRight, 12);
#if 0 | defined(PLATFORM_PC)
			pVertices->c = 0xff888888;
#else
			pVertices->c = 0x888888ff;
#endif
			pVertices->u = 1.f;
			pVertices->v = 1.f;
			pVertices++;
		}

		m_vertexBuffer.Unlock();
	}

	if (!Display::Device()->CreateIndexBuffer(m_indexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_indexBuffer))
		return;

	volatile U16 * pIndices = (U16 *) m_indexBuffer.Lock();
	if (pIndices)
	{
		int ic(0);

		for (int i=0; i < m_adverts.Size(); i++)
		{
			const int advertOffset = i * 4;

			pIndices[ic++] = 0 + advertOffset;
			pIndices[ic++] = 1 + advertOffset;
			pIndices[ic++] = 2 + advertOffset;
			
			pIndices[ic++] = 0 + advertOffset;
			pIndices[ic++] = 2 + advertOffset;
			pIndices[ic++] = 3 + advertOffset;
		}
		m_indexBuffer.Unlock();
	}
}

void AdvertManager::DestroyRenderingData()
{
	if (m_vertexBuffer.IsCreated())
		Display::Device()->DestroyVertexBuffer(&m_vertexBuffer);

	if (m_indexBuffer.IsCreated())
		Display::Device()->DestroyIndexBuffer(&m_indexBuffer);
}

// Pre-warn the system of all the different placements
int AdvertManager::PrimeAdvertDownload(unsigned int aindex)
{
	int c(0);

	// Look for us already having it
	for (Btl::Vector<AdvertDownload>::ConstIterator it=m_downloads.Begin(); it != m_downloads.End(); ++it, c++)
	{
		if (it->m_advertIndex == aindex)
			return c;
	}

	// Need to add to the list
	m_downloads.PushBack();

	AdvertDownload& advert = m_downloads.Back();

	advert.m_advertIndex = aindex;

	return m_downloads.Size()-1;
}

void AdvertDownload::Destroy()
{
	if (m_advertTexture.IsCreated())
		Display::Device()->DestroyTexture2D(&m_advertTexture);

	if (m_placeholderTexture.IsCreated())
		Display::Device()->DestroyTexture2D(&m_placeholderTexture);
}

Btl::String16 AdvertDownload::Name(bool alternateName) const
{
	Btl::String16 r;

	r.Printf((char*)(!alternateName ? "advert%d" : "advert%02d"), m_advertIndex);

	return r;
}

#if defined(ENABLE_IGA) || (defined(ENABLE_MASSIVE) && defined(PLATFORM_PC))

bool AdvertManager::TextureDownloaded(int index, const U8* textureData, size_t textureLength)
{
	AdvertDownload& advert = m_downloads[index];

	const Btl::String16& advertName = advert.Name();

	if (advert.m_downloadedAdvert)
	{
		LOGDEBUG(gLog, "AdvertManager::TextureDownloaded - name(%s) - MATCH", advertName.Chars());
		return false;
	}

	LOGDEBUG(gLog, "AdvertManager::TextureDownloaded - name(%s) - ADDING - length(%d)", advertName.Chars(), textureLength);

	Display::C_TextureParams params;

	if (Display::DDS::LoadTextureFromMemory((U8*)textureData, textureLength, &params))
	{
		LOGDEBUG(gLog, "AdvertManager::TextureDownloaded - %d x %d", params.m_Width, params.m_Height);

		if (Display::Device()->CreateTexture2D(params.m_Width, params.m_Height, params.m_Levels, params.m_Flags, params.m_Format, &advert.m_advertTexture))
		{
			LOGDEBUG(gLog, "AdvertManager::TextureDownloaded - Created ok");

			if (Display::UploadTexture2D(&advert.m_advertTexture, &params))
			{
				LOGDEBUG(gLog, "AdvertManager::TextureDownloaded - Uploaded ok");
				advert.m_downloadedAdvert = true;

				return true;
			}
			else
			{
				LOGERROR(gLog, "AdvertManager::TextureDownloaded - Upload problem");
			}
		}
		else
		{
			LOGERROR(gLog, "AdvertManager::TextureDownloaded - Texture problem");
		}
	}
	else
	{
		LOGERROR(gLog, "AdvertManager::TextureDownloaded - Texture failed to load");
	}

	return false;
}

#endif

void AdvertManager::LoadAdvertResource(Sys::I_File& file)
{
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(file);
	m_advertResource.quadArray.Destroy();
	ar >> m_advertResource;

	// Build advert subscriber objects.	
	U32 routeMask = 1 << m_currentRoute;

	int adCount(0);

	for (AdvertResource::AdvertQuadArray::ConstIterator it = m_advertResource.quadArray.Begin(); it != m_advertResource.quadArray.End(); ++it)
	{
		if (it->routeMask & routeMask)
			adCount++;
	}

#ifdef ENABLE_IGA
	m_cells.Resize(adCount);
#endif

	int adIndex(0);

	for (AdvertResource::AdvertQuadArray::ConstIterator it = m_advertResource.quadArray.Begin(); it != m_advertResource.quadArray.End(); ++it)
	{
		if (it->routeMask & routeMask)
		{
			//FIXME: unnecessary data duplication
			AdvertPosition p;
			p.lowerLeft  = Maths::Load<Maths::HVec3>(&it->v[0]);
			p.upperLeft  = Maths::Load<Maths::HVec3>(&it->v[1]);
			p.upperRight = Maths::Load<Maths::HVec3>(&it->v[2]);
			p.lowerRight = Maths::Load<Maths::HVec3>(&it->v[3]);

			m_lock.Enter();

			int downloadIndex = PrimeAdvertDownload(it->placementIndex);

			Btl::String32 id;
			id.Printf("advert%d", it->placementIndex);

			AdvertObject *pAd = new AdvertObject(id.Chars(), &p, downloadIndex
#ifdef ENABLE_IGA
				, &m_cells[adIndex++]
#endif
				);
			m_adverts.PushBack(pAd);

			m_lock.Exit();

			LOGDEBUG(gLog, "AdvertManager::LoadAdvertResource - Adding '%s' - di(%d) - (%.1f,%.1f,%.1f)->(%.1f,%.1f,%.1f)", id.Chars(), downloadIndex,
				(float)p.lowerLeft.X(), (float)p.lowerLeft.Y(), (float)p.lowerLeft.Z(),
				(float)p.upperRight.X(), (float)p.upperRight.Y(), (float)p.upperRight.Z());
		}
	}

#ifdef ENABLE_IGA
	if (AdvertManager::Instance().IsInitialised() && AdvertManager::Instance().AdvertsEnabled() && AdvertManager::Instance().SystemEnabled())
	{
		m_lock.Enter();

		if (m_cells.Size() > 0)
		{
			rad_refresh_cache(&session, m_cells.Size(), &m_cells[0], 0, my_progressfunc, my_refreshCacheCompleteFunc, this);
		}

		m_lock.Exit();
	}
#endif


#ifndef CFG_GOLD
	CreateRenderData();
#endif
}

#ifndef CFG_GOLD

void AdvertManager::GotoAdvert()
{
	if (m_adverts.Size() > 0)
	{
		const AdvertObject&		ao		= *(m_adverts[m_currentAdvert]);
		const AdvertPosition&	extents = ao.Extents();

		float x = extents.upperLeft.X() - extents.upperRight.X();
		float z = extents.upperLeft.Z() - extents.upperRight.Z();

		float width = sqrtf(x*x + z*z) * .7f;
		float height = extents.upperLeft.Y() - extents.lowerLeft.Y();

		float size = Maths::Max(width, height);

		Cam2::C_DebugFeed& debugFeed = Framework::C_ApplicationBase::GetInstance()->GetDebugFeed(0);

		debugFeed.SetDebugType(Cam2::E_FlyCamera);

		Cam2::C_Camera cam;
		cam.SetPosition(ao.Centre() + ao.Normal() * size * 2);
		cam.LookAt(ao.Centre());

		debugFeed.CopyFrom(cam); 
	}
}

void AdvertManager::NextAdvert()
{
	if (m_adverts.Size() > 0)
	{
		m_currentAdvert = (U16) ((m_currentAdvert+1) % m_adverts.Size());
		GotoAdvert();
	}
}

void AdvertManager::PrevAdvert()
{
	if (m_adverts.Size() > 0)
	{
		m_currentAdvert = (U16) ((m_currentAdvert + m_adverts.Size() - 1) % m_adverts.Size());
		GotoAdvert();
	}
}

void GotoCurrentAdvert(void *)
{
	AdvertManager::Instance().GotoAdvert();
}

void PrevAdvert(void *)
{
	AdvertManager::Instance().PrevAdvert();
}

void NextAdvert(void *)
{
	AdvertManager::Instance().NextAdvert();
}


Debug::Function g_AdvertGotoCurrent("System.Advertising.GotoCurrentAdvert", GotoCurrentAdvert, NULL);
Debug::Function g_AdvertPrev("System.Advertising.PreviousAdvert", PrevAdvert, NULL);
Debug::Function g_AdvertNext("System.Advertising.NextAdvert", NextAdvert, NULL);


#endif


}
