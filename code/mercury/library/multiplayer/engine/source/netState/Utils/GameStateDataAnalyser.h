#ifndef _DATA_ANALYSER
#define _DATA_ANALYSER

#include <btl/containers/map.h>
#include "netState/netEntity/entityState.h"
#include <system/debug/I_DebugRender.h>

class C_DataAnalyser
{
	enum { E_LOG_SIZE = 240 };

	class C_Stats
	{
	public:
		C_Stats(): m_uEntHead(0), m_uCompHead(0), m_uData(0), m_uFullCount(0), m_uCompCount(0){;}
	public:
		unsigned int	m_uEntHead;
		unsigned int	m_uCompHead;
		unsigned int	m_uCompHeadNoRLE;
		unsigned int	m_uData;
		unsigned int	m_uFullCount;
		unsigned int	m_uCompCount;

		C_Stats operator+(const C_Stats& xRhs)
		{
			C_Stats xTotal = xRhs;

			xTotal.m_uCompCount		= xTotal.m_uCompCount + m_uCompCount;
			xTotal.m_uCompHead		= xTotal.m_uCompHead + m_uCompHead;
			xTotal.m_uCompHeadNoRLE = xTotal.m_uCompHeadNoRLE + m_uCompHeadNoRLE;
			xTotal.m_uData			= xTotal.m_uData + m_uData;
			xTotal.m_uEntHead		= xTotal.m_uEntHead + m_uEntHead;
			xTotal.m_uFullCount		= xTotal.m_uFullCount + m_uFullCount;

			return xTotal;
		}
	};

	class C_EntityData
	{
	public:
		C_EntityData() : m_bActive(false), m_uTypeId(0) {;}
		bool	m_bActive;
		int		m_uTypeId;
		C_Stats	m_xOutgoing;
		C_Stats	m_xIncoming;
	};

	class C_TypeData
	{
	public:
		enum {E_NAME_LENGTH = 9};
		C_TypeData() { memset(m_strTypeName, 0, E_NAME_LENGTH); }
		C_TypeData(const C_TypeData& xCopy)
		{
			m_xOutgoing = xCopy.m_xOutgoing;
			m_xIncoming = xCopy.m_xIncoming;

			strcpy(m_strTypeName, xCopy.m_strTypeName);
		}
		
		void Reset() { m_xOutgoing = C_Stats(); m_xIncoming = C_Stats(); }

		C_TypeData operator+(const C_TypeData& xRhs)
		{
			C_TypeData xTotal = xRhs;

			xTotal.m_xOutgoing = xTotal.m_xOutgoing + m_xOutgoing;
			xTotal.m_xIncoming = xTotal.m_xIncoming + m_xIncoming;

			return xTotal;
		}

		C_TypeData operator=(const C_TypeData& xRhs)
		{
			m_xOutgoing = xRhs.m_xOutgoing;
			m_xIncoming = xRhs.m_xIncoming;

			strcpy(m_strTypeName, xRhs.m_strTypeName);

			return *this;
		}

		C_Stats	m_xOutgoing;
		C_Stats	m_xIncoming;
		char	m_strTypeName[E_NAME_LENGTH+1];
	};

	typedef Btl::Map<U32, C_TypeData> C_TypeList;
public:
	C_DataAnalyser(int uMaxEntities) 
		: m_uMaxEntities(uMaxEntities)
		, m_pxEntityData(new C_EntityData[uMaxEntities])
		, m_uTotalBytesSent(0)
		, m_uLogIndex(0)
	{;}
	~C_DataAnalyser()
	{
		delete [] m_pxEntityData;
	}

	bool	IncomingState(C_EntityState* pxEntityState, const char* strTypeName);
	bool	OutgoingState(C_EntityState* pxEntityState, const char* strTypeName);
	void	Update();
	bool	IsActive(int i);
	
	int		DebugOutputEntity(int i, char *str, int uBufferSize);
	int		DebugOutputSummary(char *str, int uBufferSize);
	int		DebugOutputTypeSummary(char *str, int uBufferSize);
	
	void	DebugGui(DbUtil::I_DebugRenderPtr debugRenderPtr);
private:
	C_TypeData& GetGlobalTypeData(const int uTypeId);
	C_TypeData& GetCurrentTypeData(const int uTypeId);
	Maths::RGBA	StringToColour(const char* str);
private:
	const int		m_uMaxEntities;
	C_EntityData*	m_pxEntityData;
	C_TypeList		m_xCurrentTypeData;
	C_TypeList		m_xGlobalTypeData;
	C_TypeList		m_xTypeDataLog[E_LOG_SIZE];
	int				m_uLogIndex;
	unsigned int	m_uTotalBytesSent;
};

#endif