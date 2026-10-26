// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline
bdD3DDevWrapper::bdD3DDevWrapper()
{
	m_device = BD_NULL;
}

inline
bdD3DDevWrapper::~bdD3DDevWrapper()
{
	if (m_device)
	{
		m_device->Release();
	}
}

inline
void bdD3DDevWrapper::setDevice(BD_D3D_DEV const device)
{
	m_device = device;
}

inline
BD_D3D_DEV bdD3DDevWrapper::getDevice()
{
	return m_device;
}
