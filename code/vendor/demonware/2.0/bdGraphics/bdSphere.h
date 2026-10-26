// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_SPHERE_H
#define BD_SPHERE_H

#include <bdCore/bdCore.h>

#if !defined(BD_PLATFORM_UNIX) || defined(BD_PLATFORM_APPLE)
#include <bdGraphics/bdMesh.h>
#else
#include <bdGraphics/bdVector4.h>
#include <bdGraphics/bdColor.h>

class bdVertex;

#endif

/// Sphere class.
/// Class to represent a graphical sphere object. Inherits from mesh and adds its own init method to allow a sphere
/// to be created.
#if !defined(BD_PLATFORM_UNIX) || defined(BD_PLATFORM_APPLE)
class bdSphere : public bdMesh
#else
class bdSphere
#endif 
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Create a sphere while constructing.
		/// \param radius[in]	The radius of the sphere.
		bdSphere(bdFloat32 radius = 1.0f);

		virtual ~bdSphere();

		/// Sphere creation method.
		/// Makes this object a sphere mesh with the given radius and using the given number of slices and stacks
		/// to specify the amount of tessellation.
		/// \param radius[in]	The radius of the sphere.
		/// \return				True if successful, false if it fails.
		bdBool init(bdFloat32 radius = 1.0f);

	protected:

		/// The number of slices to create a sphere with.
		/// This controls the degree of tessellation of the sphere.
		static const bdUInt m_numSlices;

		/// The number of stacks to create a sphere with.
		/// This controls the degree of tessellation of the sphere.
		static const bdUInt m_numStacks;

		/// A static variable counting the number of instances of this class.
		static bdUInt m_numSpheres;

		/// A static variable counting the number of vertices in the sphere vertex buffer.
		/// This is used to enable all instances of this class to share a single vertex buffer and thus save memory.
		static bdUInt m_numSphereVertices;

		/// A static pointer to the vertex buffer.
		/// This is used to enable all instances of this class to share a single vertex buffer and thus save memory.
		static bdVertex *m_sphereVertexBuffer;
#if defined(BD_PLATFORM_UNIX) && !defined(BD_PLATFORM_APPLE)
	public:
		void getPosition(bdVector4 meshPos)const;

		void setPosition(bdVector4 meshPos);
		void setColor(const bdColor& color);
		void getColor(bdColor& color) const;
		bdBool isVisible() const;
		void setVisible(bdBool visible);

#endif

};

#endif // BD_SPHERE_H
