#include <stdafx.h>

using namespace Aftk;

C_MaterialNode::C_MaterialNode() : C_Node(0)
{
	OneTimeInit();
}

C_Node* C_MaterialNode::Clone(C_Scene* pScene) const
{
	return new C_MaterialNode(*this);
}

C_MaterialNode::~C_MaterialNode()
{
	ClearThis();
}

C_MaterialNode::C_MaterialNode(const C_MaterialNode &rhs) : C_Node(0)
{
	OneTimeInit();
	*this = rhs;
}

void C_MaterialNode::AddPlugToPointerMapping(
	Ut::UtMap<const I_Object*, I_Object*>& pointerMapping, 
	const I_Plug* oldPlug,
	I_Plug* newPlug)
{
	//std::cout << "Added mapping " << oldPlug << " -> " << newPlug << std::endl;
	pointerMapping[oldPlug] = newPlug;

	//Recurse children...
	AFTK_ASSERT(oldPlug->GetNumChildren() == newPlug->GetNumChildren());
	for (U32 i = 0; i < oldPlug->GetNumChildren(); i++)
	{
		AddPlugToPointerMapping(
			pointerMapping,
			oldPlug->GetChildPtr(i), 
			newPlug->GetChildPtr(i));
	}
}

void C_MaterialNode::ResolvePlugPointerMapping(
	const Ut::UtMap<const I_Object*, I_Object*>& pointerMapping, 
	const I_Plug* oldPlug,
	I_Plug* newPlug)
{
	const I_Object* pOldConnection = oldPlug->GetSourceConnectionPtr();
	//If we had a connection...
	if (pOldConnection)
	{
		Ut::UtMap<const I_Object*, I_Object*>::ConstIterator it = 
			pointerMapping.Find(pOldConnection);
		if (it != pointerMapping.End())
		{
			newPlug->SetSourceConnectionPtr(it->GetValue());
		}
	}

	//Recurse children...
	for (U32 i = 0; i < oldPlug->GetNumChildren(); i++)
	{
		ResolvePlugPointerMapping(
			pointerMapping,
			oldPlug->GetChildPtr(i), 
			newPlug->GetChildPtr(i));
	}
}

C_MaterialNode& C_MaterialNode::operator=(const C_MaterialNode &rhs)
{
	//Copy base class info
	C_Node::operator=(rhs);

	ClearThis();

	//Clear plugs.
	for (U32 i = 0; i < (U32)m_plugs.Size(); i++)
		delete m_plugs[i];
	m_plugs.Resize(rhs.m_plugs.Size());

	//Clear shaders
	for (U32 i = 0; i < (U32)m_shaders.Size(); i++)
		delete m_shaders[i];
	m_shaders.Resize(rhs.m_shaders.Size());
	Ut::UtMap<const I_Object*, I_Object*> pointerMapping;

	//Add 'this' to our pointer mapping table
	pointerMapping[&rhs] = this;
	//std::cout << "Added " << &rhs << " -> " << this << std::endl;

	//Clone shader nodes and generate pointer mappings.
	for (U32 i = 0; i < (U32)rhs.m_shaders.Size(); i++)
	{
		m_shaders[i] = rhs.m_shaders[i]->Clone();
		//Add to old->new mapping
		pointerMapping[rhs.m_shaders[i]] = m_shaders[i];
		//std::cout << "Added " << rhs.m_shaders[i] << " -> " << m_shaders[i] << std::endl;
		m_shaders[i]->SetParentPtr(this);

		AFTK_ASSERT(rhs.m_shaders[i]->GetNumPlugs() == m_shaders[i]->GetNumPlugs());
		for (U32 j = 0; j < (U32)rhs.m_shaders[i]->GetNumPlugs(); j++)
		{
			AddPlugToPointerMapping(
				pointerMapping,
				rhs.m_shaders[i]->GetPlugPtr(j), 
				m_shaders[i]->GetPlugPtr(j));
		}
	}

	//Clone plugs for material and generate pointer mappings.
	for (U32 i = 0; i < (U32)rhs.m_plugs.Size(); i++)
	{
		m_plugs[i] = new C_DynamicPlug(*rhs.m_plugs[i]);
		//Add to old->new mapping
		pointerMapping[rhs.m_plugs[i]] = m_plugs[i];
		//Recurse child plugs.
		AddPlugToPointerMapping(pointerMapping, rhs.m_plugs[i], m_plugs[i]);
		m_plugs[i]->SetLogicalIndex(i);
		m_plugs[i]->SetParentPtr(this);
	}
	
	//Re-apply connectivity information for material plugs.
	for (U32 i = 0; i < (U32)m_plugs.Size(); i++)
	{
		ResolvePlugPointerMapping(pointerMapping, rhs.m_plugs[i], m_plugs[i]);
	}

	//Re-apply connectivity information for shader plugs.
	for (U32 i = 0; i < (U32)m_shaders.Size(); i++)
	{
		for (U32 j = 0; j < (U32)rhs.m_shaders[i]->GetNumPlugs(); j++)
		{
			ResolvePlugPointerMapping(
				pointerMapping,
				rhs.m_shaders[i]->GetPlugPtr(j), 
				m_shaders[i]->GetPlugPtr(j));
		}
	}

	m_numImages = rhs.m_numImages;
	m_origin = rhs.m_origin;
	m_materialTypeId = rhs.m_materialTypeId;
	return *this;
}

