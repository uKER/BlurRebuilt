scene_converter
===============

code\mercurygame\tools\scene_converter\docs\SceneProcessingFlow.jpg

Design Decisions:
-----------------

Task List:
----------

read light quality values from tools config xml file
handle lods
generate light sample points
build portals
build light list
build light sample kd-tree

make process graph aware of when data types can be freed
	data dependencies, i.e. scenedesc points to aftk nodes so lit aftk should not be freed if scendesc still required

extend object flags with other behaviour other than "or", add things similar to "Order_$$" to extract numerical values

Tidying :
---------

add some doxygen comments
include things like #include "btl/containers/vector.h" in stdafx

Other Issues:
-------------

use Btl::String instead of Util::C_String
use string.Isearch instead of _stricmp when it is implemented
use something other than Util::ShellSortArray<>

Done:
-----
X add render process and data
X add final "export" process
X Write high-level level.
X put in option to not run lighting tool - skip lighting stage, use preprocessed aftk instead of lit as input
X Unlit path
X add physics process and data
X move process graph to converter shared
X make process graph produce a flat list to process rather than process recursively
X make null process like null data
X How do we put user data in the SceneDesc node?
X Do we write one scene file or many?
