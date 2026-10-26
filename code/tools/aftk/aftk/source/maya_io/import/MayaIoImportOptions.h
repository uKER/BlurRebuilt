#ifndef MAYA_IO_IMPORT_OPTIONS_H
#define MAYA_IO_IMPORT_OPTIONS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	struct C_ImportOptions
	{
	public:
		C_ImportOptions()
		{
			Clear();
		}

		C_ImportOptions(const C_ImportOptions& rhs)
		{
			*this = rhs;
		}

		const C_ImportOptions& operator=(const C_ImportOptions& rhs)
		{
			strict = rhs.strict;
			importSelection = rhs.importSelection;
			triangulate = rhs.triangulate;
			mergeLodMeshes = rhs.mergeLodMeshes;
			deformedInstancesAsMesh = rhs.deformedInstancesAsMesh;
			validateMaterials = rhs.validateMaterials;
			importTangentSpace = rhs.importTangentSpace;
			maxNGonNumVertices = rhs.maxNGonNumVertices;
			absNumericRange = rhs.absNumericRange;
			importPolyBlindData = rhs.importPolyBlindData;
			project = rhs.project;
			meshWeld = rhs.meshWeld;
			zflip = rhs.zflip;
			return *this;
		}

		void Clear()
		{
			importSelection = false;
			triangulate = false;
			mergeLodMeshes = false;
			deformedInstancesAsMesh = false;
			validateMaterials = false;
			maxNGonNumVertices = 256;
			importTangentSpace = false;
			strict = false;
			//100 million range for valid coordinates.
			absNumericRange = 100000000;
			importPolyBlindData = true;
			meshWeld = true;
			zflip = true;
		}

	// Import options
		bool		strict;
		bool		importSelection;
		bool		triangulate;
		bool		mergeLodMeshes;
		bool		deformedInstancesAsMesh;
		bool		validateMaterials;
		bool		importTangentSpace;
		bool		importPolyBlindData;
		bool		meshWeld;
		bool		zflip;
		U32			maxNGonNumVertices;
		F32			absNumericRange;
		std::string project;
	};
}

#endif
