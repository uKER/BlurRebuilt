#pragma once

#include "Lux/IIntegrator.h"

namespace LUX
{

class Lux;

class ISamplePointIterator : public SurfaceIterator
{
public:
	virtual ~ISamplePointIterator() {};
	virtual void ReadXMLNode(XERCES_CPP_NAMESPACE::DOMNode *pNode) = 0;
	virtual void SetSpatialHierarchy(const SD::SpatialHierarchy *pTee) {};

	float m_fBrightness, m_fOutputGamma;
	bool m_bPrescaleSH;
};

}
