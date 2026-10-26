//***************************************************************************
// 1998 Microsoft Systems Journal. 
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual C++ 5.0 on Windows 95
//***************************************************************************
#if !defined(_MOUSE_HDR_INCLUDED_)
#define _MOUSE_HDR_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//***************************************************************************
// class to represent global mouse object.
//***************************************************************************
class CMouse 
{
public:

	CWnd*	 Capture();			
	CWnd*	 Capture(CWnd *w);
	void	 Release();
	CPoint	 GetPos();
	operator CPoint();
};

//***************************************************************************
extern CMouse Mouse;

//***************************************************************************
//***************************************************************************
inline CWnd* CMouse::Capture( void )
{ 
	return CWnd::GetCapture(); 
}

//***************************************************************************
inline CWnd* CMouse::Capture( CWnd *w )	
{ 
	return w->SetCapture(); 
}

//***************************************************************************
inline void CMouse::Release( void )
{ 
	ReleaseCapture(); 
}

//***************************************************************************
inline CPoint CMouse::GetPos( void )
{ 
	CPoint p; 
	::GetCursorPos(&p); 
	return p; 
}

//***************************************************************************
inline CMouse::operator CPoint( void )
{ 
	return GetPos(); 
}


#endif  //_MOUSE_HDR_INCLUDED_