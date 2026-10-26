// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "TagQueries.h"

#define BD_LOG_LEVEL "TAG Queries"


FindFileIDsByMyCustomTagsQuery::FindFileIDsByMyCustomTagsQuery()
{
}
FindFileIDsByMyCustomTagsQuery::FindFileIDsByMyCustomTagsQuery(bdUInt genre,
													   bdUInt yearStart, 
													   bdUInt yearEnd)
: m_genre(genre),
  m_yearStart(yearStart), 
  m_yearEnd(yearEnd) 
{
}
FindFileIDsByMyCustomTagsQuery::~FindFileIDsByMyCustomTagsQuery()
{
}

void FindFileIDsByMyCustomTagsQuery::serialize(bdByteBuffer& buffer)const
{
	buffer.writeUInt32(m_genre);
	buffer.writeUInt32(m_yearStart);
	buffer.writeUInt32(m_yearEnd);
}

bdUInt FindFileIDsByMyCustomTagsQuery::sizeOf() const
{
	return sizeof(FindFileIDsByMyCustomTagsQuery);
}
