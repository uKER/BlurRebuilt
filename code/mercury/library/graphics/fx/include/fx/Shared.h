#ifndef NGLFX_SHARED_H
#define NGLFX_SHARED_H

#ifndef FX_ALLOW_INCLUDE
#error "Illegal include of FX library internals.h"
#endif

#include <system/scheduler/jobqueue.h>

namespace Display2
{
struct C_VertexShader;
}

namespace FX
{

class Effect;
struct Function;

#define MAX_SPGS 64

enum E_CompareSPGResult
{
	E_CompareSPGResult_MisMatch,	// some of the parameters don't match
	E_CompareSPGResult_Match,		// all of the parameters match exactly
	E_CompareSPGResult_Additional1, // all of the parameters match, but group 1 has additional parameters at the end
	E_CompareSPGResult_Additional2, // all of the parameters match, but group 2 has additional parameters at the end
	E_CompareSPGResult_Missing,		// some parameters are missing off group 1
};

/// a single shared parameter identifier
struct SharedParameter
{
	U8 Idx;		// shared parameter index
	U8 Size;	// size is counted in shader constant registers or texture samplers
	U16 Type	: 4;	// resource type (E_SPGResource)
	U16 Handle	: 10;	// shader constant or texture sampler number
	int Offset;	// offset of data in shared parameter data block

	bool operator<(const SharedParameter &e) const
	{
		return Handle<e.Handle;
	}
};

/// represents a single group of shared parameters
class SharedParameterGroup
{
public:
	int iGUID;
	Btl::Vector<SharedParameter> m_Parameters;
	Btl::Vector<FunctionBinding> bindings;
};

/// A special type of effect that has all of the global shared parameters on it
class SharedEffect : public Effect
{
public:
	/// constructor
	SharedEffect();

	/// find a shared parameter and return its index
	int GetSharedParameterIdxByName(const char *Name);

	/// return a pointer to the parameter with the specified index
	Parameter* GetSharedParameterByIdx(int idx) { return &Parameters[idx]; }

	/// bind a shared parameter
	Parameter *BindSharedParameter(FunctionBinding* binding, Parameter* param);

	/// get nuber of registered shared parameters
	int GetNumSharedParameters();

	/// restore default parameter data at the end of every frame
	/// this is to stop pointers to deleted textures hanging around in the FX system
	void RestoreDefaultData();

	/// add the shared parameters of this shader into the unique shared binding list
	void RegisterShader(Effect *pEffect, Btl::Vector<FunctionBinding> &bindings, int type);

	/// recalculate the shared parameter bitfields for each effect
	void UpdateSharedBindingBitfields();

	/// return the number of shared bindings
	int GetNumSharedBindings() { return m_SharedBindings.Size(); }

	/// return the shared binding with the given index
	SharedParameter *GetSharedBinding(int i) { return &m_SharedBindings[i]; }

	/// find the first binding for the given parameter
	SharedParameter *GetSharedBinding(const char *pszName);

	/// show all conflicting bindings for the given parameter
	void DumpConflictingBindings( Debug::Logger &log, Effect *pEffect, SharedParameter *p, int &nConflicts);

	/// returns true if the given shared parameter conflicts with any other bindings
	bool DoesSharedParameterConflict(int iSharedParameter, const SharedBindingBits *pUsage);

	/// show all global bindings
	void DumpGlobalBindings();

	/// get the bitfield representing which bindings are global
	SharedBindingBits &GetGlobalBindingBits() { return m_Global; }

	/// fetch all data required for rendering from the SPU
	static void SpuFetch(SharedEffect *&pThis)
	{
		Spu::FetchAlloc(pThis);
		Btl::SpuFetch(pThis->Parameters);
		Btl::SpuFetch(pThis->m_SharedBindings);
	}

private:
	/// find a shared parameter and return a pointer to it
	Parameter* GetSharedParameterByName(const char *Name);

	/// add new shared parameter
	Parameter *AddSharedParameter(Parameter *param);

	/// initialise new shared parameter
	void InitialiseSharedParameter(Parameter *pParam, Parameter *param);

	/// calculate the shared binding bitfields for the given bindings
	void CalculateSharedBindingBitfields(Effect *pEffect, Btl::Vector<FunctionBinding> &bindings, int type, bool bShared, SharedBindingBits &usage, SharedBindingBits &clear, SharedBindingBits &global);

	/// find a shared parameter in the unique shared binding list
	int FindSharedParameterBinding(const SharedParameter &binding);

	/// show all conflicting bindings for the given parameter
	void DumpConflictingBindings( Debug::Logger &log, SharedParameter *p1, Effect *pEffect, Btl::Vector<FunctionBinding> &bindings, int type, int &nConflicts);

	/// calculate the shared binding bitfields on the given effect
	void CalculateSharedBindingBitfields(Effect *pEffect, SharedBindingBits &global);

	/// mask any shared binding bitfields which conflict with maths constants bindings on the vertex shader
	void CalculateSharedBindingBitfields(Effect *pEffect, Display2::C_VertexShader *pShader, SharedBindingBits &mask, SharedBindingBits &global);

private:
	/// list of all unique shared parameter bindings
	Btl::Vector<SharedParameter>	m_SharedBindings;

	/// list of all the groups, ordered by GUID, one for each resource type
	Btl::Vector<SharedParameterGroup*> m_Group[E_SPGResource_Max];

	/// for each type of parameter, specify the bind type
	static const BindingType m_ParamTypeToBindType[];

	/// table to store which binding to start re-uploading variables from when switching SPGs
	U8 m_SPGTable[E_SPGResource_Max][MAX_SPGS][MAX_SPGS];

	/// true if all shaders need their shared binding bitfields updating
	bool m_bUpdateSharedBindingBitfields;

	/// bitmask representing all shared parameters that don't conflict with any other bindings or local parameters on any effect
	SharedBindingBits m_Global;
};

extern SharedEffect g_SharedEffect;

}

#endif
