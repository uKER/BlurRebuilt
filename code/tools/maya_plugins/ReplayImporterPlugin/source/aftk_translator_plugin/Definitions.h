#ifndef AFTK_MAYA_DEFINITIONS_H
#define AFTK_MAYA_DEFINITIONS_H

#define CHECK_ERROR_CLEAN_UP_RETURN if (!mStatus) { assert(false); CleanUpAfterFailure(); return mStatus; }
#define CHECK_ERROR_RETURN if (!mStatus) { assert(false); return mStatus;}

#endif
