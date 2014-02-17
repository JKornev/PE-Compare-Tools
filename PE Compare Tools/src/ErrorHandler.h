#ifndef __PEERRORNDLR_H
#define __PEERRORNDLR_H

#include "PEDefs.h"

//=======================================
//temp
#define ENABLE_PE_DEBUG_MESSAGES 1
#define ENABLE_PE_ERROR_DESCR 1
//=======================================

#if !defined(ENABLE_PE_DEBUG_MESSAGES)
#define ENABLE_PE_DEBUG_MESSAGES 0
#endif

#if !defined(ENABLE_PE_ERROR_DESCR)
#define ENABLE_PE_ERROR_DESCR 0
#endif


#if (ENABLE_PE_DEBUG_MESSAGES == 1)

#if (ENABLE_PE_ERROR_DESCR == 0)
#define SetError(code, sub, descr) SetErrorA(code, sub, NULL)
#else
#define SetError SetErrorA
#endif

#define SetErrorOK		SetError(E_OK, 0, NULL)
#define SetErrorInherit	SetError(E_INHERIT, 0, NULL)
//SetErrorInherit must used only with errors

#else//ENABLE_PE_DEBUG_MESSAGES != 1

#define SetError(code, sub, descr) false
#define SetErrorOK true
#define SetErrorInherit false

#endif

enum _DefErrorCode {
	//ok
	E_OK,
	//inherit last error
	E_INHERIT,
	//errors
	E_UNKNOWN,
	E_NOT_FOUND,
	E_ALLREADY_FOUND,
	E_OVERFLOW,
	E_OUT_OF_RANGE,
	E_ACCESS_DENIED,
	E_ALLOC_FAIL,
	E_NOT_SUPPORTED,
	E_NOT_ENOUGH,
	//windows error
	E_SYSTEM,
};

class CErrorCtrl {
	enum { _STR_BUF_SIZE = 256 };
	char _descr_buf[_STR_BUF_SIZE];
	unsigned int _error;
	unsigned int _sub;

public:
	bool SetErrorA(unsigned int error_code, unsigned int sub_code = 0, void *error_descr = NULL);
	void ClearError();

	CErrorCtrl();
	~CErrorCtrl();

	unsigned int LastError();
	unsigned int LastErrorSub();
	const char *LastErrorStr();
};

#endif