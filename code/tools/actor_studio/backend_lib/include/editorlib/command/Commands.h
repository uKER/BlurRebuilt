#pragma once

#ifndef EDITOR_LIB_COMMANDS_H
#define EDITOR_LIB_COMMANDS_H

#include "ICommand.h"
#include "Dg_Cmd/CreateXtNodeCmd.h"
#include "Dg_Cmd/CreateAnimationCmd.h"
#include "Dg_Cmd/CreateGroupCmd.h"
#include "Dg_Cmd/XFormCmd.h"
#include "Dg_Cmd/ParentCmd.h"
#include "Dg_Cmd/CopyCmd.h"
#include "Dg_Cmd/DeleteCmd.h"
#include "Dg_Cmd/PasteCmd.h"
#include "Dg_Cmd/DuplicateCmd.h"
#include "Dg_Cmd/ExportCmd.h"
#include "Dg_Cmd/SelectCmd.h"
#include "Dg_Cmd/RenameCmd.h"
#include "Ext_Actors_Cmd/ExtActorsRefreshCmd.h"
#include "Ext_Actors_Cmd/ExtActorsOfflineRefreshCmd.h"
#include "Io_Cmd/FileCmd.h"
#include "Misc_Cmd/QuitCmd.h"
#include "Misc_Cmd/UndoCmd.h"
#include "Misc_Cmd/RedoCmd.h"
#include "Misc_Cmd/StoreChangesCmd.h"
#include "Misc_Cmd/GetLatestReferencesCmd.h"
#include "Xt_Cmd/XtDbCmd.h"
#include "Xt_Cmd/BuildLevelPackCmd.h"

#endif
