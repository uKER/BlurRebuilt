template<typename T>
bool C_PropertyVisibilityManager::Register(const Btl::String& displayName)
{
	if (!m_pMenu)
		return true;

	const char* type = T::GetStaticTypeName();

	C_PropertyVisibilityData visibilityData(this);
	visibilityData.SetTypeName(type);
	visibilityData.SetVisibilityType(E_PropertyVisibilityType_All);
	
	m_RegisteredProperties[type] = visibilityData;

	m_pMenu->AddProperty(displayName, &m_RegisteredProperties[type]);

	return true;
}
