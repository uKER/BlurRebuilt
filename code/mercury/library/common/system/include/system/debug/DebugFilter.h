#ifndef DEBUGFILTER_H
#define DEBUGFILTER_H

#ifndef CFG_SPEED
// Comment this out to turn off all the debug filters.
#define DEBUG_FILTERS_ON
#endif

//-----------------------------------------------------------------------------------


/* -- DEBUG FILTER SYSTEM --

- Description:

A set of macros that allow you to define blocks of code that can be turned on/off using debug variables.


- Example of usage:

Put this at the top of your source file:

	DEFINE_DEBUG_FILTER( g_FiltersExample, "Filters.Example" )

You can also use

	DECLARE_DEBUG_FILTER( g_FiltersExample )

in your header file.

Now, put the following round some blocks of code:

	DEBUG_FILTER_START( g_FiltersExample )

	... // Code goes here

	DEBUG_FILTER_END

The filtered code will run by default, but you can turn it off from DebugView.

NOTE - comment out the #define at the top of this file to turn off the filters - i.e. leave all filtered code
permanently on.

*/


#ifdef DEBUG_FILTERS_ON

#define DECLARE_DEBUG_FILTER( name )			extern Debug::Bool name;
#define DEFINE_DEBUG_FILTER( name, location )	Debug::Bool name( location, true );
#define DEBUG_FILTER_START( name )				if ( name ) {
#define DEBUG_FILTER_END						}

#else

#define DECLARE_DEBUG_FILTER( name )
#define DEFINE_DEBUG_FILTER( name, location )
#define DEBUG_FILTER_START( name )
#define DEBUG_FILTER_END

#endif


//-----------------------------------------------------------------------------------

#endif
