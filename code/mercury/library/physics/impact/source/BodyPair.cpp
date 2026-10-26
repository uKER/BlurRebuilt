#include "stdafx.h"

#include "Impact\Body.h"
#include "Impact\BodyPair.h"

using namespace Maths;

namespace Impact
{

BodyPair::BodyPair(Body *p1, Body *p2)
	: m_bActive(true)
{
	m_pBody[0] = p1;
	m_pBody[1] = p2;
}

BodyPair::~BodyPair()
{
}

};
