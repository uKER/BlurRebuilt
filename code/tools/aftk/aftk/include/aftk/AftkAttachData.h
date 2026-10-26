#ifndef AFTK_ATTACH_DATA_H
#define AFTK_ATTACH_DATA_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Node;
	class C_Polygon;
	class C_PolygonVertex;
	class C_SceneMeshNode;

	class C_AttachData
	{
	public:
		///
		///\brief Constructor.
		C_AttachData()
		{
			// no members that should be initialised.
		}
		
		///
		///\brief Destructor.
		virtual ~C_AttachData()
		{
			//nothing needs to be freed
		}
		
		///
		///\brief Reset this class to its initial state.
		///
		///Deallocate any memory acquired by this class, and reset member variables to their initial state.
		virtual void Clear() {};
		
	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		///
		///\brief Copy constructor. This should be provided in user derived classes.
		C_AttachData(const C_AttachData& rhs)
		{
			// nothing that should be copied
		}

		const C_AttachData& operator=(const C_AttachData& rhs);

		///\brief Instantiates a new instance of an attach data object. Pure virtual.
		///
		virtual C_AttachData* CreateClone() const = 0;

		// The following are friends because they make use of the CreateClone() and Copy functions.
		// These two functions are not to be made available to users directly.
		friend class C_Node;
		friend class C_Polygon;
		friend class C_PolygonVertex;
		friend class C_SceneMeshNode;
	#endif
	};
}

#endif
