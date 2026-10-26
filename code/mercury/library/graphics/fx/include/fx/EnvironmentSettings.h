#ifndef __FX_ENVSETTINGS_H__
#define __FX_ENVSETTINGS_H__

namespace FX
{

struct Parameter;

/// a block of environment settings
class I_EnvironmentSettings
{
public:
	/// destructor
	virtual ~I_EnvironmentSettings() {};

	/// cause the settings for this block to be copied to the destination output (fx parameters etc.)
	virtual void Set() = 0;

	/// returns a GUID for this settings block
	virtual Util::C_String Name() const = 0;

	/// returns a new settings block with is a lerped version of this and the other settings block passed in
	virtual I_EnvironmentSettings *Lerp(const I_EnvironmentSettings *pBlock, float f) const = 0;

	/// associate this block of environment settings with the given debug variable or debug variable page
	virtual void AssociateDebugVariables(const Util::C_String &sName) = 0;

	/// return a pointer to the environmentsettings class with the given name
	virtual I_EnvironmentSettings *GetSettingsBlockByName(const char *pszName);
};

/// a collection of environment settings
class EnvironmentSettings : public I_EnvironmentSettings
{
public:
	/// destructor
	~EnvironmentSettings();

	/// cause these environment settings to be applied to the scene
	void Set();

	/// returns a new environment settings block with is a lerped version of this and the other environment settings passed in
	I_EnvironmentSettings *Lerp(const I_EnvironmentSettings *p, float f) const;

	/// GUID for this settings block
	Util::C_String Name() const;

	/// add a settings block
	void AddBlock(I_EnvironmentSettings *pBlock);

	/// associate this block of environment settings with the given debug variable or debug variable page
	void AssociateDebugVariables(const Util::C_String &sName);

	/// return a pointer to the environmentsettings class with the given name
	I_EnvironmentSettings *GetSettingsBlockByName(const char *pszName);
private:
	Btl::Vector<I_EnvironmentSettings*> m_Blocks;
};

struct PersistentItem;

void **GetPersistentDataInternal(const Util::C_String &sName);

/// return a pointer to the persistent debug variable data
template<typename T>
T *GetPersistentData(const Util::C_String &sName)
{
	T **pp = (T**)GetPersistentDataInternal(sName);
	if (!*pp)
		*pp = new T;
	return *pp;		
}

} // namespace

#endif
