#ifndef I_ANYTHING_2475852C_8712_439B_90E5_9896A8180F40_H
#define I_ANYTHING_2475852C_8712_439B_90E5_9896A8180F40_H

// NOTE: Probably shouldn't stay in misc_shared/ContextualStream, it's here
// until we figure out if it's fit for purpose.

#ifdef _MSC_VER
#pragma once
#endif


namespace Sys
{

/**
@brief Unique type identifier.

Takes a user-defined value which allows a class to be uniquely identified at
runtime.
*/
typedef U32 C_AnythingUid;

/**
@brief Interface class for objects needing RTTI.

This interface allows the user to determine if the object is of the type 
identified by a user-defined value. Users should not derive directly from
this class, instead using C_Anything to implement cascading RTTI.

There are two basic mechanisms for downcasting. The GetTypeUid() method
determines if the downcast is legal. For single-inheritence you could 
static_cast to this type safely. For multiple-inheritence, and for future-
proofing, you should prefer the QueryInterface() method.
*/
class I_Anything
{
public:
	virtual ~I_Anything();

	inline C_AnythingUid GetTypeUid() const {return m_TypeUid;} /// @returns unique type identifier for the object.
	virtual bool IsTypeOf(const C_AnythingUid typeUid) const; ///< Determines if the object is compatible with this type.

	inline bool QueryInterface(const C_AnythingUid typeUid, void** ppDest); ///< Obtains a pointer to the requested class.
	inline bool QueryInterface(const C_AnythingUid typeUid, const void** ppDest) const; ///< Obtains a pointer to the requested class.

protected:
	explicit I_Anything(const C_AnythingUid typeUid);

	virtual bool QueryInterfaceImpl(const C_AnythingUid typeUid, void** ppDest); ///< Obtains a pointer to the requested class.

private:
	C_AnythingUid m_TypeUid; ///< unique type identifier

	I_Anything(const I_Anything&); // can not be copied
	I_Anything& operator = (const I_Anything&); // can not be copied

};

/**
@brief Base class for objects needing RTTI.

Implements the class hierarchy cascade for RTTI. Users should derive from 
this for single-inheritence of I_Anything.
@code
class C_MyClass : public Sys::C_Anything<k_MyClassUid>
{
public:
	void DoSomething();
};

void DoSomthingIfObjIsMyClass(I_Anything* pObj)
{
	C_MyClass* pMyObj = Sys::QueryInterface<C_MyClass>(pObj);
	if (pMyObj != NULL)
		pMyObj->DoSomething();
}
@endcode

@param t_TypeUid unique type identifier.
@param T_Base base class.

@see I_Anything.
*/
template <C_AnythingUid t_TypeUid, class T_Base = I_Anything>
class C_Anything : public T_Base
{
public:
	static const C_AnythingUid kClassUid = t_TypeUid; ///< compile-time constant
	static C_AnythingUid ClassUid() {return t_TypeUid;} ///< @returns unique type identifier for the class.

	/// from I_Anything
	virtual bool IsTypeOf(const C_AnythingUid typeUid) const 
	{
		return (typeUid == t_TypeUid) || T_Base::IsTypeOf(typeUid);
	}

protected:
	typedef C_Anything C_Super;

	C_Anything() : T_Base(t_TypeUid) {}
	explicit C_Anything(const C_AnythingUid typeUid) : T_Base(t_TypeUid) {}

	/// from I_Anything
	virtual bool QueryInterfaceImpl(const C_AnythingUid typeUid, void** ppDest) 
	{
		if (typeUid == t_TypeUid)
		{
			*ppDest = this;
			return true;
		}
		
		return T_Base::QueryInterfaceImpl(typeUid, ppDest);
	}

};

template <class T_Anything> inline T_Anything* QueryInterface(I_Anything* pAnything); ///< Obtains the requested type.
template <class T_Anything> inline const T_Anything* QueryInterface(const I_Anything* pAnything); ///< Obtains the requested type.

template <class T_Anything> inline T_Anything* AnythingCast(I_Anything* pAnything); ///< Shortcut for downcasting.
template <class T_Anything> inline const T_Anything* AnythingCast(const I_Anything* pAnything); ///< Shortcut for downcasting.

} // namespace Sys


#include <misc_shared/Context/I_Anything.inl>

#endif // I_ANYTHING_2475852C_8712_439B_90E5_9896A8180F40_H
