#include "stdafx.h"
#include "PEManagerRemoteVirtual.h"
#include <psapi.h>

#pragma comment(lib, "psapi.lib")


CPEManagerRemoteVirtual::CPEManagerRemoteVirtual() : _header_buf(NULL), _hproc(NULL)
{
	_runtime_object = true;
}

CPEManagerRemoteVirtual::~CPEManagerRemoteVirtual()
{
	CloseObject();
}

bool CPEManagerRemoteVirtual::OpenObject(void *handle)
{
	PPEOpenRemoteParams params = (PPEOpenRemoteParams)handle;
	DWORD protect;
	wchar_t name[MAX_PATH];
	bool found = false;

	_hmod = params->hmod;

	protect = PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ;
	if (!_readonly) {
		protect |= PROCESS_VM_WRITE;
	}

	_hproc = OpenProcess(protect, false, params->pid);
	if (!_hproc) {
		return SetError(E_SYSTEM, GetLastError(), "can't open process");
	}
	if (!GetModuleBaseNameW(_hproc, _hmod, name, MAX_PATH)) {
		return SetError(E_SYSTEM, GetLastError(), "can't open module");
	}

#if (PE_SYSTEM_WIN64 == 1)
	_imgbase.val64 = (ULONGLONG)_hmod;
#else
	_imgbase.val32l = (DWORD)_hmod;
	_imgbase.val32h = 0;
#endif

	_header_buf = VirtualAlloc(NULL, PE_HEADER_SIZE, MEM_COMMIT, PAGE_READWRITE);
	if (!_header_buf) {
		return SetError(E_SYSTEM, GetLastError(), "can't allocate memory");
	}

	return SetErrorOK;
}

void CPEManagerRemoteVirtual::CloseObject()
{
	if (_hproc) {
		CloseHandle(_hproc);
		_hproc = NULL;
	}
	if (_header_buf) {
		VirtualFree(_header_buf, 0, MEM_RELEASE);
		_header_buf = NULL;
	}
}

bool CPEManagerRemoteVirtual::ReloadObjectHeader()
{
	unsigned int sect_count = 0, temp;
	PIMAGE_SECTION_HEADER psect;
	SIZE_T readed;

	if (!SetWrAccess(_hmod, HeaderSize)) {
		return SetError(E_ACCESS_DENIED, __LINE__, NULL);
	}
	if (!ReadProcessMemory(_hproc, (void *)_hmod, _header_buf, HeaderSize, &readed)) {
		return SetError(E_SYSTEM, GetLastError(), "can't read memory");
	}
	if (!RestoreAccess()) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}

	if (!ParseHeader(_header_buf)) {
		return SetErrorInherit;
	}

	if (GetArch() == PE_32) {
		_virt_size = Alignment32(GetHOpt32()->SizeOfImage, PE_DEFAULT_VIRTUAL_ALIGNMENT);
	} else {
		_virt_size = Alignment32(GetHOpt64()->SizeOfImage, PE_DEFAULT_VIRTUAL_ALIGNMENT);
	}

	psect = GetSectorPtr(&sect_count);
	_module_size = 0;
	for (unsigned int i = 0; i < sect_count; i++) {
		temp = psect[i].PointerToRawData + Alignment32(psect[i].SizeOfRawData, PE_DEFAULT_FILE_ALIGNMENT);
		if (temp > _module_size) {
			_module_size = temp;
		}
	}

	return SetErrorOK;
}

bool CPEManagerRemoteVirtual::ReadObjectHeaderData(void *pbuffer, unsigned int buf_size, unsigned int *readed)
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

bool CPEManagerRemoteVirtual::WriteObjectHeader(void *pbuffer, unsigned int  size)
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

bool CPEManagerRemoteVirtual::ReadObjectRawData(DWORD roffset, void *pbuffer, unsigned int  size)
{
	DWORD voffset;
	unsigned int vsize;

	if (ConvRawToVirtual(roffset, &voffset, &vsize) == PE_MAP_OUT_OF_RANGE || vsize < size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}
	if (!ReadObjectVirtualData(voffset, pbuffer, size)) {
		return SetErrorInherit;
	}
	return SetErrorOK;
}

