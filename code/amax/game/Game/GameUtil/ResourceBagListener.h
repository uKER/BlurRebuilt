#ifndef RESOURCEBAGLISTENER_E9E27ADD_7DCF_44AF_A7FF_694812E35A91_H
#define RESOURCEBAGLISTENER_E9E27ADD_7DCF_44AF_A7FF_694812E35A91_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/list.h>

#include "Game/GameUtil/ResourceBagDeclare.h"


/**
Derived classes should implement OnResourceBagLoaded to be notified when a
bag has no more resources waiting to load.
*/
class C_ResourceBagListener
{
	friend class C_ResourceBag;

public:
	virtual ~C_ResourceBagListener();

protected:
	C_ResourceBagListener();

private:
	typedef Btl::List<C_ResourceBag*> C_Listenees; ///< Set of resource bags.

	C_Listenees m_Listenees; ///< the resource bags we're listening to

	C_ResourceBagListener(const C_ResourceBagListener&); // can not be copied
	C_ResourceBagListener& operator = (const C_ResourceBagListener&); // can not be copied

	/**
	@brief Called when the resource bag is no longer waiting for resources to load.
	When all bagged resources have either loaded or encountered errors, and the bag
	has been unlocked, this notification is sent to all listeners.
	@param bag bag the has loaded resources.
	*/
	virtual void OnResourceBagLoaded(const C_ResourceBag& bag) = 0;

};


#endif // RESOURCEBAGLISTENER_E9E27ADD_7DCF_44AF_A7FF_694812E35A91_H
