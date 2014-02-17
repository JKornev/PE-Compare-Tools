#include "stdafx.h"
#include "ErrorHandler.h"
#include <string.h>

CErrorCtrl::CErrorCtrl() : _error(E_OK), _sub(0)
{ 
	_descr_buf[0] = '\0';
}

CErrorCtrl::~CErrorCtrl()
{
}

bool CErrorCtrl::SetErrorA(unsigned int error_code, unsigned int sub_code, void *error_descr)
{
	int len;

	if (error_code != E_INHERIT) {
		_error = error_code;
		_sub = sub_code;

		if (NULL == error_descr) {
			_descr_buf[0] = '\0';
		} else {
			len = strlen((char *)error_descr);

			if (len >= _STR_BUF_SIZE) {
				len = _STR_BUF_SIZE - 2;
			}

			memcpy(&_descr_buf, error_descr, len);
			_descr_buf[len] = '\0';
		} 
	}

	return (_error == E_OK ? true : false);
}

unsigned int CErrorCtrl::LastError()
{
	return _error;
}

unsigned int CErrorCtrl::LastErrorSub()
{
	return _sub;
}

const char *CErrorCtrl::LastErrorStr()
{
	if (_descr_buf[0] == '\0') {
		return NULL;
	}
	return (const char *)_descr_buf;
}

void CErrorCtrl::ClearError()
{
	_error = E_OK;
	_sub = 0;
}