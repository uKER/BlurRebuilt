template<class T>
C_Status C_NativeStaticPlug<T>::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk, 
	const C_StaticPlug<T>* pPlug)
{
	C_Status status;
	status = C_NativeNullPlug::Serialise(
		pDiskWriter,
		pNativeFtk, 
		pPlug);
	if (!status) return status;

	*pDiskWriter << pPlug->m_value;
	return C_Status::E_Status_Success;
}

template<class T>
C_Status C_NativeStaticPlug<T>::Deserialise(
	C_NativeDiskReader* pDiskReader,
	C_NativeFtk* pNativeFtk, 
	C_StaticPlug<T>* pPlug)
{
	C_Status status;
	status = C_NativeNullPlug::Deserialise(
		pDiskReader,
		pNativeFtk, 
		pPlug);
	if (!status) return status;

	*pDiskReader >> pPlug->m_value;
	return C_Status::E_Status_Success;
}
