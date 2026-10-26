// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 XBOX XENON PS2 PSP-INFRA PSP-ADHOC PS3 WII OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_MESH_H
#define BD_MESH_H

#include <bdCore/bdCore.h>
#include <bdGraphics/bdVertex.h>
#include <bdGraphics/bdVector4.h>
#include <bdGraphics/bdColor.h>

/// Mesh class. Represents a drawable object using a vertex buffer and optional index buffer.
class bdMesh
{
public:
	/// The type that the mesh can take. Gives meaning to the order of the indices or vertices in their buffers.
	/// - \b BD_MESH_LINE_LIST The mesh is a list of lines. Every pair of vertices (indices if the mesh is indexed)
	/// specify one line.
	/// - \b BD_MESH_TRIANGLE_LIST The mesh is a list of triangles. Every triplet of vertices specifies a triangle.
	/// Triangles use counter-clockwise winding.
	/// - \b BD_MESH_TRIANGLE_STRIP The mesh is a strip of triangle. The first 3 vertices specify the first triangle
	/// and every vertex after that specifies another triangle, along with the last two vertices from the previous
	/// triangle.
	/// - \b BD_MESH_TRIANGLE_FAN The mesh is a radial fan of triangles. The first 3 vertices specify the first
	///	triangle and each vertex after that specifies another triangle. All triangles share the first vertex in the
	/// list.
	enum bdMeshType
	{
		BD_MESH_LINE_LIST,
		BD_MESH_TRIANGLE_LIST,
		BD_MESH_TRIANGLE_STRIP,
		BD_MESH_TRIANGLE_FAN
	};

public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Default constructor. Gives all member variables default values and adds the mesh to the scene.
	bdMesh();

	virtual ~bdMesh();

	/// Changes the position of the mesh in the scene.
	/// \param position[in] The new position of the mesh.
	void setPosition(const bdVector4 &position);

	/// Gets the position of the mesh in the scene.
	/// \param position[out] The current position of the mesh.
	void getPosition(bdVector4 &position) const;

	/// Sets the color of the mesh.
	/// \param color[in] The new color of the mesh.
	void setColor(const bdColor &color);

	/// Gets the color of the mesh.
	/// \param color[out] The current color of the mesh.
	void getColor(bdColor &color) const;

	/// Control whether or not the mesh is drawn.
	/// Note that visibility does not determine whether the mesh will appear on the screen. For example, if the mesh
	/// is moved behind the camera, it will not be shown on screen no matter what the visibility is set to.
	/// \param visible[in]
	/// - True, the mesh should be drawn
	/// - False, the mesh should not be drawn
	void setVisible(const bdBool visible);

	/// Determines whether or not the graphics engine draws the mesh.
	/// Note that visibility does not determine whether the mesh will appear on the screen. For example, if the mesh
	/// is moved behind the camera, it will not be shown on screen no matter what the visibility is set to.
	/// \return
	/// - True, if the mesh will be drawn in the next scene
	/// - False, if the mesh will not be drawn in the next scene
	bdBool isVisible() const;

	/// Clears the mesh by freeing up allocated memory and releasing the vertex and index buffers.
	void clear();

	/// Initialize the mesh using the provided index and vertex buffers.
	/// \param type[in] The type of the mesh.
	/// \param numIndices[in] The number of indices in \a indexBuffer. Set to zero if the mesh in non-indexed.
	/// \param numVertices[in] The number of vertices in \a vertexBuffer.
	/// \param indexBuffer[in] A pointer to an array of indices to the vertexBuffer. If \a numIndices is zero, this
	/// parameter is ignored. The buffer's length is given by \a numIndices.
	/// \param vertexBuffer[in] A pointer to an array of vertices used represent the mesh.
	bdBool init(bdMeshType type,
				bdUInt numIndices,
				bdUInt numVertices,
				const bdUInt16 *const indexBuffer,
				const bdVertex *const vertexBuffer);

	/// Draws the mesh if \a m_visible is true.
	void draw() const;

protected:
	/// The type of the mesh's primitives.
	bdMeshType m_meshType;

	/// The number of indices in the index buffer. If this is zero, the index buffer is not used in drawing.
	bdUInt m_numIndices;

	/// The number of vertices in the vertex buffer.
	bdUInt m_numVertices;

	/// A pointer to the index buffer.
	/// If the mesh is indexed, the elements of the index buffer are indices into the vertex buffer. This can save
	/// memory because neighboring faces require duplication of vertices. If indices are used, only the index is
	/// duplicated, not the whole vertex.
	bdUInt16 *m_indexBuffer;

	/// A pointer to the vertex buffer.
	bdVertex *m_vertexBuffer;

	/// The color of the mesh
	bdColor	m_color;

	/// The position of the mesh in the 3D scene.
	bdVector4 m_position;

	/// If:
	/// - True, The mesh will be drawn by a call to \a draw()
	/// - False, The mesh will not be drawn by a call to \a draw()
	bdBool m_visible;

	/// Controls whether the mesh's buffers should be deallocated by this class or by a subclass. This is used so that
	/// the subclass has more control over the memory allocated for it. For example, so that all instances of a sub
	/// class share one index/vertex buffer.
	/// If:
	/// - True, The mesh will be destroyed by this class
	/// - False, The mesh will be destroyed by the subclass
	bdBool m_meshDestroy;
};

#endif // BD_MESH_H

