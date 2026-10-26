// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX 
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdSphere.h>


bdSphere::bdSphere(bdFloat32 radius )
{
}

bdSphere::~bdSphere()
{
}

bdBool bdSphere::init(bdFloat32 radius)
{
	return true;
}

void bdSphere::getPosition(bdVector4 meshPos)const
{
}

void bdSphere::setPosition(bdVector4 meshPos)
{
}

void bdSphere::setColor(const bdColor& color)
{
}

void bdSphere::getColor(bdColor& color) const
{
}

bdBool bdSphere::isVisible() const
{
	return true;
}

void bdSphere::setVisible(bdBool visible)
{
}
