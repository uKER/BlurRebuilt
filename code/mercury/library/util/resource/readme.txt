Resource System
===============

see documentation in resource.h


Task List:
----------

- Make C_HandleBase load take a resource ID and include factory functionality in the TypeRegistry
- Stats / perf code. Identify disk thrashing etc.
- Identify what resources are loaded in particular secitons of code.
- Include DataOld flag check in C_ResourceItem::operator <
- Some doxygen comments
- Handle reloading of resources - this will now generally be done via actor code callbacks etc. OldData() method added to help this


Done:
----------
X store resource data state as enum with error conditions, file not found, data corrupted etc. states
X Make add listener send notify immediately if data already loaded.
X Handle load/reload callbacks somehow
X Move task registry to own file
X Investigate derived vs templated further
X Make C_LoadingThread members thread safe
