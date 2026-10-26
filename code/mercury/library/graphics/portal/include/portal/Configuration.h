#ifndef PORTAL_CONFIGURATION_H
#define PORTAL_CONFIGURATION_H

#ifdef _MSC_VER
#pragma once
#endif

#define			PORTAL_CONFIG_MAXAFFECTEDPORTALS				32		// Maximum number of portals that can be affected by a blocker.
#define			PORTAL_CONFIG_MAXBLOCKERS						128		// Maximum number of blockers that the manager can contain.
#define			PORTAL_CONFIG_MAXPOLYVERTS						32		// Maximum number of polys in a portal polygon.
#define			PORTAL_CONFIG_MAXPORTALSYSTEMS					8		// Maximum number of portal systems that can exist at any one time.
#define			PORTAL_CONFIG_NO_REFLECTION_OPTIMISATIONS		0		// Set to 1 to enable extra optimisations that rely on there being no reflections.
#define			PORTAL_DEBUG_CHECK_ROOM_INDEX_HACK				0		// Set to 1 to perform checking against the room index hack.
#define			PORTAL_DEBUG_DONT_USE_GRID						1		// Set to 1 to search list of rooms linearly rather than using room grid.
#define			PORTAL_DEBUG_LOAD_DEBUG_DATA					1		// Set to 1 to load data from level render file that aids in visualisation.

#endif // PORTAL_CONFIGURATION_H
