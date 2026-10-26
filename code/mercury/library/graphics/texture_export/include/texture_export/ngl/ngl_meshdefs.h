#ifndef NGL_MESHDEFS_H
#define NGL_MESHDEFS_H

namespace apk
{
	class apkFile;
}

/// Describes a single bone in a mesh skeleton, including name and simple hierarchy information.
struct nglSkeletonBone
{
	/// Transform relative to parent bone.
	math::RotTranMat43 Relative;

	/// Inverse of rest pose matrix, used in skinning.
	math::RotTranMat43 InvRest;

	/// Name of the bone.
	Util::C_String Name;

	/// Index of parent bone.
	int       Parent;

	u32       Pad0;
};

/** Skeleton object.
   Skeletons are stored as named objects in APK files and are referenced by meshes (and possibly other data types).

   A skeleton stores a simple hierarchy of bones with both relative and inverse absolute transforms.

   The bone indices stored in the vertices of the mesh (when mapped through the section's bone indices table) match
   the order of the bones in the skeleton assigned to the mesh.
 */
struct nglSkeleton
{
	Util::C_String         Name; //< Name of the skeleton.

	int               NBones; //< Number of bones in the skeleton.
	nglSkeletonBone  * Bones; //< Table of structures describing each bone in the skeleton.
};

/** Base class for material objects.
   Materials are stored as named objects in APK files and are referenced by meshes (and possibly other data types).

   Conceptually, a material is a data object that describes how the faces of a mesh are rendered (light interaction, reflectivity,
   special effects, etc).

   Typically, each class derived from nglShader will be paired with a class derived from nglMaterial.  The material class
   declares all the parameters the shader expects to receive from the art package.

   NGL takes care of exporting these parameters from MAX and packing them into material objects.
 */
struct nglMaterial
{
	/// Name of the material.
	Util::C_String         Name;

	/// Pointer to the shader for this material, set up at load time.
	class nglShader  * Shader;

	/// This field allows changes to the .matdef file which can affect mesh files to be detected by the code.
	int               BinaryVersion;

	/// Pointer to allocate runtime data, created when the material is loaded/bound.
	/// Example: RuntimeData could point to a generated DMA chain on PS2, or to a structure containing textures created at load time.
	void             * RuntimeData;
};

/// Convenience typedef for older code.
typedef nglMaterial nglMaterialBase;

/** Simple LOD structure with a range and a reference to a mesh object.
 */
struct nglMeshLOD
{
	struct nglMesh  * Mesh; //< Mesh structure pointed to by the LOD.
	float            Range; //< Distance at which the mesh will swap to this LOD.
};

/** Table structure used to hold pointers to sections of a mesh.
   Each table entry stores a pointer to the section object as well as flags determining whether the mesh owns the section.
 */
struct nglMeshSectionTableEntry
{
	/// Flags describing the section table entry.  \see nglMeshSectionTableFlags
	u32                     Flags;

	// Pointer to the mesh section referenced by the table entry.
	struct nglMeshSection  * Section;

	// Future enhancement:
	//nglMaterial* Material;
};

/** Renderable mesh object
   Meshes are the basic unit of rendering in NGL.

   A mesh is a collection of faces and vertices, divided into sections.  Each section contains all the faces of a mesh that share the
   same material (and their vertices).  If a vertex is used by more than one section, it is duplicated.  Each section also has a
   reference to the material that is applied to the faces.

   Sections can be owned by the mesh, or not.  If owned, a section will be destroyed when the mesh is destroyed.  Sections can be added
   and removed (and ownership rights set) by changing the SectionTable structure in the nglMesh.  When a mesh shares its sections
   with another mesh but does not own them, it is considered a shallow copy.  It is the user's responsibility to ensure that only one
   mesh owns each section object.
 */
struct nglMesh
{
	Util::C_String                  Name; //< Name of the mesh.
	u32                        Flags; //< Flags describing the mesh. \see nglMeshFlags

	u32                        NSections; //< Number of sections in the section table.
	nglMeshSectionTableEntry  * Sections; //< Table of structures describing each section, with pointers to the sections.

	nglSkeleton               * Skeleton; //< Skeleton object referenced by the mesh.

	u32                        NLODs; //< Number of LODs in the mesh.
	nglMeshLOD                * LODs; //< Table of structures describing each LOD, with pointers to the mesh objects referenced.

	math::Vector4 Sphere;    //< Bounding sphere containing the mesh.  Sphere.xyz is the position and Sphere.w is the radius.
	math::Dir3 BoxSize;      //< Axis Aligned Bounding Box dimensions containing the mesh.  The box is centered around Sphere.xyz.

	apk::apkFile* File;      //< APK file the mesh is found in.  NULL for created meshes.

	u32                        NPolys; //< Number of polygons in the mesh.
	int                        LastFrameRef; //< Last frame number this resource was referenced on.  Used to determine when it is safe to release resources.

	u32                        Pad0;
};

//#include "ngl_section.h"

#endif
