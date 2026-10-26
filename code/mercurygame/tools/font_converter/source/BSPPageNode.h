
#pragma once

#include "Font.h"

namespace Font
{

class C_BSPPageNode
{
public:

	C_BSPPageNode();
    ~C_BSPPageNode();
    
    int Insert(struct Rect *result, struct Rect *rectSize);

public:

    Rect				m_Rect;
    C_BSPPageNode		*m_Child[2];	///< Child nodes (if any) which split this page into two
    bool				m_Full;
};


}; // namespace FontLib