#ifndef AFTK_COMPONENT_CLUSTER_H
#define AFTK_COMPONENT_CLUSTER_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace Aftk
{
	class C_ComponentCluster
	{
	public:
		
		///\brief Constructor.
		///
		C_ComponentCluster(C_SceneMeshNode* pMesh);

		///\brief Copy constructor.
		///
		///@param rhs The component cluster to copy.
	//	C_ComponentCluster(const C_ComponentCluster& rhs);

		///\brief Destructor.
		///
		virtual ~C_ComponentCluster();

		///\brief Asignment operator.
		///
		///@param rhs The component cluster to copy.
		///@return A reference to this node.
		const C_ComponentCluster& operator=(const C_ComponentCluster& rhs);

		///\brief Sets the name of this component cluster.
		///
		///@param name The name.
		void SetName(const std::string& name);

		///\brief Returns the mesh that this component cluster is associated with.
		///
		///@return The mesh.
		C_SceneMeshNode* GetMeshNodePtr();

		///\brief Returns the mesh that this component cluster is associated with.
		///
		///@return The mesh.
		const C_SceneMeshNode* GetMeshNodePtr() const;

		///\brief Returns the name of this component cluster.
		///
		///@return The name.
		const std::string& GetName() const;

		///\brief Returns the type of this component cluster.
		///
		///@return The cluster type.
		virtual E_ComponentClusterType GetType() const = 0;

		void Clear();
	    
	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		C_SceneMeshNode* m_pMeshNode;
		std::string m_name;
	#endif
	};
}

#endif
