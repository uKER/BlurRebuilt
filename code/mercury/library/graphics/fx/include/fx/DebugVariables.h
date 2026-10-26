#ifndef __FX_DEBUGVARIABLES_H__
#define __FX_DEBUGVARIABLES_H__

#include <Btl\Containers\Vector.h>

namespace FX
{

class Effect;
struct Parameter;

namespace DebugVariables
{

enum E_VarType
{
	E_VarType_None,
	E_VarType_Float,
	E_VarType_Float2,
	E_VarType_Float3,
	E_VarType_Colour,
	E_VarType_Array,
};

// a single parameter associated with a debug variable
struct Parameter
{
	Util::C_String sName;
	Util::C_String sFXParamName;
	Util::C_String *psDbgVarName;
	Debug::VarBase *pDbgVar;
	E_VarType type;
	FX::Parameter *pFXparam;
	Btl::Vector<Debug::VarBase*> dbgVarVector; //Used for array type
};

// a group of shaders associated with a single debugview page
class Group
{
public:
	/// returns the parameter type
	E_VarType GetParameterType(const FX::Parameter *p, Util::C_String *psOutName);

	/// add tweakable parameter to this group
	void AddParameter(const Effect *pEffect, E_VarType type, const char *pszName, const FX::Parameter *pParam);

	/// add the parameters of the given effect into the group
	void AddEffect(const Effect *pEffect);

	/// remove the given effect from this group
	void ReleaseEffect(const Effect *pEffect);

	/// update variables in the effects with the overrides from debugvariables
	void Update();

	Util::C_String m_sName;
	Util::C_String *m_psDbgVarName;
	Debug::Bool *m_pDbgVar;
	Btl::Vector<const Effect*> m_Effects;
	Btl::Vector<Parameter> m_Parameters;
};

class Manager
{
public:
	/// destructor
	~Manager();

	/// update variables in the effects with the overrides from debugvariables. to be called once per frame
	void Update();

	/// add the parametes for this effect into the debug variable system
	void AddEffect(const Effect *pEffect);

	/// called by the effects system when an effect is freed
	void ReleaseEffect(const Effect *pEffect);

	/// return a new string
	Util::C_String *NewString();

private:
	/// return a pointer to the group with the given name
	/// create a new group if necessary
	Group *GetGroup(const char *pszName);

	/// return the index of the group with the given name
	int FindGroup(const char *pszName);

	Btl::Vector<Group> m_Groups;
	Btl::Vector<Util::C_String*> m_Strings;
};

extern Manager g_Manager;

}

extern bool _g_bEnableFXDebugVariables;

}

#endif
