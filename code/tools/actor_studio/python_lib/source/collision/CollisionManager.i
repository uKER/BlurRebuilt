%{
#include <sigslot/sigslot.h>
#include <EditorLib/Collision/Ray.h>
#include <EditorLib/Collision/CollisionStructs.h>
#include <EditorLib/Collision/CollisionResult.h>
#include <EditorLib/Collision/CollisionManager.h>
#include <EditorLib/Collision/CollisionScene.h>
#include <EditorLib/Collision/CollisionHelpers.h>
using namespace EditorLib;
%}

%ignore RegisterCollidableHandler(Ut::Delegate<bool(const I_DgNode*)> pHandler);
%ignore UnregisterCollidableHandler(Ut::Delegate<bool(const I_DgNode*)> pHandler);

%include "..\backend_lib\include\editorlib\collision\CollisionManager.h"
