#include "stdafx.h"

#include <btl/containers/vector.h>
#include "fx/Fx.h"
#include "fx/Effect.h"
#include "fx/Structs.h"

namespace FX
{
/*
/// a place to store the struct declarations from incoming shaders
class StructManager
{
public:
	/// add the given struct declaration into to the manager
	void Add(Struct *pDecl)
	{
		if (Get(pDecl->Name))
		{
			delete pDecl;
			return;
		}

		// add new struct
		m_Structs.PushBack(pDecl);
	}

	/// get a struct declaration for the given struct name
	Struct *Get(const Util::C_String &sName)
	{
		for (int i=0; i<m_Structs.Size(); i++)
		{
			if (m_Structs[i]->Name == sName)
				return m_Structs[i];
		}
		return NULL;
	}

private:
	Ut::Vector<Struct*> m_Structs;
};

static StructManager g_StructManager;

/// add the given struct declaration into to the manager
void RegisterStruct(Struct *pDecl)
{
	g_StructManager.Add(pDecl);
}

/// get a struct declaration for the given struct name
Struct *GetStruct(const Util::C_String &sName)
{
	return g_StructManager.Get(sName);
}
*/
} // namespace
