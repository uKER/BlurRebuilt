#include "AdvertisingPch.h"

#include "Advertising/AdvertManager.h"
#include "Advertising/AdvertObject.h"

#ifdef ENABLE_MASSIVE
#include "XMassiveAdClient.h"
#endif

#include <Display/Device/Device.h>

//#pragma optimize("", off)

namespace Advertising
{

	extern Debug::Logger &gLog;

#ifdef ENABLE_IGA
	void my_progressfunc(RADSESSION *sess, int progress, int total, void *userdata);
	void my_refreshCacheCompleteFunc(RADSESSION *sess, int rc, void *userdata);

	const char* IGAErrorToString(int rc);
#endif

#ifdef ENABLE_MASSIVE

using namespace MassiveAdClient3;

//--------------------------------------------------------------------------------------
// Name: TextureReplacementFunction
// Desc: The callback function that gets passed to the Massive subscribers
//       and will be called when new media is downloaded
//--------------------------------------------------------------------------------------
static BOOL TextureReplacementFunction(MAD_MEDIA_PARAMS *pRequiredParams, MAD_MEDIA_OPTIONAL *pOptionalParams, void *pGameObject)
{
	return FALSE;
}

//--------------------------------------------------------------------------------------
// Name: OcclusionCallbackFunction
// Desc: This callback, when set on a subscriber, will be periodically
//       called to allow the game to inform Massive if a subscriber is occluded
//--------------------------------------------------------------------------------------
BOOL OcclusionCallbackFunction(MAD_OCCLUSION_PARAMS *pOcclusionsParams)
{
	AdvertObject *p = (AdvertObject*)pOcclusionsParams->pState;
	return p->GetOcclusion() ? FALSE:TRUE;
}

#endif

bool AdvertObject::GetOcclusion()
{
	return m_bVisible;
}

void AdvertObject::RenderOcclusionQuery() const
{
	// Massive would like to track all placements - even if we don't download textures successfully
#if !defined(ENABLE_MASSIVE)
	if (m_downloadIndex < 0 || m_downloadIndex >= AdvertManager::Instance().GetNumDownloads())
	{
		m_bVisible = false;
		return;
	}

	// Check if we've got a proper texture downloaded, else it's a waste of time doing this
	const AdvertDownload& dlTexture = AdvertManager::Instance().GetDownloadTexture(m_downloadIndex);
	if (dlTexture.m_currentlyShowing != 2)
	{
		m_bVisible = false;
		return;
	}
#endif

#ifdef ADVERT_CALCULATE_SCREEN_SIZE
	if (m_screenSize < 2)
	{
		m_bVisible = false;
		return;
	}
#else
	if (!m_onScreen)
	{
		m_bVisible = false;
		return;
	}
#endif

	const AdvertPosition& ape = Extents();

	if (m_bFirstTime)
	{
		m_pOcclusionQuery1->Create();
		m_pOcclusionQuery2->Create();
		m_bFirstTime = false;
	}
	else
	{
		F32 fOcclusion = 0;
		Display::E_OcclusionQueryResult result1 = m_pOcclusionQuery1->Visible(fOcclusion);
		Display::E_OcclusionQueryResult result2 = m_pOcclusionQuery2->Visible(fOcclusion);
		
		if (result1 != Display::E_OcclusionQueryResult_Failed && result2 != Display::E_OcclusionQueryResult_Failed)
		{
			m_bVisible = (result1 == Display::E_OcclusionQueryResult_Visible && result2 == Display::E_OcclusionQueryResult_Visible);
		}
	}
 
	m_pOcclusionQuery1->Begin();
	const U32 vertexCount = 6;
	Display::C_BasicRenderColouredVertex3D* pVertex = (Display::C_BasicRenderColouredVertex3D*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, vertexCount, sizeof(Display::C_BasicRenderColouredVertex3D), Display::BasicRender()->m_pVertexDeclaration[Display::E_BasicRenderVertexShader_Coloured3D], true); // This can fail
	if(pVertex)
	{
		memcpy(pVertex, m_pVertices, sizeof(*m_pVertices)*6);
		Display::Device()->EndVertices(pVertex + 6);
	}
	m_pOcclusionQuery1->End();

	m_pOcclusionQuery2->Begin();
	pVertex = (Display::C_BasicRenderColouredVertex3D*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, vertexCount, sizeof(Display::C_BasicRenderColouredVertex3D), Display::BasicRender()->m_pVertexDeclaration[Display::E_BasicRenderVertexShader_Coloured3D], true); // This can fail
	if(pVertex)
	{
		memcpy(pVertex, m_pVertices+6, sizeof(*m_pVertices)*6);
		Display::Device()->EndVertices(pVertex + 6);
	}
	m_pOcclusionQuery2->End();
}

AdvertObject::AdvertObject(const char* identName, const AdvertPosition* extents, int adIndex
#ifdef ENABLE_IGA
		, RADCELL* pCell
#endif
						   ) :
#ifdef ADVERT_CALCULATE_ANGLE_MANUALLY
	m_angle(-1.f),
#endif
#ifdef ENABLE_IGA
	m_pCell(pCell),
#endif
	m_onScreen(false),
	m_downloadIndex(adIndex),
	m_bVisible(false),
	m_bFirstTime(true)
{
	m_identName = identName;

	m_pOcclusionQuery1 = new Display::C_OcclusionQuery;
	m_pOcclusionQuery2 = new Display::C_OcclusionQuery;
	m_pVertices = new Display::C_BasicRenderColouredVertex3D[12];

	memcpy(&m_extents, extents, sizeof(AdvertPosition));

	// build occlusion query vertex data
	const AdvertPosition& ape = Extents();

	Maths::HVec3 vCenter = (ape.lowerLeft + ape.upperRight) * 0.5f;

	Maths::HVec3 vNormal = (ape.lowerLeft - ape.upperLeft) ^ (ape.upperLeft - ape.upperRight);
	vNormal = Maths::Normalise(vNormal) * 0.5f;

	Maths::HVec3 vLL = (ape.lowerLeft + vCenter) * 0.5f + vNormal;
	Maths::HVec3 vLR = (ape.lowerRight + vCenter) * 0.5f + vNormal;
	Maths::HVec3 vUL = (ape.upperLeft + vCenter) * 0.5f + vNormal;
	Maths::HVec3 vUR = (ape.upperRight + vCenter) * 0.5f + vNormal;

	vCenter += vNormal;

	Display::C_BasicRenderColouredVertex3D vertex[12] = 
	{
		{vLL.X(), vLL.Y(), vLL.Z(), 1.f, 1.f, 1.f, 1.f},
		{vUL.X(), vCenter.Y(), vUL.Z(), 1.f, 1.f, 1.f, 1.f},
		{vCenter.X(), vCenter.Y(), vCenter.Z(), 1.f, 1.f, 1.f, 1.f},
		{vCenter.X(), vLR.Y(), vCenter.Z(), 1.f, 1.f, 1.f, 1.f},

		{vCenter.X(), vCenter.Y(), vCenter.Z(), 1.f, 1.f, 1.f, 1.f},
		{vCenter.X(), vUL.Y(), vCenter.Z(), 1.f, 1.f, 1.f, 1.f},
		{vUR.X(), vUR.Y(), vUR.Z(), 1.f, 1.f, 1.f, 1.f},
		{vLR.X(), vCenter.Y(), vLR.Z(), 1.f, 1.f, 1.f, 1.f},
	};

	Display::C_BasicRenderColouredVertex3D *pVertex = m_pVertices;

	*(pVertex++) = vertex[0]; *(pVertex++) = vertex[1]; *(pVertex++) = vertex[2];
	*(pVertex++) = vertex[0]; *(pVertex++) = vertex[2]; *(pVertex++) = vertex[3];
	*(pVertex++) = vertex[4]; *(pVertex++) = vertex[5]; *(pVertex++) = vertex[6];
	*(pVertex++) = vertex[4]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[7];

	m_centre = (m_extents.lowerLeft + m_extents.upperLeft + m_extents.upperRight + m_extents.lowerRight) / 4;

#ifdef ADVERT_CALCULATE_ANGLE_MANUALLY
	// Calculate normal
	{
		Maths::HVec3 v1 = m_extents.upperLeft - m_extents.lowerLeft;
		Maths::HVec3 v2 = m_extents.upperRight - m_extents.lowerLeft;

		Maths::HVec3 crossp = v1 ^ v2;

		m_normal = Maths::Normalise(crossp);
	}
#endif

#ifdef ENABLE_MASSIVE
	m_pSubscriberObject = NULL;

	if (AdvertManager::Instance().IsInitialised() && AdvertManager::Instance().AdvertsEnabled() && AdvertManager::Instance().SystemEnabled())
	{
		if (AdvertManager::Instance().ValidZone())
		{
			MAD_EXTENTS madExtents;

			Maths::Store(m_extents.lowerLeft,	(MathsStorage::Float3*) &madExtents.vLowerLeft);
			Maths::Store(m_extents.upperLeft,	(MathsStorage::Float3*) &madExtents.vUpperLeft);
			Maths::Store(m_extents.upperRight,	(MathsStorage::Float3*) &madExtents.vUpperRight);
			Maths::Store(m_extents.lowerRight,	(MathsStorage::Float3*) &madExtents.vLowerRight);

			m_pSubscriberObject = new XMassiveAdClientSubscriber3D(identName, (const MAD_EXTENTS*)&madExtents, TextureReplacementFunction, this);
			LOGDEBUG(gLog, "AdvertObject - Created XMassiveAdClientSubscriber3D");

			if (m_pSubscriberObject && m_pSubscriberObject->GetLastError() == MAD_INTERNAL_SUCCESS)
			{
				// Set the occlusion callback function for the subscriber
				m_pSubscriberObject->SetOcclusionFunction(OcclusionCallbackFunction);
			}
			else
			{
				LOGERROR(gLog, "Advertising: XMassiveAdClientSubscriber3D - Error creating subscriber(%s)", identName);

				// The subscriber couldn't be allocated or encountered an error during construction.  You should report
				// this issue appropriately (for example, by logging it to your game's log file).
				XMassiveAdClient::Log(LOG_LEVEL_ERROR, "GameObject", "Error creating subscriber");
			}
		}
	}
	else
	{
		LOGDEBUG(gLog, "AdvertObject - Advert system not initialised - not creating object");
	}
#endif

#ifdef ENABLE_IGA
	memset(m_pCell, 0, sizeof(*m_pCell));

	if (AdvertManager::Instance().IsInitialised() && AdvertManager::Instance().AdvertsEnabled() && AdvertManager::Instance().SystemEnabled())
	{
		const char* zoneName = AdvertManager::Instance().ZoneName().Chars();

		int rc = rad_init_cell(&AdvertManager::Instance().Session(), m_pCell, zoneName, identName);
		if (rc == RAD_SUCCESS)
		{
			LOGDEBUG(gLog, "Advertising: rad_init_cell('%s'-'%s')", zoneName, identName);
		}
		else
		{
			LOGWARN(gLog, "Advertising: rad_init_cell('%s'-'%s'): failed ('%s' - %d)", zoneName, identName, IGAErrorToString(rc), rc);
		}
	}
#endif
}

#ifdef ENABLE_IGA

void AdvertObject::ReadyToLoad()
{
	// Try and load something
	static int decrypt_count(0);

	// select and load content
	LOGDEBUG(gLog, "Advertising: rad_select_content(%s)", m_identName.Chars());

	int rc = rad_select_content(&AdvertManager::Instance().Session(), 1, m_pCell);

	if (m_pCell->result == RAD_SUCCESS)
	{
		LOGDEBUG(gLog, "Advertising: content_mimetype=[%s], size[%d], filesize[%d]", m_pCell->content_mimetype, m_pCell->content_size, m_pCell->content_filesize);

		int fd(0);

		// read the content file
		CellFsErrno err = cellFsOpen(m_pCell->content_filename, CELL_FS_O_RDONLY, &fd, NULL, 0);

		if (err != CELL_FS_SUCCEEDED)
		{
			LOGERROR(gLog, "Advertising: cellFsOpen failed (%d)", err);
			return;
		} 

		U8* textureData = new U8[m_pCell->content_filesize];

		uint64_t nread(0);

		err = cellFsRead(fd, textureData, m_pCell->content_filesize, &nread);
		if (err != CELL_FS_SUCCEEDED)
		{
			delete [] textureData;
			LOGERROR(gLog, "Advertising: cellFsRead failed (%d)", err);
			return;
		}
		
		err = cellFsClose(fd);
		if (err != CELL_FS_SUCCEEDED)
		{
			delete [] textureData;
			LOGERROR(gLog, "Advertising: cellFsClose failed (%d)", err);
			return;
		}

		// decrypt the content
		rc = rad_decrypt_content(&AdvertManager::Instance().Session(), textureData, m_pCell->content_filesize);

		if (rc != RAD_SUCCESS)
		{
			delete [] textureData;
			LOGERROR(gLog, "Advertising: rad_decrypt_content failed ('%s' - %d)", IGAErrorToString(rc), rc);
			return;
		}

		LOGDEBUG(gLog, "Advertising: [%d] decrypts successful", ++decrypt_count);

		LOGDEBUG(gLog, "Advertising: ReadyToLoad: %s", m_identName.Chars());
		LOGDEBUG(gLog, "Advertising: ReadyToLoad: pMediaData(0x%x) uDataLength(%d)", textureData, m_pCell->content_size-1);

		bool created = AdvertManager::Instance().TextureDownloaded(m_downloadIndex, textureData, m_pCell->content_size-1);

		delete [] textureData;
	}
	else
	{
		LOGERROR(gLog, "Advertising: rad_select_content failed ('%s' - %d)", IGAErrorToString(rc), rc);
	}
}

#endif

AdvertObject::~AdvertObject()
{
#ifdef ENABLE_MASSIVE
	delete m_pSubscriberObject;
#endif

	m_pOcclusionQuery1->Destroy();
	m_pOcclusionQuery2->Destroy();
	delete m_pOcclusionQuery1;
	delete m_pOcclusionQuery2;
	delete [] m_pVertices;
}

const Btl::String16 AdvertObject::Name() const
{
	return m_identName;
}

Maths::HVec3 AdvertObject::Normal() const
{
#ifdef ADVERT_CALCULATE_ANGLE_MANUALLY
	return m_normal;
#elif defined(ENABLE_MASSIVE)
	MAD_VECTOR3 norm;

	if (!m_pSubscriberObject)
		return Maths::HVec3(0.f, 0.f, 0.f);

	m_pSubscriberObject->GetNormal(&norm);

	return Maths::HVec3(norm.x, norm.y, norm.z);
#else
	return Maths::HVec3(0.f, 0.f, 0.f);
#endif
}

float AdvertObject::Angle() const
{
#if defined(ENABLE_MASSIVE)
	return m_pSubscriberObject ? m_pSubscriberObject->GetCalculatedAngle() : 0.f;
#else
	return m_angle;
#endif
}

float AdvertObject::Size() const
{
#if defined(ADVERT_CALCULATE_SCREEN_SIZE)
	return m_screenSize;
#elif defined(ENABLE_MASSIVE)
	return m_pSubscriberObject ? (float)m_pSubscriberObject->GetCalculatedSize() : 0.f;
#else
	return -1.f;
#endif
}

void AdvertObject::ScreenExtents(AdvertScreenExtents* out) const
{
	// TODO: Put back in when Massive works properly again
#if 0 // defined(ENABLE_MASSIVE)
	const MAD_VECTOR2* ssmin = m_pSubscriberObject->GetScreenSpaceMin();
	const MAD_VECTOR2* ssmax = m_pSubscriberObject->GetScreenSpaceMax();

	out->min.x = ssmin->x;
	out->min.y = ssmin->y;
	out->max.x = ssmin->x;
	out->max.y = ssmin->y;
#else
	*out = m_screenExtents;
#endif
}

static const Maths::HF32 screenWidth = 1024.f;
static const Maths::HF32 screenHeight = 768.f;
static const F32 fScreenWidth = 1024.f;
static const F32 fScreenHeight = 768.f;
static const Maths::HF32 vZero(0.f);

void AdvertObject::Update(const Display::C_View* view, bool isPaused)
{
	const Display::C_ViewSettings3D& viewSettings3D = view->m_ViewSettings3D;

	bool isBehind1, isBehind2, isBehind3, isBehind4;

	// These co-ordinates are always in the range 1024x768
	Maths::HVec3 sp1 = viewSettings3D.TransformWorldToWindow(m_extents.lowerLeft, &isBehind1);
	Maths::HVec3 sp2 = viewSettings3D.TransformWorldToWindow(m_extents.upperLeft, &isBehind2);
	Maths::HVec3 sp3 = viewSettings3D.TransformWorldToWindow(m_extents.upperRight, &isBehind3);
	Maths::HVec3 sp4 = viewSettings3D.TransformWorldToWindow(m_extents.lowerRight, &isBehind4);

    // Ensure that the object reports as being onscreen only if it's x and y
    // values fall within screenspace dimensions
    // (TransformToScreen() doesn't perform this check)

	if ( (!isBehind1 && sp1.X() >= vZero && sp1.X() <= screenWidth && 
						sp1.Y() >= vZero && sp1.Y() <= screenHeight) || 
	     (!isBehind2 && sp2.X() >= vZero && sp2.X() <= screenWidth && 
						sp2.Y() >= vZero && sp2.Y() <= screenHeight) || 
	     (!isBehind3 && sp3.X() >= vZero && sp3.X() <= screenWidth && 
						sp3.Y() >= vZero && sp3.Y() <= screenHeight) || 
	     (!isBehind4 && sp4.X() >= vZero && sp4.X() <= screenWidth && 
						sp4.Y() >= vZero && sp4.Y() <= screenHeight) )
	{
		m_onScreen = true;

		const Display::C_Float4& viewPosO = viewSettings3D.GetViewPosition();
		Maths::HVec3 viewPos = Maths::HVec3(viewPosO.x, viewPosO.y, viewPosO.z);
		Maths::HVec3 normalisedDir = Maths::Normalise(viewPos - m_centre);

	#ifdef ADVERT_CALCULATE_ANGLE_MANUALLY
		m_angle = Maths::DotProduct<Maths::HVec3>(normalisedDir, m_normal);
	#endif

		m_screenExtents.min.x = Maths::Min(sp1.X(), Maths::Min(sp2.X(), Maths::Min(sp3.X(), sp4.X())));
		m_screenExtents.min.y = Maths::Min(sp1.Y(), Maths::Min(sp2.Y(), Maths::Min(sp3.Y(), sp4.Y())));
		m_screenExtents.max.x = Maths::Max(sp1.X(), Maths::Max(sp2.X(), Maths::Max(sp3.X(), sp4.X())));
		m_screenExtents.max.y = Maths::Max(sp1.Y(), Maths::Max(sp2.Y(), Maths::Max(sp3.Y(), sp4.Y())));

		// Clamp the coordinates to the screen
		m_screenExtents.min.x = Maths::Max(m_screenExtents.min.x, 0.0f);
		m_screenExtents.min.y = Maths::Max(m_screenExtents.min.y, 0.0f);
		m_screenExtents.max.x = Maths::Min(m_screenExtents.max.x, fScreenWidth);
		m_screenExtents.max.y = Maths::Min(m_screenExtents.max.y, fScreenHeight);

		float dx		= m_screenExtents.max.x - m_screenExtents.min.x;
		float dy		= m_screenExtents.max.y - m_screenExtents.min.y;

	#ifdef ADVERT_CALCULATE_SCREEN_SIZE
		m_screenSize	= m_onScreen ? (dx*dy) * (100.f/(fScreenWidth*fScreenHeight)) : 0.f;
	#endif
	}
	else
	{
		m_onScreen = false;
	#ifdef ADVERT_CALCULATE_SCREEN_SIZE
		m_screenSize = 0;
	#endif
	}


#if defined(ENABLE_MASSIVE)
	const MAD_VIEW_STATE vs = m_pSubscriberObject ? m_pSubscriberObject->GetViewState() : VIEW_STATE_INVALID;

	m_makingMoney = (vs == VIEW_STATE_IMPRESSION);
#elif defined(ENABLE_IGA)
	// Impression = Up to 80 degrees deflection and over 2% of the screen and occlusion tests say we can see most of it
	m_makingMoney = (Angle() >= 0.174f) && (Size() >= 2.f) && m_bVisible && !isPaused;
#else
	m_makingMoney = false;
#endif

#ifdef ENABLE_IGA
	// Update the impression information while we're generating money, or one last time as it stops being useful
	if (AdvertManager::Instance().AdvertsEnabled() && AdvertManager::Instance().SystemEnabled() && (m_makingMoney || (m_prevMakingMoney != m_makingMoney)))
	{
		// If we ever stop making money we need to send a size of Zero for the last call to finish off our reporting information
		if (!m_makingMoney)
		{
			m_screenSize = 0.f;
		}

		LOGDEBUG(gLog, "AdvertObject::Update: rad_track_cell: name(%s) ss(%f) a(%f)", m_identName.Chars(), m_screenSize, m_angle);
		int rc = rad_track_cell(&AdvertManager::Instance().Session(), m_pCell, m_screenSize, m_angle, RAD_ANGLE_COS);

		if((rc & RAD_TRACKCELL_ROTATE) > 0)
		{
			LOGINFO(gLog, "AdvertObject::Update: rad_track_cell: received RAD_TRACKCELL_ROTATE notification");
		}
	}
#endif
	
	m_prevMakingMoney = m_makingMoney;
}

}
