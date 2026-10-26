// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdTestSocket.h>

bdTestSocket::bdTestSocket()
: m_drop(0.f),
  m_minDelay(0.f),
  m_maxDelay(0.f),
  m_avgDelay(0.f),
  m_lag(0.f)
  
{
}

bdTestSocket::~bdTestSocket()
{
	close();
}


void bdTestSocket::setSeed(const bdUInt seed)
{
	m_rand.setSeed(seed);
}

void bdTestSocket::setDropFactor(bdFloat32 drop)
{
	m_drop = drop;
}

void bdTestSocket::setLag(bdFloat32 min, bdFloat32 max, bdFloat32 avg)
{
	m_minDelay = min;
	m_maxDelay = max;
	m_avgDelay = avg;

	m_lag = m_avgDelay;
}


bdInt bdTestSocket::sendTo(const bdAddr &addr, 
							 const void *data, 
							 const bdUInt length)
{

	const bdUInt random = m_rand.nextUInt();
	const bdFloat32 percent = (1.0f * random / (BD_MAX_RANDOM+1.0f));
	
	if(m_drop >= percent) 
	{
		return length;
	}
	
	bdByteBufferRef byteBuffer = new bdByteBuffer(length);
	bdUByte8 *const buffer = byteBuffer->getData();
	bdMemcpy(buffer, data, length);
	
	calculateLag();

	PacketStore store(addr, byteBuffer, m_lag);

	m_outDelayed.enqueue(store);

	flushOut();

	return length;
}


bdInt	bdTestSocket::receiveFrom(bdAddr &addr, 
								  void *data,
								  const bdUInt size)
{
	flushOut();

	bdInt retval = bdSocket::receiveFrom(addr, data, size);

	if(retval != BD_NET_WOULD_BLOCK)
	{
		calculateLag();

		if(retval > 0)
		{
			bdByteBufferRef buffer = new bdByteBuffer(static_cast<bdUInt>(retval));
			bdMemcpy(buffer->getData(), data, static_cast<bdUInt>(retval));

			PacketStore store(addr, buffer, m_lag, retval);

			m_inDelayed.enqueue(store);
		}
		else
		{
			PacketStore store(addr, BD_NULL, m_lag, retval);

			m_inDelayed.enqueue(store);
		}

		
	}

	bdByteBufferRef buffer;
	const bdInt bufferSize = flushIn(addr, buffer);

	if(static_cast<bdInt>(size) < bufferSize)
	{
		return BD_NET_MSG_SIZE;
	}
	else if(bufferSize > 0)
	{
		bdMemcpy(data, buffer->getData(), buffer->getSize());
	}

	return bufferSize;
}

bdBool bdTestSocket::close()
{
	// when closing just flush the output ignoring delay times
	while(!m_outDelayed.isEmpty())
	{
		PacketStore &store = m_outDelayed.peek();

		bdSocket::sendTo(store.m_addr, store.m_buffer->getData(), store.m_buffer->getSize());
		m_outDelayed.dequeue();
	}

	return bdSocket::close();
}


void bdTestSocket::flushOut()
{
	while(!m_outDelayed.isEmpty())
	{
		PacketStore &store = m_outDelayed.peek();

		if(store.isReady())
		{
			bdSocket::sendTo(store.m_addr, store.m_buffer->getData(), store.m_buffer->getSize());
			m_outDelayed.dequeue();
		}
		else
		{
			break;
		}
	}
}

bdInt	bdTestSocket::flushIn(bdAddr &addr, 
								bdByteBufferRef &buffer)
{

	if(m_inDelayed.isEmpty())
	{
		return BD_NET_WOULD_BLOCK;
	}

	PacketStore &store = m_inDelayed.peek();

	if(store.isReady())
	{
		addr = store.m_addr;
		bdInt retval = store.m_retval;

		buffer = store.m_buffer;

		m_inDelayed.dequeue();

		return retval;
	}

	return BD_NET_WOULD_BLOCK;
}


void bdTestSocket::calculateLag()
{
	const bdUInt random = m_rand.nextUInt();
	const bdUInt percent = static_cast<bdUInt>(100.0 * random / (BD_MAX_RANDOM+1.0));

	if(percent < 50)
	{
		const bdFloat32 dec = (m_avgDelay - m_minDelay) / 10;
		m_lag -= dec;
		if(m_lag < m_minDelay)
		{
			m_lag = m_minDelay;
		}
	}
	else
	{
		const bdFloat32 inc = (m_maxDelay - m_avgDelay) / 10;
		m_lag += inc;
		if(m_lag > m_maxDelay)
		{
			m_lag = m_maxDelay;
		}
	}
}









