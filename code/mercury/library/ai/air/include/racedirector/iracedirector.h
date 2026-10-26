#ifndef _RACEDIRECTOR_I_H
#define _RACEDIRECTOR_I_H


#include <ComponentSystem/componentlib.h>


namespace airacingtech
{
	class RaceDirectorEvent;
	class IData;

	// Another quick hack for the amax game, shouldn`t be here....but sod it...
	class AIEventSegmentData
	{
	public:

		AIEventSegmentData(){};
		~AIEventSegmentData(){};

		int		m_To;
		float	m_Distance;
		float	m_TrackSpeedPercentageMax;
		float   m_TrackSpeedPercentageMin;
		int		m_Enable;
		int		m_Group;
		int		m_AttributeTemplateIndex;
		int		m_PerkPrioritiesTemplateIndex;
	};

	class AIEventBalancingData
	{
	public:

		AIEventBalancingData(){};
		~AIEventBalancingData()
		{
		}

		Btl::Vector<AIEventSegmentData>	m_SegmentData;
	};

	class AIEventBalancing
	{
	public:
		AIEventBalancing(){};
		~AIEventBalancing()
		{
			Destroy();
		};

		void Destroy()
		{
			for(int i=0; i<m_Data.Size(); i++)
			{
				AIEventBalancingData *pData = m_Data[i];
				delete pData;
			}
			m_Data.Clear();
		}

		Btl::Vector<AIEventBalancingData*> m_Data;
	};







	BEGIN_INTERFACE_DEC(IRaceDirector)
	
	public:
		virtual void Update() = 0;
		virtual void PreUpdate() = 0;
		virtual void PostUpdate() = 0;

		
		virtual AIEventBalancing *GetEventBalancing() = 0;
		virtual IData *GetAIAtPosition(int rank) = 0;

	END_INTERFACE_DEC

	
}

#endif //_RACEDIRECTOR_I_H
