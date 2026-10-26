#ifndef RENDER_OCCLUSION_POOL_H
#define RENDER_OCCLUSION_POOL_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Render/Utils/ListPool.h"

namespace Render
{
namespace Occlusion
{

template<typename Query>
class C_Pool
{
public:

	C_Pool() {}
	~C_Pool() {}

	bool Create(const U32 size)
	{
		m_iQueryMax = size;
		for (U32 queryNo = 0; queryNo < m_iQueryMax; ++queryNo)
			m_QueryList.Cache(new Query());
		return true;
	}

	bool CreateVolatile()
	{
		bool bSucceeded = true;
		typename Ut::ListPool<Query>::Iterator itBegin = m_QueryList.m_Cache.Begin();
		typename Ut::ListPool<Query>::Iterator itEnd = m_QueryList.m_Cache.End();
		for (typename Ut::ListPool<Query>::Iterator it = itBegin; it != itEnd; ++it)
			bSucceeded = bSucceeded && it->CreateVolatile();
		return bSucceeded;
	}

	void DestroyVolatile()
	{
		// Clear the cache list
		{
			typename Ut::ListPool<Query>::Iterator itBegin = m_QueryList.m_Cache.Begin();
			typename Ut::ListPool<Query>::Iterator itEnd = m_QueryList.m_Cache.End();
			for (typename Ut::ListPool<Query>::Iterator it = itBegin; it != itEnd; ++it)
				it->DestroyVolatile();
		}

		// Clear the active list
		{
			typename Ut::ListPool<Query>::Iterator itBegin = m_QueryList.m_Active.Begin();
			typename Ut::ListPool<Query>::Iterator itEnd = m_QueryList.m_Active.End();
			for (typename Ut::ListPool<Query>::Iterator it = itBegin; it != itEnd; ++it)
				it->DestroyVolatile();
		}
	}

	void Destroy()
	{
		m_QueryList.Free();
	}

	Query * Allocate() { return m_QueryList.Allocate(); }

	void Free(Query * pQuery) { m_QueryList.Free(pQuery); }

private:

	Ut::ListPool<Query>	m_QueryList;
	U32					m_iQueryMax;
	
};

template<typename Query>
class C_RingBuffer
{
public:

	C_RingBuffer() {}
	~C_RingBuffer() {}

	bool Create(const U32 size)
	{
		m_iQueryNo = size - 1;
		m_iQueryMax = size;
		m_pQueries = new Query[m_iQueryMax];
		return true;
	}

	bool CreateVolatile()
	{
		bool bSucceeded = true;
		for (U32 queryNo = 0; queryNo < m_iQueryMax; ++queryNo)
			bSucceeded = bSucceeded && m_pQueries[queryNo].CreateVolatile();
		return bSucceeded;
	}

	void DestroyVolatile()
	{
		for (U32 queryNo = 0; queryNo < m_iQueryMax; ++queryNo)
			m_pQueries[queryNo].DestroyVolatile();
	}

	void Destroy()
	{
		delete[] m_pQueries;
		m_pQueries = 0;
		m_iQueryNo = 0;
		m_iQueryMax = 0;
	}

	Query * Allocate()
	{
		m_iQueryNo = (m_iQueryNo + 1) % m_iQueryMax;
		return m_pQueries[m_iQueryNo];
	}

private:

	Query *	m_pQueries;
	U32		m_iQueryNo;
	U32		m_iQueryMax;

};

}; // namespace Occlusion
}; // namespace Render

#endif // RENDER_OCCLUSION_POOL_H
