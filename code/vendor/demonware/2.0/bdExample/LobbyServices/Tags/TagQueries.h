
// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef CUSTOM_TAG_QUERIES_H
#define CUSTOM_TAG_QUERIES_H

#include <bdLobby/bdTags/bdTagQueryParams.h>

enum primaryTags
{
	GENRE,
	YEAR
};
enum genreTags
{
	TRADITIONAL,
	COUNTRY,
	METAL
};
/*
We'll just use the direct int value of the year for tags. No need to enum these values.
enum yearTags
{
}
*/

/// Encapsulates a tag custom query, previously defined in WebIMP.
/// Query approximation: 
/// SELECT * FROM tags WHERE 
///   primaryTag = m_genre 
///   AND secondaryTag >= m_yearStart
///   AND secondaryTag <= m_yearEnd
class FindFileIDsByMyCustomTagsQuery : public bdTagQueryParams
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		FindFileIDsByMyCustomTagsQuery();
		FindFileIDsByMyCustomTagsQuery(bdUInt genre,
									   bdUInt yearStart, 
									   bdUInt yearEnd);
		virtual ~FindFileIDsByMyCustomTagsQuery();

		virtual void serialize(bdByteBuffer &buffer)const;

		virtual bdUInt sizeOf() const;

		/// Custom query params
		bdUInt m_genre;
		bdUInt m_yearStart;
		bdUInt m_yearEnd;
};

#endif // CUSTOM_TAG_QUERIES_H

