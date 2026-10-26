#ifndef ADVERTOBJECT_H
#define ADVERTOBJECT_H

#ifdef _MSC_VER
#pragma once
#endif

#define ENABLE_ADVERT_DEBUG

#include <core/utmaths.h>
#include "Advertising\Advertising.h"

#include <Display/Texture/Texture2D.h>
#include <display\occlusion\OcclusionQuery.h>

#if defined(ENABLE_MASSIVE)						// Massive

namespace MassiveAdClient3
{
	class XMassiveAdClientSubscriber3D;
}

#elif defined(ENABLE_IGA)						// IGA

#include "radial.h"

#define ADVERT_CALCULATE_ANGLE_MANUALLY
#define ADVERT_CALCULATE_SCREEN_SIZE

#else											// None

#define ADVERT_CALCULATE_ANGLE_MANUALLY
#define ADVERT_STORE_AND_CALC_NORMAL
#define ADVERT_CALCULATE_SCREEN_SIZE

#endif

namespace Display
{
	class C_View;
}

namespace Advertising
{

struct AdvertPosition
{
	Maths::HVec3	lowerLeft;
	Maths::HVec3	upperLeft;
	Maths::HVec3	upperRight;
	Maths::HVec3	lowerRight;
};

struct AdvertScreenExtents
{
	MathsStorage::Float2 min;
	MathsStorage::Float2 max;
};

class AdvertObject
{
protected:

#ifdef ENABLE_MASSIVE
    // Pointer to the subscriber object
	MassiveAdClient3::XMassiveAdClientSubscriber3D*	m_pSubscriberObject;
#endif

#ifdef ENABLE_IGA
	RADCELL*				m_pCell;
#endif

	// TODO: For Massive we only need this for debugging really - we can get it on demand from their library
	AdvertPosition			m_extents;
	
	Maths::HVec3			m_centre;

	Btl::String16			m_identName;

#ifdef ADVERT_CALCULATE_ANGLE_MANUALLY
	float					m_angle;
	Maths::HVec3			m_normal;
#endif

#ifdef ADVERT_CALCULATE_SCREEN_SIZE
	float					m_screenSize;
#endif

	AdvertScreenExtents		m_screenExtents;

	int						m_downloadIndex;

	Display::C_BasicRenderColouredVertex3D *m_pVertices;

	bool					m_onScreen;
	bool					m_makingMoney;
	bool					m_prevMakingMoney;
	mutable bool			m_bFirstTime;

	Display::C_OcclusionQuery *m_pOcclusionQuery1;
	Display::C_OcclusionQuery *m_pOcclusionQuery2;
	mutable bool			m_bVisible;

public:
	AdvertObject(const char* identName, const AdvertPosition* extents, int adIndex
#ifdef ENABLE_IGA
		, RADCELL* pCell
#endif
		);
	~AdvertObject();

	const AdvertPosition& Extents() const				{ return m_extents; }
	const Btl::String16 Name() const;

	Maths::HVec3 Centre() const							{ return m_centre; }
	Maths::HVec3 Normal() const;

	float Angle() const;
	float Size() const;

	bool IsOnScreen() const								{ return m_onScreen; }

	bool MakingMoney() const							{ return m_makingMoney; }

	int GetDownloadIndex() const						{ return m_downloadIndex; }

	void RenderOcclusionQuery() const;

	bool GetOcclusion();
	
	void ScreenExtents(AdvertScreenExtents* out) const;

	void Update(const Display::C_View* view, bool isPaused);

#ifdef ENABLE_IGA
	void ReadyToLoad(void);		// We have to wait for the cache to be refreshed before we can reload
#endif

#ifdef ENABLE_MASSIVE
	MassiveAdClient3::XMassiveAdClientSubscriber3D*	GetMassiveSubscriber() const { return m_pSubscriberObject; }
#endif

};

}

#endif