E_ObjType C_MaterialNode::GetType() const
{
	return E_ObjType_Material;
}

bool C_MaterialNode::HasType(E_ObjType type) const
{
	if (C_Node::HasType(type))
		return true;
	if (type == C_MaterialNode::GetType()) 
		return true;

	return false;
}

bool C_MaterialNode::operator==(const C_MaterialNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_MaterialNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest) const
{
	return IsEqualToAll(rhs, classToAbortTest);
}

bool C_MaterialNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest) const
{
	if (classToAbortTest == E_ObjType_Material)
		return IsEqualToThis(rhs);
	return (IsEqualToThis(rhs) && C_Node::IsEqualToAll(rhs, classToAbortTest));
}

bool C_MaterialNode::IsEqualToThis(const I_Object &_rhs) const
{
	if (!_rhs.HasType(E_ObjType_Material))
	{
		return false;
	}

	const C_MaterialNode& rhs = (C_MaterialNode&)_rhs;

	// compare base classes
	C_Node* pNodeLhs = (C_Node*)this;
	C_Node* pNodeRhs = (C_Node*)&rhs;

	if (*pNodeLhs != *pNodeRhs)
	{
		return false;
	}

	// base classes are equal, lets look the members of C_MaterialNode
	if (GetType() != rhs.GetType() ||
		m_materialTypeId != rhs.m_materialTypeId ||
		m_numImages != rhs.m_numImages ||
		m_origin != rhs.m_origin
		) 
	{
		return false;
	} 

	if (m_plugs.Size() != rhs.m_plugs.Size())
	{
		return false;
	} 
	else
	{
		// each material has the same number of attributes
		U32 i;
		for (i = 0; i < (U32)m_plugs.Size(); i++) 
		{
			// Use the comparison operator of the C_Plug class
			if (!m_plugs[i]->IsEqualTo(*rhs.m_plugs[i])) 
			{
				return false;
			}
		}
	}

	if (m_shaders.Size() != rhs.m_shaders.Size())
	{
		return false;
	} 
	else 
	{
		// each material has the same number of attributes
		for (U32 i = 0; i < (U32)m_shaders.Size(); i++) 
		{
			const C_ShaderNode* pShaderLhs;
			const C_ShaderNode* pShaderRhs;
			pShaderLhs = GetShaderNodePtr(i);
			pShaderRhs = rhs.GetShaderNodePtr(i);

			// Use the comparison operator of the C_ShaderNode class
			if (!pShaderLhs->IsEqualTo(*pShaderRhs))
			{
				return false;
			}
		}
	}
	return true;
}

bool C_MaterialNode::operator!=(const C_MaterialNode &rhs) const
{
	if ( *this == rhs )
	{
		return false;
	}
	else 
	{
		return true;
	}
}

void C_MaterialNode::SetOrigin(E_MaterialOrigin origin)
{
	m_origin = origin;
}

void C_MaterialNode::SetMaterialTypeId(U32 id)
{
	m_materialTypeId = id;
}

void C_MaterialNode::SetPlugInitialCapacity(U32 capacity)
{
	if ((U32)m_plugs.Capacity() < capacity)
	{
		m_plugs.Reserve(capacity);
	}
}

void C_MaterialNode::SetShaderNodeInitialCapacity(U32 capacity)
{
	if ((U32)m_shaders.Capacity() < capacity)
	{
		m_shaders.Reserve(capacity);
	}
}

U32 C_MaterialNode::AddShaderNodePtr(C_ShaderNode* pShader)
{
	//Attach this shader to this material
	pShader->SetNodeInstance((U32)m_shaders.Size());
	pShader->SetMaterialNodePtr(this);

	m_shaders.PushBack(pShader);

	if (pShader->GetType() == E_ObjType_ShaderImage)
	{
		m_numImages++;
	}
	
	return (U32)m_shaders.Size() -1;
}

