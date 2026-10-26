#ifndef _MATHS_H_
#define _MATHS_H_

#define ALIGN16
#define ALIGN8

#include "VecMat.h"
#include "box.hpp"

#define DEG2RAD(x)				((x)*(PI / 180.0f))
#define RAD2DEG(x)				((x)*(180.0f / PI))

// fast float to int and back
#define FAST_INT 1

#define _doublemagicroundeps	      (.5-1.4e-11)
	//almost .5f = .5f - 1e^(number of exp bit)
inline int Round2Int(double val) {
#ifdef FAST_INT
#define _doublemagic			double (6755399441055744.0)
	//2^52 * 1.5,  uses limited precision to floor
	val		= val + _doublemagic;
	return ((long*)&val)[0];
#else
	return int (val+_doublemagicroundeps);
#endif
}
inline int Float2Int(double val) {
#ifdef FAST_INT
	return (val<0) ?  Round2Int(val+_doublemagicroundeps) :
		   Round2Int(val-_doublemagicroundeps);
#else
	return (int)val;
#endif
}
inline int Floor2Int(double val) {
#ifdef FAST_INT
	return Round2Int(val - _doublemagicroundeps);
#else
	return (int)floorf(val);
#endif
}
inline int Ceil2Int(double val) {
#ifdef FAST_INT
	return Round2Int(val + _doublemagicroundeps);
#else
	return (int)ceilf(val);
#endif
}

#ifndef M_PI
#define M_PI		3.14159265358979323846f
#endif
#define INV_PI		0.31830988618379067154f
#define INV_TWOPI	0.15915494309189533577f

inline bool IsPowerOf2(int v) {
	return (v & (v - 1)) == 0;
}
inline unsigned int RoundUpPow2(unsigned int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v+1;
}

#endif
