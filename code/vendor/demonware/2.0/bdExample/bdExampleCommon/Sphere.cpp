// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/Sphere.h>

Sphere::Sphere()
:	m_radius(0.5f),
	m_mesh(m_radius)
{
	m_vel[0] = 0;
	m_vel[1] = 0;
}

Sphere::~Sphere()
{
}

bdFloat32 Sphere::getRadius() const
{
	return m_radius;
}

void Sphere::getPos(bdFloat32 *const pos) const
{
	bdVector4 meshPos;
	m_mesh.getPosition(meshPos);
	pos[0] = meshPos[0];
	pos[1] = meshPos[1];
}

void Sphere::setPos(const bdFloat32 *const pos)
{
	m_mesh.setPosition(bdVector4(pos[0], pos[1], 0.f));
}

void Sphere::setVel(const bdFloat32 *const vel)
{
	m_vel[0] = vel[0];
	m_vel[1] = vel[1];	
}

void Sphere::getVel(bdFloat32 *const vel) const
{
	vel[0] = m_vel[0];
	vel[1] = m_vel[1];
}

void Sphere::dampVel(const bdFloat32 factor)
{	
	for(bdUInt32 i=0; i < 2; i++)
	{
		if((m_vel[i] > 0.01f) || (m_vel[i] < -0.01f))
		{
			const bdFloat32 ratio = BD_CLAMP(factor, 0, 1);
			m_vel[i] -= m_vel[i] * ratio;
		}
		else
		{
			m_vel[i] = 0;
		}
	}
}

void Sphere::move(const bdFloat32 secs)
{
	bdVector4 pos;
	m_mesh.getPosition(pos);
	pos[0] += m_vel[0] * secs;
	pos[1] += m_vel[1] * secs;

	m_mesh.setPosition(pos);
}

void Sphere::moveToward(const Sphere *const target,
						const bdFloat32 targetRatio)
{
	movePosToward(target, targetRatio);
	moveVelToward(target, targetRatio);
}

void Sphere::movePosToward(const Sphere *const target,
						   const bdFloat32 targetRatio)
{
	bdFloat32 targetPos[2];
	target->getPos(targetPos);
	// Interpolate towards target position
	bdFloat32 pos[2];
	getPos(pos);

	pos[0] += (targetPos[0]-pos[0]) * targetRatio;
	pos[1] += (targetPos[1]-pos[1]) * targetRatio;

	setPos(pos);

}

void Sphere::moveVelToward(const Sphere *const target,
						   const bdFloat32 targetRatio)
{
	bdFloat32 targetVel[2];
	target->getVel(targetVel);

	// Interpolate towards target velocity
	m_vel[0] += (targetVel[0]-m_vel[0]) * targetRatio;
	m_vel[1] += (targetVel[1]-m_vel[1]) * targetRatio;

}

void Sphere::collideBoxInternal(const bdFloat32 xDim,
								const bdFloat32 yDim)
{
	const bdFloat32 dim[2] = {xDim, yDim};
	bdFloat32 pos[2];
	getPos(pos);

	for(bdUInt32 i=0; i < 2; i++)
	{
		if(pos[i] > (dim[i] - m_radius))
		{
			m_vel[i] = -m_vel[i];
			pos[i] = dim[i] - m_radius;
		}
		else if(pos[i] < (-dim[i] + m_radius))
		{
			m_vel[i] = -m_vel[i];
			pos[i] = -dim[i] + m_radius;
		}
	}

	setPos(pos);
}


