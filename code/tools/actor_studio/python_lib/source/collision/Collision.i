%module collision

%{
#include        "stdafx.h"
%}

%include "..\backend_lib\include\editorlib\collision\CollisionStructs.h"

%include Ray.i
%include CollisionManager.i
%include CollisionScene.i
%include CollisionResult.i
%include CollisionHelpers.i

