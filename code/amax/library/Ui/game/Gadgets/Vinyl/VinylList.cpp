#include "stdafx.h"

#include "VinylList.h"

namespace UI
{

VinylList::VinylList() :
	_maxCount(0),
	_usedCount(0),
	_items(0),
	_root(-1),
	_free(-1)
{
}

VinylList::~VinylList()
{
	delete [] _items;
}

bool VinylList::Create(int maxCount)
{
	Destroy();

	if (maxCount < 2)
		return false;

	if ((_items = new VinylItem[maxCount]) == 0)
		return false;

	_maxCount = maxCount;

	Clear();

	return true;
}

void VinylList::Destroy()
{
	delete [] _items;

	_maxCount	= 0;
	_usedCount	= 0;
	_items		= 0;
	_root		= -1;
	_free		= -1;
}

void VinylList::Clear()
{
	if (!_items)
		return;

	_usedCount	= 1;
	_root		= 0;
	_free		= 1;

	for(int i = 0; i < _maxCount; i++)
		_items[i] = VinylItem(i);

	_items[_root].flags	= kVinylFlag_Allocated;
	_items[_root].next	= _root;
	_items[_root].prev	= _root;

	for(int i = 1; i < _maxCount; i++)
		_items[i].next = i + 1;

	_items[_maxCount - 1].next = -1;
}

int VinylList::InsertBefore(int currIndex)
{
	if (!_items || (currIndex < 0) || (currIndex >= _maxCount))
		return -1;

	VinylItem* curr = &_items[currIndex];

	if ((curr->flags & kVinylFlag_Allocated) == 0)
		return -1;

	if (curr->parent < 0)
		return -1;

	int newIndex = _New();

	if (newIndex < 0)
		return -1;

	_AttachBefore(newIndex, currIndex);

	_items[newIndex].parent = curr->parent;

	VinylItem* parent = &_items[curr->parent];

	if (parent->child == currIndex)
		parent->child = newIndex;

	return newIndex;
}

int VinylList::InsertAfter(int currIndex)
{
	if (!_items || (currIndex < 0) || (currIndex >= _maxCount))
		return -1;

	VinylItem* curr = &_items[currIndex];

	if ((curr->flags & kVinylFlag_Allocated) == 0)
		return -1;

	if (curr->parent < 0)
		return -1;

	int newIndex = _New();

	if (newIndex < 0)
		return -1;

	_AttachAfter(newIndex, currIndex);

	_items[newIndex].parent = curr->parent;

	return newIndex;
}

int VinylList::InsertFirstChildOf(int currIndex)
{
	if (!_items || (currIndex < 0) || (currIndex >= _maxCount))
		return -1;

	VinylItem* curr = &_items[currIndex];

	if ((curr->flags & kVinylFlag_Allocated) == 0)
		return -1;

	if (curr->child >= 0)
		return InsertBefore(curr->child);

	int newIndex = _New();

	if (newIndex < 0)
		return -1;

	curr->child				= newIndex;
	_items[newIndex].parent	= currIndex;

	return newIndex;
}

int VinylList::InsertLastChildOf(int currIndex)
{
	if (!_items || (currIndex < 0) || (currIndex >= _maxCount))
		return -1;

	VinylItem* curr = &_items[currIndex];

	if ((curr->flags & kVinylFlag_Allocated) == 0)
		return -1;

	if (curr->child >= 0)
		return InsertAfter(_items[curr->child].prev);

	int newIndex = _New();

	if (newIndex < 0)
		return -1;

	curr->child				= newIndex;
	_items[newIndex].parent	= currIndex;

	return newIndex;
}

void VinylList::Remove(int currIndex)
{
	if (!_items || (currIndex < 0) || (currIndex >= _maxCount))
		return;

	VinylItem* curr = &_items[currIndex];

	if ((curr->flags & kVinylFlag_Allocated) == 0)
		return;

	if (curr->parent < 0)
		return;

	while(curr->child >= 0)
		Remove(curr->child);

	VinylItem* parent = &_items[curr->parent];

	if (parent->child == currIndex)
	{
		if (curr->next == currIndex)
			parent->child = -1;
		else
			parent->child = curr->next;
	}

	_Detach(currIndex);
	_Free(currIndex);
}

bool VinylList::Load( Sys::BinaryDeserialiserAutoEndian& serial )
{
	if (!_items)
		return false;

	S32 maxCount = 0;
	serial >> maxCount;

	if( maxCount != _maxCount )
		return false;

	serial >> _usedCount;
	serial >> _root;
	serial >> _free;

	serial.Read( _items, maxCount * sizeof( VinylItem ) ); 

	return true;
}

bool  VinylList::Save( Sys::BinarySerialiserAutoEndian& serial )
{
	if (!_items)
		return false;

	serial << _maxCount;
	serial << _usedCount;
	serial << _root;
	serial << _free;

	serial.Write( _items, _maxCount * sizeof( VinylItem ) );

	return true;
}

int VinylList::UsedCount()
{
	return _usedCount;
}

int VinylList::FreeCount()
{
	return _maxCount - _usedCount;
}

VinylItem* VinylList::At(int index)
{
	if (!_items || (index < 0) || (index >= _maxCount))
		return 0;

	VinylItem* item = &_items[index];

	if ((item->flags & kVinylFlag_Allocated) == 0)
		return 0;

	return item;
}

int VinylList::Root()
{
	return _root;
}

int VinylList::GetFirstSibling(int index)
{
	VinylItem* item = At(index);

	if (!item)
		return index;

	if (item->parent < 0)
		return index;

	return _items[item->parent].child;
}

int VinylList::GetLastSibling(int index)
{
	VinylItem* item = At(index);

	if (!item)
		return index;

	if (item->parent < 0)
		return index;

	return _items[_items[item->parent].child].prev;
}

int VinylList::GetNext(int index)
{
	if (VinylItem* item = At(index))
		return item->next;

	return -1;
}

int VinylList::GetPrev(int index)
{
	if (VinylItem* item = At(index))
		return item->prev;

	return -1;
}

int VinylList::GetParent(int index)
{
	if (VinylItem* item = At(index))
		return item->parent;

	return -1;
}

int	VinylList::GetFirstChild(int index)
{
	if (VinylItem* item = At(index))
		return item->child;

	return -1;
}

int	VinylList::GetLastChild(int index)
{
	VinylItem* item = At(index);

	if (!item)
		return -1;

	if (item->child < 0)
		return -1;

	return _items[item->child].prev;
}

int VinylList::_New()
{
	if (_free < 0)
		return -1;

	int index = _free;

	VinylItem* item = &_items[index];

	_free = item->next;

	item->flags		= kVinylFlag_Allocated;
	item->next		= index;
	item->prev		= index;
	item->parent	= -1;
	item->child		= -1;

	_usedCount++;

	return index;
}

void VinylList::_Free(int index)
{
	VinylItem* item = &_items[index];

	item->flags		= 0;
	item->next		= _free;
	item->prev		= -1;
	item->parent	= -1;
	item->child		= -1;

	_free = index;

	_usedCount--;
}

void VinylList::_Detach(int index)
{
	VinylItem* item = &_items[index];

	_items[item->next].prev = item->prev;
	_items[item->prev].next = item->next;

	item->next = -1;
	item->prev = -1;
}

void VinylList::_AttachBefore(int index, int beforeIndex)
{
	VinylItem* item		= &_items[index];
	VinylItem* before	= &_items[beforeIndex];

	item->next = beforeIndex;
	item->prev = before->prev;

	_items[before->prev].next	= index;
	before->prev				= index;
}

void VinylList::_AttachAfter(int index, int afterIndex)
{
	VinylItem* item		= &_items[index];
	VinylItem* after	= &_items[afterIndex];

	item->next = after->next;
	item->prev = afterIndex;

	_items[after->next].prev	= index;
	after->next					= index;
}

VinylIterator GetChildIterator(VinylList* list, int currIndex)
{
	int firstIndex = list->GetFirstChild(currIndex);

	return VinylIterator(list, firstIndex, firstIndex, list->GetLastChild(currIndex));
}

void VinylIterator::Next()
{
	if (_curr < 0)
		_curr = _start;
	else if (_curr != _end)
		_curr = _list->GetNext(_curr);
	else
		_curr = -1;
}

void VinylIterator::Prev()
{
	if (_curr < 0)
		_curr = _end;
	else if (_curr != _start)
		_curr = _list->GetPrev(_curr);
	else
		_curr = -1;
}

VinylIterator GetSiblingIterator(VinylList* list, int currIndex)
{
	return VinylIterator(list, list->GetFirstSibling(currIndex), currIndex, list->GetLastSibling(currIndex));
}

VinylIterator GetFirstSiblingIterator(VinylList* list, int currIndex)
{
	int index = list->GetFirstSibling(currIndex);
	return VinylIterator(list, index, index, list->GetLastSibling(currIndex));
}

VinylIterator GetLastSiblingIterator(VinylList* list, int currIndex)
{
	int index = list->GetLastSibling(currIndex);
	return VinylIterator(list, list->GetFirstSibling(currIndex), index, index);
}

int CountItems(VinylList* list, int currIndex)
{
	if (!list)
		return 0;

	VinylItem* item = list->At(currIndex);

	if (!item)
		return 0;

	int total = 1;
	for(VinylIterator it( GetChildIterator(list, currIndex) ); it; ++it)
		total += CountItems(list, it);

	return total;
}

void VinylItem::GetTransform( Maths::HMat43& mat )
{
	Maths::HMat33 r( Maths::HMat33( Maths::C_MatrixRotatePHR( 0.0f, 0.0f, Maths::DegToRad( rotation ) ) ) );
	Maths::HMat33 s( Maths::HVec3( ScaleX(),  ShearY() * ScaleY(), 0.0f ), 
				 	 Maths::HVec3( ScaleX() * ShearX(),	 ScaleY(), 0.0f ), 
					 Maths::HVec3( 0.0f, 0.0f, 1.0f ) );

	mat.Rotation( s * r );
	mat.Translation( Maths::HVec3( x, y, 0.0f ) );
}

void VinylItem::GetColourTransform( Maths::HMat43& mat )
{
	F32 ssqr = ColourScale();
	ssqr *= ssqr;

	Maths::HMat43 o( Maths::HMat33(), Maths::HVec3( -0.5f, -0.5f, 0.0f ) );
	Maths::HMat43 m( Maths::C_MatrixRotatePHR( 0.0f, 0.0f, Maths::DegToRad( ColourRotation() ) ), Maths::HVec3( ColourPos() + 0.5f, ColourPos() + 0.5f, 0.0f ) );
	Maths::HMat43 s( Maths::C_MatrixScale( ssqr, ssqr, 1.0f ), Maths::HVec3() );

	mat = (o * m) * s;
}

void VinylItem::IncScale( const bool moveX, const F32 amount, const bool nudge )
{
	const F32 scaleClamp = 10.0f;
	const F32 nudgeScale = abs( amount );

	if( moveX )
	{
		scale_x += amount;

		if( nudge && amount )
			scale_x = floorf( (scale_x / nudgeScale) + 0.5f ) * nudgeScale;

		scale_x = Maths::Clamp<F32>( scale_x, -scaleClamp, scaleClamp );
	}
	else
	{
		scale_y += amount;

		if( nudge && amount )
			scale_y = floorf( (scale_y / nudgeScale) + 0.5f ) * nudgeScale;
			
		scale_y = Maths::Clamp<F32>( scale_y, -scaleClamp, scaleClamp );
	}

}

void VinylItem::IncTranslation( const bool moveX, const F32 amount, const bool nudge )
{
	const F32 nudgeScale = fabs( amount );
	const F32 transClamp = 1000.0f;

	if( moveX )
	{
		x += amount;

		if( nudge && amount )
			x = floorf( (x / nudgeScale) + 0.5f ) * nudgeScale;

		x = Maths::Clamp( x, -transClamp, transClamp );
	}
	else
	{
		y += amount;

		if( nudge && amount )
			y = floorf( (y / nudgeScale) + 0.5f ) * nudgeScale;

		y = Maths::Clamp( y, -transClamp, transClamp );
	}
}


void VinylItem::IncShear( const bool shearX, const F32 amount, const bool nudge )
{
	const F32 nudgeScale = fabs( amount );
	const F32 shearClamp = 4.0f;
	if( shearX )
	{
		shear_x += amount;

		if( nudge && amount )
			shear_x = floorf( (shear_x / nudgeScale) + 0.5f ) * nudgeScale;

		shear_x = Maths::Clamp<F32>( shear_x, -shearClamp, shearClamp );
	}
	else
	{
		shear_y += amount;

		if( nudge && amount )
			shear_y = floorf( (shear_y / nudgeScale) + 0.5f ) * nudgeScale;

		shear_y = Maths::Clamp<F32>( shear_y, -shearClamp, shearClamp );
	}
}

void VinylItem::IncRotation( const F32 amount, const bool nudge )
{
	rotation += amount;

	if( nudge && amount )
	{
		const F32 nudgeScale = fabs( amount );

		rotation = floorf( (rotation / nudgeScale) + 0.5f ) * nudgeScale;
	}

	if( rotation >= 360.0f )
		rotation -= 360.0f;
	else
	if( rotation < 0.0f )
		rotation += 360.0f;
}

void VinylItem::IncAlpha( const U32 index, const F32 amount, const bool nudge )
{
	F32& alpha = (index == 0) ? alpha0 : alpha1;

	alpha += amount;

	if( nudge && amount )
	{
		const F32 nudgeScale = fabs( amount );

		alpha = floorf( (alpha / nudgeScale) + 0.5f ) * nudgeScale;
	}

	alpha = Maths::Clamp<F32>( alpha, 0, 1.0f );
}

void VinylItem::IncColourRotation( const F32 amount, const bool nudge )
{
	col_rot += amount;

	if( nudge && amount )
	{
		const F32 nudgeScale = fabs( amount );

		col_rot = floorf( (col_rot / nudgeScale) + 0.5f ) * nudgeScale;
	}

	if( col_rot >= 360.0f )
		col_rot -= 360.0f;
	else
	if( col_rot < 0.0f )
		col_rot += 360.0f;
}

void VinylItem::IncColourTranslation( const F32 amount, const bool nudge )
{
	const F32 nudgeScale = fabs( amount );

	col_pos += amount;

	if( nudge && amount )
		col_pos = floorf( (col_pos / nudgeScale) + 0.5f ) * nudgeScale;

	col_pos = Maths::Clamp<F32>( col_pos, -1.0f, 1.0f );
}

void VinylItem::IncColourScale( const F32 amount, const bool nudge )
{
	col_scale += amount;

	if( nudge && amount )
	{
		const F32 nudgeScale = fabs( amount );

		if( nudge && amount )
			col_scale = floorf( (col_scale / nudgeScale) + 0.5f ) * nudgeScale;

	}

	col_scale = Maths::Clamp<F32>( col_scale, 0, 5.0f );
}

// Accessor
bool VinylItem::Highlight() const
{
	return (flags & kVinylFlag_Selected) != 0; 
}

bool VinylItem::Gradient() const
{
	return (flags & kVinylFlag_Gradient ) != 0;
}

U16 VinylItem::Shape() const
{
	return shape;
}

U16 VinylItem::Colour0() const
{
	return colour0;
}

U16 VinylItem::Colour1() const
{
	return colour1;
}


F32 VinylItem::PosX() const
{
	return x;
}

F32 VinylItem::PosY() const
{
	return y;
}

F32 VinylItem::ScaleX() const
{
	return scale_x;
}

F32 VinylItem::ScaleY() const
{
	return scale_y;
}

F32 VinylItem::Alpha0() const
{
	return alpha0;
}

F32 VinylItem::Alpha1() const
{
	return alpha1;
}

F32 VinylItem::ShearX() const
{
	return shear_x;
}

F32 VinylItem::ShearY() const
{
	return shear_y;
}

F32 VinylItem::Rotation() const
{
	return rotation;
}

F32 VinylItem::ColourRotation() const
{
	return col_rot;
}

F32 VinylItem::ColourScale() const
{
	return col_scale;
}

F32 VinylItem::ColourPos() const
{
	return col_pos;
}

void VinylItem::Highlight( bool set )
{
	flags = ( set ) ? (flags | kVinylFlag_Selected) : (flags & ~kVinylFlag_Selected);
}

void VinylItem::Gradient( bool set )
{
	flags = ( set ) ? (flags | kVinylFlag_Gradient) : (flags & ~kVinylFlag_Gradient);
}

void VinylItem::Colour0( U16 c )
{
	colour0 = c;
}

void VinylItem::Colour1( U16 c )
{
	colour1 = c;
}

void VinylItem::Shape( U16 index )
{
	shape = index;
}

void VinylItem::PosX( F32 px )
{
	x = px;
}

void VinylItem::PosY( F32 py )
{
	y = py;
}

void VinylItem::ScaleX( F32 sx )
{
	scale_x = sx;
}

void VinylItem::ScaleY( F32 sy )
{
	scale_y = sy;
}

void VinylItem::Alpha0( F32 a )
{
	alpha0 = a;
}

void VinylItem::Alpha1( F32 a )
{
	alpha1 = a;
}

void VinylItem::ShearX( F32 s )
{
	shear_x = s;
}

void VinylItem::ShearY( F32 s )
{
	shear_y = s;
}

void VinylItem::Rotation( F32 r )
{
	rotation = r;
}

void VinylItem::ColourRotation( F32 r )
{
	col_rot = r;
}

void VinylItem::ColourScale( F32 s )
{
	col_scale = s;
}

void VinylItem::ColourPos( F32 p )
{
	col_pos = p;
}

void CopySub(UI::VinylList* dstList, int dstRoot, UI::VinylList* srcList, int srcRoot)
{
	VinylItem* pDst = dstList->At( dstRoot );
	VinylItem* pSrc = srcList->At( srcRoot );

	if (!pDst || !pSrc)
		return;

	pDst->Copy( *pSrc );

	for(VinylIterator src_it( GetChildIterator( srcList, srcRoot ) ); src_it; ++src_it )
		CopySub(dstList, dstList->InsertLastChildOf( dstRoot ), srcList, src_it.Index());
}

} // namespace UI

