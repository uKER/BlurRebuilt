#ifndef XTWIN_SVNAUTHDLG_H
#define XTWIN_SVNAUTHDLG_H

#ifdef _MSC_VER
#pragma once
#endif

class SvnAuthDlg
{
public:
	SvnAuthDlg(HWND parentWindow);
	~SvnAuthDlg();

public:
	bool Modal();

public:
	inline void SetAuthNotice(const XtString& authNotice)	{ _authNotice = authNotice;		}
	inline void SetUsername(const XtString& username)		{ _username = username;			}
	inline void SetPassword(const XtString& password)		{ _password = password;			}
	inline void SetRemember(bool remember)					{ _remember = remember;			}
	inline void SetCanRemember(bool canRemember)			{ _canRemember = canRemember;	}

public:
	inline XtString	AuthNotice()	{ return _authNotice;	}
	inline XtString	Username()		{ return _username;		}
	inline XtString	Password()		{ return _password;		}
	inline bool		Remember()		{ return _remember;		}
	inline bool		CanRemember()	{ return _canRemember;	}

private:
	static INT_PTR CALLBACK WndProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND		_parentWindow;
	XtString	_authNotice;
	XtString	_username;
	XtString	_password;
	bool		_remember;
	bool		_canRemember;
};

#endif // XTWIN_SVNAUTHDLG_H