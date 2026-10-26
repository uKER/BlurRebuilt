namespace Samples
{

INLINE const bool C_SectionLauncherBase::IsActive() const
{
	return m_pSection ? true : false;
}

INLINE const Util::C_String& C_SectionLauncherBase::GetName() const
{
	return m_Name;
}

INLINE const bool C_SectionManager::InSection() const
{
	return m_pCurrLauncher && m_pCurrLauncher->IsActive() ? true : false;
}

}; // namespace Samples