bdBool Sphere::collideSphere(Sphere *const sphere)
{
	bdFloat32 pos[2];
	getPos(pos);

	bdFloat32 otherPos[2];
	sphere->getPos(otherPos);

	const bdFloat32 diffX = pos[0] - otherPos[0];
	const bdFloat32 diffY = pos[1] - otherPos[1];
	const bdFloat32 distSq = (diffX*diffX) + (diffY*diffY);

	const bdFloat32 combinedRadius = m_radius + sphere->m_radius;
	const bdFloat32 combinedRadiusSq = combinedRadius * combinedRadius;

	if(distSq < combinedRadiusSq)
	{

		Sphere *const s1 = this;
		Sphere *const s2 = sphere;
		
		bdFloat32 pos1[2];
		bdFloat32 vel1[2];
		s1->getPos(pos1);
		s1->getVel(vel1);		

		bdFloat32 pos2[2];
		bdFloat32 vel2[2];
		s2->getPos(pos2);
		s2->getVel(vel2);		
		
		// Do Collision reaction. Notation: "U" is old velocity, "V" is new velocity
		// "O" is the position of a sphere.
//		const bdVector4 O1(pos1[0], pos1[1], 0);
//		const bdVector4 O2(pos2[0], pos2[1], 0);
		
		// xAxis represents the axis of incidence
		bdVector4 xAxis;
		xAxis.set(diffX, diffY, 0.f);
		xAxis.normalize();
		
		// Determine the components of s1's velocity w.r.t xAxis
		bdVector4 U1(vel1[0], vel1[1], 0.f);
		
		bdVector4 U1x(xAxis[0], xAxis[1], 0.f);
		U1x.multiply3(xAxis.dot3(U1));
		U1.subtract3(U1x);

		bdVector4 U1y(U1);		

		// Determine the components of s2's velocity w.r.t xAxis
		bdVector4 U2(vel2[0], vel2[1], 0.f);

		bdVector4 U2x(xAxis[0], xAxis[1], 0.f);
		U2x.multiply3(xAxis.dot3(U2));
		U2.subtract3(U2x);

		bdVector4 U2y(U1);

		// Calculate new velocity for s1
		bdVector4 V1x, V1y;
		// ((U1x+U2x)-(U1x-U2x))*0.5f
		{
			bdVector4 temp1, temp2;
			temp1.set(U1x[0], U1x[1], 0.0f);
			temp1.add3(U2x);
			temp2.set(U1x[0], U1x[1], 0.0f);
			temp2.subtract3(U2x);

			V1x.set(temp1[0], temp1[1], 0.0f);
			V1x.subtract3(temp2);
			V1x.multiply3(0.5f);

			V1y=U1y;
		}

		// Calculate new velocity for s2
		bdVector4 V2x, V2y;
		// ((U1x+U2x)-(U2x-U1x))*0.5f
		{
			bdVector4 temp1, temp2;
			temp1.set(U1x[0], U1x[1], 0.0f);
			temp1.add3(U2x);
			temp2.set(U2x[0], U2x[1], 0.0f);
			temp2.subtract3(U1x);

			V2x.set(temp1[0], temp1[1], 0.0f);
			V2x.subtract3(temp2);
			V2x.multiply3(0.5f);

			V2y=U2y;
		}
		
		// Set the result
		bdVector4 V1(V1x[0], V1x[1], 0.0f);
		V1.add3(V1y);
		s1->setVel(V1.getData());

		// Set the result
		bdVector4 V2(V2x[0], V2x[1], 0.0f);
		V2.add3(V2y);
		s2->setVel(V2.getData());

		// Store the expected velocity of the other object in 
		// the collision
		m_expectedResponse[0] = V2x[0];
		m_expectedResponse[1] = V2x[1];

		// Move spheres apart so that they no longer inter-penetrate
		bdVector4 targetDifference = xAxis;
		targetDifference.multiply3(combinedRadius + (combinedRadius*0.05f));
		bdVector4 currentDifference(diffX, diffY, 0.0f);
		targetDifference.subtract3(currentDifference);
		pos1[0] = pos1[0] + targetDifference.dot3(bdVector4(1.0f, 0.0f, 0.0f))/2.0f;
		pos1[1] = pos1[1] + targetDifference.dot3(bdVector4(0.0f, 1.0f, 0.0f))/2.0f;
		pos2[0] = pos2[0] - targetDifference.dot3(bdVector4(1.0f, 0.0f, 0.0f))/2.0f;
		pos2[1] = pos2[1] - targetDifference.dot3(bdVector4(0.0f, 1.0f, 0.0f))/2.0f;

		s1->setPos(pos1);
		s2->setPos(pos2);

		return true;		
	}

	return false;
}

void Sphere::setColor(const bdVector4& color)
{
	setColor(bdColor(color));
}

void Sphere::setColor(const bdColor& color)
{
	m_mesh.setColor(color);
}

void Sphere::getColor(bdVector4& color) const
{
	bdColor temp;

	m_mesh.getColor(temp);

	temp.asFloat(reinterpret_cast<bdFloat32*>(&color));
}

void Sphere::getColor(bdColor& color) const
{
	m_mesh.getColor(color);
}

void Sphere::setVisible(const bdBool visible)
{
	m_mesh.setVisible(visible);
}

bdBool Sphere::isVisible() const
{
	const bdBool visible = m_mesh.isVisible();
	return visible;
}

void Sphere::getExpectedResponse(bdFloat32 (&response)[2]) const
{
	response[0] = m_expectedResponse[0];
	response[1] = m_expectedResponse[1];
}
