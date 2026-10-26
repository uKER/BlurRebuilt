// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "SphereEx.h"

SphereEx::SphereEx()
{
}

SphereEx::~SphereEx()
{
}

Sphere& SphereEx::getSphere()
{
return m_sphere;
}

void SphereEx::setConnection(const bdConnectionRef connection)
{
	m_connection = connection;
}

bdConnectionRef SphereEx::getConnection()
{
	return m_connection;
}

void SphereEx::writeInitData(const bdBitBufferRef buffer)
{
	bdFloat32 pos[2];
	bdVector4 color;

	m_sphere.getPos(pos);
	m_sphere.getColor(color);

	buffer->writeFloat32(pos[0]);
	buffer->writeFloat32(pos[1]);
	buffer->writeFloat32(color.getX());
	buffer->writeFloat32(color.getY());
	buffer->writeFloat32(color.getZ());
	buffer->writeFloat32(color.getW());
}

bdBool SphereEx::applyInitData(const bdReceivedMessage& initMsg)
{
	const bdMessageRef message = initMsg.getMessage();
	const bdBitBufferRef buffer = message->getPayload();
	const bdConnectionRef connection = initMsg.getConnection();

	bdFloat32 pos[2];
	bdVector4 color;

	bdBool ok = buffer->readFloat32(pos[0]);
	ok = ok && buffer->readFloat32(pos[1]);
	ok = ok && buffer->readFloat32(color[0]);
	ok = ok && buffer->readFloat32(color[1]);
	ok = ok && buffer->readFloat32(color[2]);
	ok = ok && buffer->readFloat32(color[3]);

	if(ok)
	{
		m_sphere.setPos(pos);
		m_sphere.setColor(color);
		m_connection = connection;
	}

	return ok;
}

void SphereEx::createUpdateData(const bdBitBufferRef buffer)
{
	bdFloat32 pos[2];

	m_sphere.getPos(pos);

	buffer->writeFloat32(pos[0]);
	buffer->writeFloat32(pos[1]);
}

bdBool SphereEx::applyUpdateData(const bdReceivedMessage& updateMsg)
{
	const bdMessageRef message = updateMsg.getMessage();
	const bdBitBufferRef buffer = message->getPayload();
	const bdConnectionRef connection = updateMsg.getConnection();

	BD_ASSERT(connection == m_connection, "Applying update from wrong peer.");

	bdFloat32 pos[2];

	bdBool ok = buffer->readFloat32(pos[0]);
	ok = ok && buffer->readFloat32(pos[1]);

	if(ok)
	{
		m_sphere.setPos(pos);
	}

	return ok;
}


