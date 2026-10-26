// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SPHERE_H
#define SPHERE_H

#include <bdCore/bdCore.h>
# include <bdGraphics/bdSphere.h>

class bdVector4;

class Sphere
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		Sphere();

		~Sphere();

		bdFloat32 getRadius() const;

		void getPos(bdFloat32 *const pos) const;

		void setPos(const bdFloat32 *const pos);

		void getVel(bdFloat32 *const vel) const;

		void setVel(const bdFloat32 *const vel);

		void dampVel(const bdFloat32 factor);

		void move(const bdFloat32 secs);

		void moveToward(const Sphere *const target,
						const bdFloat32 targetRatio);

		void movePosToward(	const Sphere *const target,
							const bdFloat32 targetRatio);

		void moveVelToward(	const Sphere *const target,
							const bdFloat32 targetRatio);

		void collideBoxInternal(const bdFloat32 xDim,
								const bdFloat32 yDim);

		bdBool collideSphere(Sphere *const sphere);

		void setColor(const bdVector4& color);

		void setColor(const bdColor& color);

		void getColor(bdVector4& color) const;

		void getColor(bdColor& color) const;

		void setVisible(const bdBool visible);

		bdBool isVisible() const;

		void getExpectedResponse(bdFloat32 (&response)[2]) const; 

	protected:

		bdFloat32 m_radius;
		bdFloat32 m_vel[2];
		bdFloat32 m_expectedResponse[2];
	
		bdSphere m_mesh;

};

#endif // SPHERE_H
