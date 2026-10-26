
#ifndef UI_UITESTLIST_H
#define UI_UITESTLIST_H

#include "UiManager.h"
#include "Ui/Game/Gadgets/BaseUIList.h"

namespace UI
{

class UITestList : public BaseUIList
{
public:
	DECLARE_GADGET( E_ButtonType_TestList, "TEST_LIST" );

	void	Setup( lua_State* pState );

	void	RenderItem( F32 x, F32 y, const U32 item  );

	S32		GetNumberOfItems() const
			{
				return 10;
			}
};

}; // namespace UI

#endif // UI_UITESTLIST_H