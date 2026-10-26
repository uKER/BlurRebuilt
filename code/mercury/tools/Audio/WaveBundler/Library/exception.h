#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include "string.h"

class Error
{
public:
    Error(char* pchError, int iLine = -1, char* pchFile = NULL) { m_pchError = pchError; m_iLine = iLine; m_pchFile = pchFile; }
    Error(const char* pchError, int iLine = -1, const char* pchFile = NULL) { m_pchError = (char*)pchError; m_iLine = iLine; m_pchFile = (char*)pchFile; }

    inline char* What() { return m_pchError; }
    inline int When() { return m_iLine; }
    inline char* Where() { return m_pchFile; }

private:
    char* m_pchError;
    int m_iLine;
    char* m_pchFile;
};

#endif // _EXCEPTION_H_
