#ifndef HANDLEDECLARE_H
#define HANDLEDECLARE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Util
{

class C_HandleBase;
class C_HandleItemBase;
class C_HandleManagerBase;

template <class T_ItemType> class C_Handle;
template <class T_ItemType> class C_HandleItem;
template <class T_ItemType> class C_HandleManager;

template <class T_ItemType> struct I_HandleVisitor;

} // namespace Util

#endif // HANDLEDECLARE_H
