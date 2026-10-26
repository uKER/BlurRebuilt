// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdInterpolationData.h>

#define BD_LOG_LEVEL "bdClientServer/interpolationdata"

bdInterpolationData::bdInterpolationData()
: m_target(BD_NULL),
  m_active(false),
  m_averageUpdateInterval(0.f),
  m_elapsedTime(0),
  m_interpolationInterval(0),
  m_interpolationBreak(false)
{
}

bdInterpolationData::~bdInterpolationData()
{
	if(m_target)
	{
		delete m_target;
	}
}

void bdInterpolationData::setTarget(bdInterpolationInterface *const target)
{
	m_target = target;
	m_active = m_target?true:false;
}

bdInterpolationInterface* bdInterpolationData::getTarget()
{
	return m_target;
}

void bdInterpolationData::setActive(const bdBool active, 
									bdReplicaRef replica)
{
	if(m_target)
	{
		m_active = active;
		if(m_active)
		{
			// initialise the target to be the same as the replica
			bdBitBufferRef buffer = new bdBitBuffer;
			replica->writeFullState(buffer);
			if (!m_target->deserialize(buffer))
			{
				BD_WARN(BD_LOG_LEVEL,"Deserialization of replica failed.");
			}
		}
	}
}

bdBool bdInterpolationData::getActive() const
{
	return m_active;
}

void bdInterpolationData::setInterpolationInterval(bdFloat32 interpolationInterval)
{
	m_interpolationInterval = interpolationInterval;
}

bdFloat32 bdInterpolationData::getNextInterpolationStep(const bdFloat32 deltaTime)
{	

	bdFloat32 ratio = 1;
	if (m_interpolationBreak)
	{
		m_interpolationBreak = false;
	} 
	else if(m_elapsedTime > m_interpolationInterval)
	{
		ratio = 0;
	}
	else
	{
		// protect against divide by zero
		if(m_interpolationInterval != m_elapsedTime)
		{
			const bdFloat32 timeLeft = m_interpolationInterval - m_elapsedTime;
			ratio = BD_MIN(deltaTime / timeLeft, 1);
		}

		m_elapsedTime += deltaTime;
	}

	return ratio;
}


void bdInterpolationData::startInterpolation()
{
	m_elapsedTime = 0.0f;
}

void bdInterpolationData::endInterpolation()
{
	m_elapsedTime = m_interpolationInterval + 1;
	m_interpolationBreak = true;
}
