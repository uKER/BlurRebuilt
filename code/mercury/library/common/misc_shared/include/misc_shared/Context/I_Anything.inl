namespace Sys
{

/**
@brief Obtains a pointer to the requested class.
@param typeUid type identifier.
@param ppDest receives the pointer.
@returns true if the class was available.
*/
inline bool I_Anything::QueryInterface(const C_AnythingUid typeUid, void** ppDest) 
{
	assert(ppDest != NULL);
	return QueryInterfaceImpl(typeUid, ppDest);
}

/**
@brief Obtains a pointer to the requested class.
@param typeUid type identifier.
@param ppDest receives the pointer.
@returns true if the class was available.
*/
inline bool I_Anything::QueryInterface(const C_AnythingUid typeUid, const void** ppDest) const
{
	assert(ppDest != NULL);
	return const_cast<I_Anything*>(this)->QueryInterfaceImpl(typeUid, const_cast<void**>(ppDest));
}



// implementation detail
template <class T_Anything>
inline T_Anything* QueryInterfaceImpl(I_Anything* pAnything)
{
	T_Anything* pObj;

	if (!pAnything->QueryInterface(T_Anything::ClassUid(), &pObj))
		pObj = NULL;

	return pObj;
}

/**
@param T_Anything C_Anything-derived class to obtain.
@param pAnything object to obtain from.
@returns NULL if the object is not of this type.
*/
template <class T_Anything>
inline T_Anything* QueryInterface(I_Anything* pAnything)
{
	return QueryInterfaceImpl<T_Anything>(pAnything);
}

/**
@param T_Anything C_Anything-derived class to obtain.
@param pAnything object to obtain from.
@returns NULL if the object is not of this type.
*/
template <class T_Anything>
inline const T_Anything* QueryInterface(const I_Anything* pAnything)
{
	return QueryInterfaceImpl<T_Anything>(const_cast<I_Anything*>(pAnything));
}


// implementation detail
template <class T_Anything>
inline T_Anything* AnythingCastImpl(I_Anything* pAnything)
{
	assert((pAnything != NULL) && pAnything->IsTypeOf(T_Anything::ClassUid()));
	return static_cast<T_Anything*>(pAnything);
}

/**
@param T_Anything C_Anything-derived derived class.
@param pAnything object to cast, must be exactly of type T_Anything.
@returns derived object.
*/
template <class T_Anything>
inline T_Anything* AnythingCast(I_Anything* pAnything)
{
	return AnythingCastImpl<T_Anything>(pAnything);
}

/**
@param T_Anything C_Anything-derived derived class.
@param pAnything object to cast, must be exactly of type T_Anything.
@returns derived object.
*/
template <class T_Anything>
inline const T_Anything* AnythingCast(const I_Anything* pAnything)
{
	return AnythingCastImpl<T_Anything>(const_cast<I_Anything*>(pAnything));
}

} // namespace Sys