bool CPEManagerRemoteVirtual::WriteObjectRawData(DWORD roffset, void *pbuffer, unsigned int  size)
{
	DWORD voffset;
	unsigned int vsize;

	if (ConvRawToVirtual(roffset, &voffset, &vsize) == PE_MAP_OUT_OF_RANGE || vsize < size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}
	if (!WriteObjectVirtualData(voffset, pbuffer, size)) {
		return SetErrorInherit;
	}
	return SetErrorOK;
}

bool CPEManagerRemoteVirtual::ReadObjectVirtualData(DWORD voffset, void *pbuffer, unsigned int  size)
{
	SIZE_T readed;
	void *addr = (void *)((uintptr_t)_hmod + voffset);
	if (voffset + size > _virt_size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}
	/*if (!SetWrAccess(addr, size)) {
		return SetError(E_ACCESS_DENIED, __LINE__, NULL);
	}*/
	if (!ReadProcessMemory(_hproc, addr, pbuffer, size, &readed)) {
		return SetError(E_SYSTEM, GetLastError(), "can't read memory");
	}
	/*if (!RestoreAccess()) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}*/
	return SetErrorOK;
}

bool CPEManagerRemoteVirtual::WriteObjectVirtualData(DWORD voffset, void *pbuffer, unsigned int  size)
{
	SIZE_T written;
	void *addr = (void *)((uintptr_t)_hmod + voffset);
	if (voffset + size > _virt_size) {
		return SetError(E_OUT_OF_RANGE, __LINE__, NULL);
	}
	if (!SetWrAccess(addr, size)) {
		return SetError(E_ACCESS_DENIED, __LINE__, NULL);
	}
	if (!WriteProcessMemory(_hproc, addr, pbuffer, size, &written)) {
		return SetError(E_SYSTEM, GetLastError(), "can't write memory");
	}
	if (!RestoreAccess()) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}
	return SetErrorOK;
}

bool CPEManagerRemoteVirtual::ReloadObjectRelocs()
{
	_rels.RemoveAll();
	return _rels.LoadDir(this);
}

bool CPEManagerRemoteVirtual::SetWrAccess(void *addr, int size)
{
	MEMORY_BASIC_INFORMATION mbi;

	if (!VirtualQueryEx(_hproc, addr, &mbi, PE_HEADER_SIZE)) {
		return false;
	}
	if (mbi.Protect != PAGE_EXECUTE && mbi.Protect != PAGE_EXECUTE_READ 
	&& mbi.Protect != PAGE_NOACCESS && mbi.Protect != PAGE_READONLY) {
		_old_addr = NULL;
		return true;
	}

	_old_addr = addr;
	_old_size = size;

	if (!VirtualProtectEx(_hproc, _old_addr, _old_size, PAGE_EXECUTE_READWRITE, &_old_protect)) {
		return false;
	}

	return true;
}

bool CPEManagerRemoteVirtual::RestoreAccess()
{
	if (!_old_addr) {
		return true;
	}
	if (!VirtualProtectEx(_hproc, _old_addr, _old_size, _old_protect, &_old_protect)) {
		return false;
	}
	return true;
}

bool CPEManagerRemoteVirtual::CompareModuleNames(char *path1, char *path2)
{
	uintptr_t len1 = strlen(path1), 
		len2 = strlen(path2);
	intptr_t i1, i2;

	if (!len1 || !len2) {
		return false;
	}

	for (i1 = len1 - 1, i2 = len2 - 1; i1 >= 0 && i2 >= 0; i1--, i2--) {
		if (path1[i1] != path2[i2]) {
			break;
		}
		if (path1[i1] == '\\' || path2[i2] == '\\') {
			return true;
		}
	}
	if (i1 < 0 && i2 && path2[i2] == '\\') {
		return true;
	}
	if (i2 < 0 && i1 && path2[i1] == '\\') {
		return true;
	}

	return false;
}