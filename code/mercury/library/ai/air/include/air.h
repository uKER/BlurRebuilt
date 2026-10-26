#ifndef _AIR_H
#define _AIR_H

// component system
#include <ComponentSystem/CObjectManager.h>
#include <ComponentSystem/SObjectManagerDB.h>
#include <ComponentSystem/ComponentIterators.h>
#include <ComponentSystem/CMessage.h>
#include <ComponentSystem/cparamsnode.h>
#include <ComponentSystem/Globals.h>
#include <ComponentSystem/CObjectManager.h>
#include <ComponentSystem/IComponent.h>
#include <ComponentSystem/CObjectId.h>
#include <ComponentSystem/CHash.h>

// fuzzy libraries
#include <fuzzylib/fuzzylibone/fuzzyone.h>
#include <fuzzylib/fuzzylibtwo/fuzzyoperators.h>
#include <fuzzylib/fuzzylibtwo/fuzzypolicy.h>
#include <fuzzylib/fuzzylibtwo/fuzzyrule.h>
#include <fuzzylib/fuzzylibtwo/fuzzyset.h>
#include <fuzzylib/fuzzylibtwo/fuzzyterm.h>
#include <fuzzylib/fuzzylibtwo/fuzzyvariable.h>
#include <fuzzylib/fuzzylibtwo/fzset.h>

// utility functions
#include <utilityai/kdtree.h>
#include <utilityai/pid.h>
#include <utilityai/utilityai.h>

// ai components.....
#include <pidcontrollerai/cpidcontrollerai.h>
#include <system/csystemai.h>
#include <data/cdata.h>
#include <driverai/cdriverai.h>
#include <gameservertime/cgameservertime.h>
#include <paramsai/cparamsai.h>
#include <speedtracker/cspeedtracker.h>
#include <splinefollower/csplinefollower.h>
#include <splinetracker/csplinetracker.h>
#include <trackdata/ctrackdata.h>
#include <trackdata/trackdata.h>
#include <vehiclelogic/cvehiclelogic.h>
#include <vehiclestate/cvehiclestate.h>
#include <avoidance/cavoidance.h>
#include <avoidance/cavoidanceBasic.h>
#include <accelbrakecontroller/caccelbrakecontroller.h>
#include <controlsstate/ccontrolstate.h>
#include <racedirector/cracedirector.h>
#include <debugTools/cdebugai.h>
#include <recovery/crecoveryai.h>
#include <rubberbanding/crubberbandingai.h>
#include <introoutrosplineai/cintrooutrosplineai.h>
#include <spline/cspline.h>
#include <messages/AISideSwipe.h>
#include <driverai/igamebrain.h>



//#define DEBUG_AI


//#define WHOLE_AI_MULTI_THREADED
#define PARTS_OF_AI_MULTI_THREADED
#define PERF_REMOVE_POST_UPDATE_AS_BLUR_DOES_NOT_HAVE_IMPORTANT_CODE_HERE

#endif // _AIR_H
