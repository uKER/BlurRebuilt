#ifndef X_TO_MSH_H
#define X_TO_MSH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "X.h"
#include "MSH.h"

void XtoMSH(const Util::C_String& srcFileDir, const Util::C_String& srcFileName, C_XMesh* pSrcMesh, C_Mesh* pDstMesh);

#endif // X_TO_MSH_H
