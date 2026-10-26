#ifndef XTHANDLECONTEXT_8017CFE8_DFA7_4F7E_9A13_6E4F3FEBE70D_H
#define XTHANDLECONTEXT_8017CFE8_DFA7_4F7E_9A13_6E4F3FEBE70D_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/context/I_Anything.h>
#include <misc_shared/uid/FourCC.h>
#include <xt/XtGame.h>

namespace Sys { class C_ContextualStream; }


/**
@brief Interface for serialising XtHandle objects.
*/
class I_XtHandleContext : public Sys::C_Anything<FOURCC_U32('IXTH')>
{
public:
	virtual ~I_XtHandleContext();

	template <class T_DataType>
	inline XtId GetXtHandleId(const XtHandle<T_DataType>& handle) const; ///< Gets a serialisable id for this handle.

	template <class T_DataType>
	inline XtHandle<T_DataType> GetXtHandle(const XtId id) const; ///< Gets a handle from this serialisable id.

protected:
	I_XtHandleContext(const Sys::C_AnythingUid typeUid);

private:
	/// @brief Gets a record from the serialisable id.
	/// @param id serialisable id.
	/// @returns NULL if the record does not exist.
	virtual XtRecord* GetXtRecord(const XtId id) const = 0;

};

/**
@brief  Implements I_XtHandleContext using an Xt database object.
*/
class C_XtHandleDatabaseContext : public Sys::C_Anything<FOURCC_U32('XTHD'), I_XtHandleContext>
{
public:
	C_XtHandleDatabaseContext(XtDb& database);
	virtual ~C_XtHandleDatabaseContext();

private:
	XtDb& m_Database; ///< Xt database to search

	virtual XtRecord* GetXtRecord(const XtId id) const; // from I_XtHandleContext

};

// overload the default Serialise/Deserialise implementations for XtHandle
template <class T_DataType> void Serialise(Sys::C_ContextualStream& ostream, const XtHandle<T_DataType>& value);
template <class T_DataType> void Deserialise(Sys::C_ContextualStream& istream, XtHandle<T_DataType>& value);


#include <shared_actors/Utility/XtHandleSerialisation.inl>

#endif // XTHANDLECONTEXT_8017CFE8_DFA7_4F7E_9A13_6E4F3FEBE70D_H
