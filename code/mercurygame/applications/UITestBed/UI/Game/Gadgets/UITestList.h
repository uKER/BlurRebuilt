
#ifndef UI_TEST_LIST_H
#define UI_TEST_LIST_H

#include "UIManager.h"
#include <Ui/Game/Gadgets/BaseUIList.h>

namespace UI
{

class UITestList : public BaseUIList
{
public:
	DECLARE_GADGET( E_ButtonType_TestList, "TEST_LIST" )

	void		 				Init();

								/// Returns the number of items
	virtual S32					GetNumberOfItems() const
								{
									return m_NumberOfItems;
								}

								/// Function defining how each item should be rendered.
	virtual void 				RenderItem( const F32 fX, const F32 fY, const U32 uItem, const Maths::HVec3& vRelativePos );

private:
	S32							m_NumberOfItems;
};

}; // namespace UI

#endif // UI_TEST_LIST_H