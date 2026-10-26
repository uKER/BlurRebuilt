#ifndef _IINTEGRATOR_H_
#define	_IINTEGRATOR_H_

class SurfaceIterator;
struct NmRawTriangle;

class IIntegrator : public BaseObject
{
public:
	virtual void LightData(SurfaceIterator *) = 0;
	virtual void GetLights(const box3f &bbox, Ut::Vector<NmRawTriangle> *pTriList) = 0;
};

#endif