void C_MaterialNode::SetShaderNodePtr(C_Scene* pScene, const C_SharedResource<C_MaterialNode>* pRes, U32 instance, C_ShaderNode* pShader)
{
	AFTK_ASSERT(pScene);
	// If the shader being overwritten is a shader image
	// we may need to do some syncronisation of UV linking info
	if (m_shaders[instance]->GetType() == E_ObjType_ShaderImage)
		pScene->SyncMaterials(pRes, m_shaders[instance], (C_ShaderImageNode*)pShader);

	delete m_shaders[instance];
	m_shaders[instance] = pShader;
	pShader->SetMaterialNodePtr(this);
}

E_MaterialOrigin C_MaterialNode::GetOrigin() const
{
	return m_origin;
}

U32 C_MaterialNode::GetMaterialTypeId() const
{
	return m_materialTypeId;
}

const C_ShaderNode* C_MaterialNode::GetShaderNodePtr(U32 instance) const
{
	return m_shaders[instance];
}

C_ShaderNode* C_MaterialNode::GetShaderNodePtr(U32 instance)
{
	return m_shaders[instance];
}

const C_ShaderNode* C_MaterialNode::GetShaderNodePtr(const std::string& name) const
{
	for (U32 i = 0; i < (U32)m_shaders.Size(); i++)
	{
		if (m_shaders[i]->GetName() == name)
		{
			return m_shaders[i];
		}
	}
	return 0;
}

C_ShaderNode* C_MaterialNode::GetShaderNodePtr(const std::string& name)
{
	for (U32 i = 0; i < (U32)m_shaders.Size(); i++)
	{
		if (m_shaders[i]->GetName() == name)
		{
			return m_shaders[i];
		}
	}
	return 0;
}

S32 C_MaterialNode::AddPlugPtr(C_DynamicPlug* newPlug)
{
	m_plugs.PushBack(newPlug);
	newPlug->SetLogicalIndex(m_plugs.Size()-1);
	newPlug->SetParentPtr(this);
	return (U32)m_plugs.Size() -1;
}

const C_DynamicPlug* C_MaterialNode::GetPlugPtr(U32 instance) const
{
	return m_plugs[instance];
}

C_DynamicPlug* C_MaterialNode::GetPlugPtr(U32 instance)
{
	return m_plugs[instance];
}

U32	C_MaterialNode::GetNumPlugs() const
{
	return (U32)m_plugs.Size();
}

U32 C_MaterialNode::GetNumShaderNodes() const
{
	return (U32)m_shaders.Size();
}

U32 C_MaterialNode::GetNumShaderImageNodes() const
{
	return m_numImages;
}

const C_ShaderImageNode* C_MaterialNode::GetShaderImageNodePtr(U32 instance) const
{
	U32 instanceCount = 0;
	for (U32 i = 0; i < (U32)m_shaders.Size(); i++)
	{
		if (m_shaders[i]->GetType() == E_ObjType_ShaderImage)
		{
			if (instanceCount == instance)
			{
				return (C_ShaderImageNode*)m_shaders[i];
			}
			else
			{
				instanceCount++;
			}
		}
	}

	return 0;
}

C_ShaderImageNode* C_MaterialNode::GetShaderImageNodePtr(U32 instance)
{
	U32 instanceCount = 0;
	for (U32 i = 0; i < (U32)m_shaders.Size(); i++)
	{
		if (m_shaders[i]->GetType() == E_ObjType_ShaderImage)
		{
			if (instanceCount == instance)
			{
				return (C_ShaderImageNode*)m_shaders[i];
			}
			else
			{
				instanceCount++;
			}
		}
	}

	return 0;
}

