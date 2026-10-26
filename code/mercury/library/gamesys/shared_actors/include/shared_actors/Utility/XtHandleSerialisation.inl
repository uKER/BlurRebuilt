#include <misc_shared/Context/ContextualStream.h>


/**
@param handle handle to be serialised.
@returns serialisable id.
*/
template <class T_DataType>
inline XtId I_XtHandleContext::GetXtHandleId(const XtHandle<T_DataType>& handle) const
{
	return handle.Id();
}

/**
@param id serialisable id.
@returns deserialised handle.
*/
template <class T_DataType>
inline XtHandle<T_DataType> I_XtHandleContext::GetXtHandle(const XtId id) const
{
	XtRecord* pRecord = GetXtRecord(id);

	if (pRecord == NULL)
		return XtHandle<T_DataType>();

	BIZ_ASSERT(XtIsBaseOf(XtGetType<T_DataType>(), pRecord->type));

	return XtHandle<T_DataType>(pRecord);
}






template <class T_DataType>
void Serialise(Sys::C_ContextualStream& ostream, const XtHandle<T_DataType>& value)
{
	ostream << ostream.GetContextItem<I_XtHandleContext>()->GetXtHandleId(value);
}

template <class T_DataType>
void Deserialise(Sys::C_ContextualStream& istream, XtHandle<T_DataType>& value)
{
	XtId temp = 0;
	istream >> temp;
	value = istream.GetContextItem<I_XtHandleContext>()->GetXtHandle<T_DataType>(temp);
}
