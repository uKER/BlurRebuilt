
#include "stdafx.h"
#include "netLeaderboardWriteData.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netLeaderboardWriteData::netLeaderboardWriteData(int scoreboardId)
: m_scoreboardId(scoreboardId)
{
	m_rating	= 0;
	m_writeType = Scoreboard::eWriteTypeReplace;
}

netColumnDataRef netLeaderboardWriteData::findProperty(U32 fieldId)
{
	bdLinkedList<netColumnDataRef>::Position pos = m_properties.getHeadPosition();

	while(pos)
	{
		netColumnDataRef prop = m_properties.forward(pos);

		if(prop->m_FieldId == fieldId)
		{
			return prop;
		}
	}
	return NULL;
}

netColumnDataRef netLeaderboardWriteData::createProperty(U32 fieldId)
{
	netColumnDataRef prop = new netColumnData;
	prop->m_FieldId = fieldId;
	m_properties.addTail(prop);
	return prop;
}

bool netLeaderboardWriteData::setProperty(Scoreboard::ColumnData* data)
{
	netColumnDataRef prop = findProperty(data->m_FieldId);
	if(prop.isNull())
		prop = createProperty(data->m_FieldId);

	prop->m_Data = data->m_Data;
	prop->m_DataType = data->m_DataType;
	return true;
}
	

bool netLeaderboardWriteData::setPropertyS32(U32 fieldId, S32 value)
{
	netColumnDataRef prop = findProperty(fieldId);

	if(prop.isNull())
		prop = createProperty(fieldId);

	prop->SetS32(fieldId, value);
	return true;
}

bool netLeaderboardWriteData::setPropertyF32(U32 fieldId, F32 value)
{
	netColumnDataRef prop = findProperty(fieldId);

	if(prop.isNull())
		prop = createProperty(fieldId);

	prop->SetF32(fieldId, value);
	return true;
}

bool netLeaderboardWriteData::setPropertyS64(U32 fieldId, S64 value)
{
	netColumnDataRef prop = findProperty(fieldId);

	if(prop.isNull())
		prop = createProperty(fieldId);

	prop->SetS64(fieldId, value);
	return true;
}

bool netLeaderboardWriteData::setPropertyF64(U32 fieldId, F64 value)
{
	netColumnDataRef prop = findProperty(fieldId);

	if(prop.isNull())
		prop = createProperty(fieldId);

	prop->SetF64(fieldId, value);
	return true;
}


#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

