#include "stdafx.h"
#include "entityHeader.h"
#include "netstate/StateEnums.h"
#include "netStream/netStream.h"

C_Header::C_Header()
	: m_uEntityId(0)
	, m_uTypeId(0)
	, m_uAuthorityId(0)
	, m_uNetStateType(0)
	, m_uSize(0)
	, m_uFlags(0)
{
	;
}

C_Header::C_Header(const short int uEntityId, const int uTypeId, const char uAuthorityId, const char eStateType, short int uSize)
	: m_uEntityId(uEntityId)
	, m_uTypeId(uTypeId)
	, m_uAuthorityId(uAuthorityId)
	, m_uNetStateType(eStateType)
	, m_uSize(uSize)
	, m_uFlags(0)
{
	;
}

int C_Header::NetStateType() const
{
	return m_uNetStateType;
}

bool C_Header::Serialise(class netStream* stream) const
{
	assert(Valid());
	stream->writeType(m_uEntityId);
	stream->writeType(m_uTypeId);
	stream->writeType(m_uAuthorityId);
	stream->writeType(m_uNetStateType);
	stream->writeType(m_uSize);
	stream->writeType(m_uFlags);
	return stream->ok();
}
bool C_Header::Deserialise(class netStream* stream)
{
	stream->readType(m_uEntityId);
	stream->readType(m_uTypeId);
	stream->readType(m_uAuthorityId);
	stream->readType(m_uNetStateType);
	stream->readType(m_uSize);
	stream->readType(m_uFlags);
	return stream->ok();
}