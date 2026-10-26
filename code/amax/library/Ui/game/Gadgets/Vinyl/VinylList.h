#ifndef UI_VINYLLIST_H
#define UI_VINYLLIST_H

#include <Xt/XtGame.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

namespace UI
{

//
// VinylItem
//

const int kVinylFlag_Allocated	= 0x0001;
const int kVinylFlag_Selected	= 0x0002;
const int kVinylFlag_Gradient	= 0x0004;

struct VinylItem
{
	XtU16 flags;
	XtI16 self;

	XtI16 next;
	XtI16 prev;
	XtI16 parent;
	XtI16 child;

private:
	XtU16 shape;
	XtU16 colour0;
	XtU16 colour1;
	float alpha0;
	float alpha1;

	float x;
	float y;
	float rotation;
	float col_rot;
	float col_scale;
	float col_pos;
	float scale_x;
	float scale_y;
	float shear_x;
	float shear_y;

public:
	VinylItem(XtI16 self_ = -1) :
		flags(0),
		self(self_),
		next(-1),
		prev(-1),
		parent(-1),
		child(-1),
		shape(0xffff),
		colour0(0xffff),
		colour1(0xffff),
		x(0.0f),
		y(0.0f),
		alpha0(1.0f),
		alpha1(1.0f),
		rotation(0.0f),
		col_rot(0.0f),
		col_scale(1.0f),
		col_pos(0.0f),
		scale_x(1.0f),
		scale_y(1.0f),
		shear_x(0.0f),
		shear_y(0.0f)
	{
	}

	void Reset()
	{
		shape = 0xffff;
		colour0 = 0xffff;
		colour1 = 0xffff;
		x = 0.0f;
		y = 0.0f;
		rotation = 0.0f;
		col_rot = 0.0f;
		col_scale = 1.0f;
		col_pos = 0.0f;
		scale_x = 1.0f;
		scale_y = 1.0f;
		alpha0 = 1.0f;
		alpha1 = 1.0f;
		shear_x = 0.0f;
		shear_y = 0.0f;
	}

	void Copy( const VinylItem& rhs )
	{
		shape = rhs.shape;
		colour0 = rhs.colour0;
		colour1 = rhs.colour1;
		x = rhs.x;
		y = rhs.y;
		rotation = rhs.rotation;
		col_rot = rhs.col_rot;
		col_scale = rhs.col_rot;
		col_pos = rhs.col_pos;
		scale_x = rhs.scale_x;
		scale_y = rhs.scale_y;
		alpha0 = rhs.alpha0;
		alpha1 = rhs.alpha1;
		shear_x = rhs.shear_x;
		shear_y = rhs.shear_y;

		// Sigh
		if( rhs.Gradient() )
			Gradient( true );
	}

	bool			Highlight() const;
	bool			Gradient() const;
	U16				Colour0() const;
	U16				Colour1() const;
	U16				Shape() const;
	F32				PosX() const;
	F32				PosY() const;
	F32				ScaleX() const;
	F32				ScaleY() const;
	F32				Alpha0() const;
	F32				Alpha1() const;
	F32				ShearX() const;
	F32				ShearY() const;
	F32				Rotation() const;
	F32				ColourRotation() const;
	F32				ColourScale() const;
	F32				ColourPos() const;

	void			Highlight( bool set );
	void			Gradient( bool set );
	void			Colour0( const U16 index );
	void			Colour1( const U16 index );
	void			Shape( U16 index );
	void			PosX( F32 x );
	void			PosY( F32 y );
	void			ScaleX( F32 sx );
	void			ScaleY( F32 sy );
	void			Alpha0( F32 a );
	void			Alpha1( F32 a );
	void			ShearX( F32 s );
	void			ShearY( F32 s );
	void			Rotation( F32 r );
	void			ColourRotation( F32 r );
	void			ColourScale( F32 s );
	void			ColourPos( F32 a );

	void			GetTransform( Maths::HMat43& mat );
	void			GetColourTransform( Maths::HMat43& mat );

	void			IncTranslation( const bool moveX, const F32 amount, const bool nudge  );
	void			IncScale( const bool scaleX, const F32 amount, const bool nudge );
	void			IncShear( const bool shearX, const F32 amount, const bool nudge  );
	void			IncRotation( const F32 amount, const bool nudge );
	void			IncColourRotation( const F32 amount, const bool nudge );
	void			IncColourTranslation( const F32 amount, const bool nudge );
	void			IncColourScale( const F32 amount, const bool nudge );
	void			IncAlpha( const U32 index, const F32 alphaInc, const bool nudge );
};

//
// VinylList
//

class VinylList
{
public:
	VinylList();
	~VinylList();

public:
	bool Create(int maxCount);
	void Destroy();

public:
	void Clear();
	int InsertBefore(int currIndex);
	int InsertAfter(int currIndex);
	int InsertFirstChildOf(int currIndex);
	int InsertLastChildOf(int currIndex);
	void Remove(int currIndex);

	bool Load( Sys::BinaryDeserialiserAutoEndian& serial );
	bool Save( Sys::BinarySerialiserAutoEndian& serial );

public:
	int UsedCount();
	int FreeCount();

public:
	VinylItem*	At(int index);
	int			Root();
	int			GetFirstSibling(int index);
	int			GetLastSibling(int index);
	int			GetNext(int index);
	int			GetPrev(int index);
	int			GetParent(int index);
	int			GetFirstChild(int index);
	int			GetLastChild(int index);

private:
	int _New();
	void _Free(int index);

	void _Detach(int index);

public:
	void _AttachBefore(int index, int before);
	void _AttachAfter(int index, int after);

private:
	int			_maxCount;
	int			_usedCount;
	VinylItem*	_items;

private:
	int _root;
	int _free;
};

class VinylIterator
{
public:
	VinylIterator(VinylList* list, int startIndex, int currIndex, int endIndex) :
		_list(list), _start(startIndex), _curr(currIndex), _end(endIndex)
	{ }		

public:
	operator bool()	const	{ return _curr >= 0;	}
	operator int() const	{ return _curr;			}

public:
	void operator++() { Next(); }
	void operator--() { Prev(); }

public:
	void Next();
	void Prev();

public:
	int Index() const { return _curr; }

private:
	VinylList*	_list;
	int			_start;
	int			_curr;
	int			_end;
};

VinylIterator GetChildIterator(VinylList* list, int currIndex);
VinylIterator GetSiblingIterator(VinylList* list, int currIndex);
VinylIterator GetFirstSiblingIterator(VinylList* list, int currIndex);
VinylIterator GetLastSiblingIterator(VinylList* list, int currIndex);

int CountItems(VinylList* list, int currIndex);
void CopySub(VinylList* dstList, int dstRoot, VinylList* srcList, int srcRoot);

}; // namespace UI

#endif // UI_VINYLLIST_H
