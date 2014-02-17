#include "stdafx.h"
#include "PEManagerFile.h"


CPEManagerFile::CPEManagerFile() : _header_buf(NULL), _hfile(INVALID_HANDLE_VALUE)
{
}

CPEManagerFile::~CPEManagerFile()
{
	Close();
}

bool CPEManagerFile::OpenObject(void *handle)
{
	wchar_t *wpath = (wchar_t *)handle;

	_hfile = CreateFileW(wpath, GENERIC_READ | (_readonly ? 0 : GENERIC_WRITE), FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (_hfile == INVALID_HANDLE_VALUE) {
		return SetError(E_SYSTEM, GetLastError(), "manager: create file error");
	}

	_module_size = GetFileSize(_hfile, NULL);

	_header_buf = VirtualAlloc(NULL, HeaderSize, MEM_COMMIT, PAGE_READWRITE);
	if (!_header_buf) {
		return SetError(E_SYSTEM, GetLastError(), "manager: create file mapping error");
	}

	return SetErrorOK;
}

void CPEManagerFile::CloseObject()
{
	if (_hfile != INVALID_HANDLE_VALUE) {
		CloseHandle(_hfile);
		_hfile = INVALID_HANDLE_VALUE;
	}
	if (_header_buf) {
		VirtualFree(_header_buf, 0, MEM_RELEASE);
		_header_buf = NULL;
	}
}

bool CPEManagerFile::ReloadObjectHeader()
{
	DWORD readed;

	SetFilePointer(_hfile, 0, NULL, FILE_BEGIN);
	if (!ReadFile(_hfile, _header_buf, HeaderSize, &readed, NULL)) {
		return SetErrorInherit;
	}

	if (!ParseHeader(_header_buf)) {
		return SetErrorInherit;
	}

	if (GetArch() == PE_32) {
		_imgbase.val32l = GetHOpt32()->ImageBase;
		_imgbase.val32h = 0;
	} else {
		_imgbase.val64 = GetHOpt64()->ImageBase;
	}

	return SetErrorOK;
}

bool CPEManagerFile::ReadObjectHeaderData(void *pbuffer, unsigned int buf_size, unsigned int *readed)
{
	unsigned int header_size;

	if (GetArch() == PE_32) {
		header_size = GetHOpt32()->SizeOfHeaders;
	} else {
		header_size = GetHOpt64()->SizeOfHeaders;
	}

	if (header_size > HeaderSize || header_size > buf_size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}

	if (!ReadObjectRawData(0, pbuffer, header_size)) {
		return SetErrorInherit;
	}

	*readed = header_size;
	return SetErrorOK;
}

bool CPEManagerFile::WriteObjectHeader(void *pbuffer, unsigned int  size)
{
	unsigned int header_size;

	if (GetArch() == PE_32) {
		header_size = GetHOpt32()->SizeOfHeaders;
	} else {
		header_size = GetHOpt64()->SizeOfHeaders;
	}

	if (!pbuffer) {
		pbuffer = _header_buf;
		size = header_size;
	} else if (size > header_size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}

	if (!WriteObjectRawData(0, pbuffer, size)) {
		return SetErrorInherit;
	}
	if (!ReloadHeader()) {
		return SetErrorInherit;
	}

	return SetErrorOK;
}

bool CPEManagerFile::ReadObjectRawData(DWORD roffset, void *pbuffer, unsigned int  size)
{
	DWORD readed;

	if (roffset + size > _module_size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}

	SetFilePointer(_hfile, roffset, 0, FILE_BEGIN);
	if (!ReadFile(_hfile, pbuffer, size, &readed, NULL)) {
		return SetError(E_SYSTEM, GetLastError(), "manager: file writting error");
	}

	return SetErrorOK;
}

bool CPEManagerFile::WriteObjectRawData(DWORD roffset, void *pbuffer, unsigned int  size)
{
	DWORD written;
	if (roffset + size > _module_size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}

	SetFilePointer(_hfile, roffset, 0, FILE_BEGIN);
	if (!WriteFile(_hfile, pbuffer, size, &written, NULL)) {
		return SetError(E_SYSTEM, GetLastError(), "manager: file writting error");
	}

	return SetErrorOK;
}

bool CPEManagerFile::ReadObjectVirtualData(DWORD voffset, void *pbuffer, unsigned int  size)
{
	DWORD roffset;
	unsigned int rsize;

	if (ConvVirtualToRaw(voffset, &roffset, &rsize) == PE_MAP_OUT_OF_RANGE || rsize < size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}
	if (!ReadObjectRawData(roffset, pbuffer, size)) {
		return SetErrorInherit;
	}
	return SetErrorOK;
}

bool CPEManagerFile::WriteObjectVirtualData(DWORD voffset, void *pbuffer, unsigned int  size)
{
	DWORD roffset;
	unsigned int rsize;

	if (ConvVirtualToRaw(voffset, &roffset, &rsize) == PE_MAP_OUT_OF_RANGE || rsize < size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}
	if (!WriteObjectRawData(roffset, pbuffer, size)) {
		return SetErrorInherit;
	}
	return SetErrorOK;
}

bool CPEManagerFile::ReloadObjectRelocs()
{
	_rels.RemoveAll();
	return _rels.LoadDir(this);
}
