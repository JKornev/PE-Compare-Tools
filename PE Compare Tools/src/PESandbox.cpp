#include "stdafx.h"
#include "PESandbox.h"


CPESandbox::CPESandbox() : _attached(false)
{
}

CPESandbox::~CPESandbox()
{
	Detach();
}

bool CPESandbox::Attach(IPEManager *pmngr, unsigned int size, bool use_relocs)
{
	if (!pmngr->IsOpened()) {
		return false;
	}

	_sandbox_size = size;
	if (!_sandbox_size) {
		_sandbox_size = pmngr->GetSize();
	}

	if (!AllocRegion(_sandbox_size)) {
		return false;
	}

	if (!RegDataLoadingCallback(DataLoadingProc, this)) {
		DestroyRegion();
		return false;
	}
	if (!RegDataUnloadingCallback(DataUnloadingProc, this)) {
		DestroyRegion();
		return false;
	}

	_pmngr = pmngr;
	return _attached = true;
}

void CPESandbox::Detach()
{
	if (!_attached) {
		return;
	}
	DestroyRegion();
	_attached = false;
}

bool CPESandbox::IsAttached()
{
	return _attached;
}

unsigned int CPESandbox::GetSize()
{
	return _sandbox_size;
}

bool CPESandbox::SetSize(unsigned int size)
{
	if (!_attached || size < PE_HEADER_RAW_SIZE) {
		return false;
	}
	if (!ReallocRegion(size)) {
		return false;
	}
	_sandbox_size = size;
	return true;
}

char *CPESandbox::GetRawDataPtr(DWORD roffset, unsigned int size)
{
	if (!_attached) {
		return NULL;
	}
	if (roffset + size > _sandbox_size) {
		return NULL;
	}
	return (char *)GetMappedData(roffset, size);
}

char *CPESandbox::GetVirtualDataPtr(DWORD voffset, unsigned int size)
{
	DWORD roffset;
	unsigned int rsize;
	if (!_attached) {
		return NULL;
	}
	if (_pmngr->ConvVirtualToRaw(voffset, &roffset, &rsize) == PE_MAP_OUT_OF_RANGE || rsize < size) {
		return NULL;
	}
	return GetRawDataPtr(roffset, size);
}

bool CPESandbox::FlushBuffer()
{
	if (!_attached) {
		return false;
	}
	return FlushMappedData(0, _pmngr->GetSize());
}

bool CPESandbox::ClearBuffer()
{
	if (!_attached) {
		return false;
	}
	return UnassignAllPages();
}

bool CPESandbox::DataLoadingProc(DWORD roffset, void *buffer, unsigned int size, void *param)
{
	CPESandbox *pthis = (CPESandbox *)param;

	if (roffset >= pthis->_pmngr->GetSize()) {
		return true;
	} else if (roffset + size >= pthis->_pmngr->GetSize()) {
		size = pthis->_pmngr->GetSize() - roffset;
	}

	if (pthis->_pmngr->IsRuntimeObject()) {//Если обьект в рантайме, то догружаем его покускам в случае необходимости
		unsigned int sector_size;
		DWORD offset = 0;
		//TOTEST мб необходимо выравнивание
		while (size) {
			if (pthis->_pmngr->GetRawBlockSize(roffset, &sector_size) == PE_MAP_OUT_OF_RANGE) {
				return false;
			}

			if (sector_size >= size) {
				sector_size = size;
				size = 0;
			} else {
				size -= sector_size;
			}

			if (!pthis->_pmngr->ReadRawData(roffset + offset, (void *)((uintptr_t)buffer + offset), sector_size, pthis->_use_relocs)) {
				return false;
			}

			offset += sector_size;
		}

	} else if (!pthis->_pmngr->ReadRawData(roffset, buffer, size, pthis->_use_relocs)) {
		return false;
	}

	return true;
}

bool CPESandbox::DataUnloadingProc(DWORD roffset, void *buffer, unsigned int size, void *param)
{
	CPESandbox *pthis = (CPESandbox *)param;

	if (roffset >= pthis->_pmngr->GetSize()) {
		return true;
	} else if (roffset + size >= pthis->_pmngr->GetSize()) {
		size = pthis->_pmngr->GetSize() - roffset;
	}

	if (pthis->_pmngr->IsRuntimeObject()) {//TOTEST
		unsigned int sector_size;
		DWORD offset = 0;
		while (size) {
			if (pthis->_pmngr->GetRawBlockSize(roffset, &sector_size) == PE_MAP_OUT_OF_RANGE) {
				return false;
			}

			if (sector_size >= size) {
				sector_size = size;
				size = 0;
			} else {
				size -= sector_size;
			}

			if (!pthis->_pmngr->WriteRawData(roffset + offset, (void *)((uintptr_t)buffer + offset), sector_size, pthis->_use_relocs)) {
				return false;
			}

			offset += sector_size;
		}
	} else if (!pthis->_pmngr->WriteRawData(roffset, buffer, size, pthis->_use_relocs)) {
		return false;
	}
	return true;
}

