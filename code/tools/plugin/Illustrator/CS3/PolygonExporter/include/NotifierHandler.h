
#ifndef NOTIFIER_HANDLER_H
#define NOTIFIER_HANDLER_H

#pragma pack(push, 4)
#include "AINotifier.h"
#include "AIRuntime.h"
#pragma pack(pop)

extern AIErr addNotifiers( SPInterfaceMessage *message );

extern AIErr goNotifier( AINotifierMessage *message );

#endif // NOTIFIER_HANDLER_H