void C_MaterialNode::Output(const C_MaterialNode& material)
{
	//Display basic material proporties
	AFTK_DO_BEGIN << "--- MATERIAL BEGIN ---\n" AFTK_DO_INFO_END;
	AFTK_DO_BEGIN << "Name: " << material.GetName() << "\n" AFTK_DO_INFO_END;

	switch (material.GetOrigin())
	{
	case E_MaterialOrigin_Editor:
		AFTK_DO_BEGIN << "Origin: Editor\n" AFTK_DO_INFO_END;
		break;

	case E_MaterialOrigin_Maya:
		AFTK_DO_BEGIN << "Origin: Maya\n" AFTK_DO_INFO_END;
		break;
	
	case E_MaterialOrigin_Maya_CgFX:
		AFTK_DO_BEGIN << "Origin: Maya CgFX\n" AFTK_DO_INFO_END;
		break;
	
	case E_MaterialOrigin_XSI:
		AFTK_DO_BEGIN << "Origin: Softimage XSI\n" AFTK_DO_INFO_END;
		break;

	case E_MaterialOrigin_SI:
		AFTK_DO_BEGIN << "Origin: Softimage 3D\n" AFTK_DO_INFO_END;
		break;

	case E_MaterialOrigin_Maya_Hlsl_2008:
		AFTK_DO_BEGIN << "Origin: Maya 2008 Hlsl\n" AFTK_DO_INFO_END;
		break;

	default:
		break;
	}
	
	AFTK_DO_BEGIN << "Num plugs: " << material.GetNumPlugs() << "\n" AFTK_DO_INFO_END;
	AFTK_DO_BEGIN << "Num shader nodes (total): " << material.GetNumShaderNodes() << "\n" AFTK_DO_INFO_END;
	AFTK_DO_BEGIN << "Num image nodes: " << material.GetNumShaderImageNodes() << "\n" AFTK_DO_INFO_END;
	AFTK_DO_BEGIN << "Material type id: " << material.GetMaterialTypeId() << "\n" AFTK_DO_INFO_END;
	AFTK_DO_BEGIN << "\n--- Material Plugs ---\n" AFTK_DO_INFO_END;
	for (U32 i = 0; i < material.GetNumPlugs(); i++)
	{
		const I_Plug* pPlug = material.GetPlugPtr(i);
		AFTK_ASSERT(pPlug);
		I_Plug::Output(*pPlug);
	}

	AFTK_DO_BEGIN << "\n--- Material Shader Nodes ---\n" AFTK_DO_INFO_END;
	for (U32 i = 0; i < material.GetNumShaderNodes(); i++)
	{
		const C_ShaderNode* pShaderNode = material.GetShaderNodePtr(i);
		AFTK_DO_BEGIN << "Shader: " << material.GetPath() << " type: " AFTK_DO_INFO_END;
		switch (pShaderNode->GetType())
		{
			case E_ObjType_Shader:
			{
				AFTK_DO_BEGIN << "\n\nShader node\n" AFTK_DO_INFO_END;
				break;
			}
			case E_ObjType_ShaderImage:
			{
				C_ShaderImageNode* pShaderImageNode = (C_ShaderImageNode*)pShaderNode;
				AFTK_DO_BEGIN << "Image shader node\n" AFTK_DO_INFO_END;
				std::string fileName;
				pShaderImageNode->GetFilePlug().GetValue(fileName);
				AFTK_DO_BEGIN << "File name: " << fileName << "\n" AFTK_DO_INFO_END;
				break;
			}
			default:
			{
				AFTK_DO_BEGIN << "Unknown shader node type\n" AFTK_DO_INFO_END;
				continue;
			}
		}
		AFTK_DO_BEGIN << "Shader num plugs: " << pShaderNode->GetNumPlugs() << "\n" AFTK_DO_INFO_END;

		switch (pShaderNode->GetType())
		{
			case E_ObjType_Shader:
			case E_ObjType_ShaderImage:
			{
				for (U32 j = 0; j < pShaderNode->GetNumPlugs(); j++)
				{
					const I_Plug* pPlug = pShaderNode->GetPlugPtr(j);
					AFTK_ASSERT(pPlug);
					I_Plug::Output(*pPlug);
				}
				break;
			}
			default:
				break;
		}
	}

	AFTK_DO_BEGIN << "--- MATERIAL END ---\n" AFTK_DO_INFO_END;
}

void* C_MaterialNode::operator new (size_t size)
{
	return C_MemoryManager::PoolAllocate((U32)size);
}

void C_MaterialNode::operator delete(void* p)
{
	C_MemoryManager::PoolFree(sizeof(C_MaterialNode), p);
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_MaterialNode::Clear()
{
	ClearThis();
	C_Node::Clear();
}

void C_MaterialNode::OneTimeInit()
{
	m_numImages = 0;
	m_materialTypeId = 0;
}

void C_MaterialNode::ClearThis()
{
	for (U32 i = 0; i < (U32)m_plugs.Size(); i++)
	{
		delete m_plugs[i];
	}
	m_plugs.Clear();

	//Clear shaders
	for (U32 i = 0; i < (U32)m_shaders.Size(); i++)
	{
		delete m_shaders[i];
	}
	m_shaders.Clear();
}

#endif
