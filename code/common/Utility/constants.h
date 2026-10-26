/////////////////////////////////////////////////////////////
// Template used to get constants of of a given type.
#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifdef _MSC_VER
#pragma once
#endif


namespace UtMaths
{

template<typename DataType>
struct Constants {};

template <> struct Constants<int> 
{
	static int Zero() {return 0;}
	static int One() {return 1;}
	static int Two() {return 2;}
};

template <> struct Constants<float> 
{
	static float Zero() {return 0.f;}
	static float Epsilon() {return 0.000001f;}
	static float One() {return 1.f;}
	static float Half() {return 0.5f;}
	static float Two() {return 2.f;}
};

template <> struct Constants<double> 
{
	static double Zero() {return 0.0;}
	static double Epsilon() {return 0.000001;}
	static double One() {return 1.0;}
	static double Half() {return 0.5;}
	static double Two() {return 2.0;}
};

}//end namespace UtMaths

#endif
