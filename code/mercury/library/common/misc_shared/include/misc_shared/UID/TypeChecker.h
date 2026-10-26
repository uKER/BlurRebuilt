#ifndef TYPECHECKER_BB6C7DE6_03A0_4176_9DC8_19692965470B_H
#define TYPECHECKER_BB6C7DE6_03A0_4176_9DC8_19692965470B_H

#ifdef _MSC_VER
#pragma once
#endif


/**
@brief An object of which there is one per type.
Use GetTypeChecker<T>() to get the address of the object representing type T.
*/
struct C_TypeChecker
{
	C_TypeChecker() {}
};


template <typename T>
struct C_TypeCheckerImpl
{
	static const C_TypeChecker s_Instance;
};

template <typename T>
const C_TypeChecker C_TypeCheckerImpl<T>::s_Instance;


template <typename T>
inline const C_TypeChecker* GetTypeChecker() {return &C_TypeCheckerImpl<T>::s_Instance;}


#endif // TYPECHECKER_BB6C7DE6_03A0_4176_9DC8_19692965470B_H
