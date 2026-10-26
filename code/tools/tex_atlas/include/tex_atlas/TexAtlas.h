#ifndef TEX_ATLAS_H
#define TEX_ATLAS_H

#pragma once

#include <string>
#include <sstream>

#include <core/core.h>
#include <core/containers.h>
#include <core/containers/sortedbinarytree.h>

#include <aftk/Aftk.h>
#include <attk/Attk.h>

#include <tex_atlas/Definitions.h>
#include <lite_maths/LiteMathsHeaders.h>
using namespace LiteMaths;

#include <tools_common/Status.h>
#include <tools_common/GenericDiskAccess.h>
#include <tools_common/GenericDiskWriter.h>
#include <tools_common/GenericDiskReader.h>

#include <tex_atlas/Space.h>
#include <tex_atlas/TextOutput.h>
#include <tex_atlas/Progress.h>
#include <tex_atlas/TextureKey.h>
#include <tex_atlas/ImageCache.h>
#include <tex_atlas/TexturePackingLists.h>
#include <tex_atlas/Database.h>
#include <tex_atlas/CropSheet.h>
#include <tex_atlas/FixedSizeCropSheet.h>
#include <tex_atlas/ScaleCropSheet.h>
#include <tex_atlas/GridCropSheet.h>
#include <tex_atlas/AutoScaleCropSheet.h>
#include <tex_atlas/ImageHeader.h>

#include "file_tag_db\FileTagDB.h"

#endif
