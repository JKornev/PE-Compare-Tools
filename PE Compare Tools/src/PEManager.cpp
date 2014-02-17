#include "stdafx.h"
#include "PEManager.h"


IPEManager::IPEManager() : _opened(false), _mode_relocs(false), _module_size(0), _runtime_object(false), _readonly(false)
{
}

IPEManager::~IPEManager() 
{
	CloseManager();
}

bool IPEManager::Open(void *handle, bool use_relocs, bool readonly)
{
	if (IsOpened()) {
		return SetError(E_ALLREADY_FOUND, __LINE__, NULL);
	}
	
	_module_size = 0;
	_imgbase.val64 = 0;
	_rel_imgbase.val64 = 0;
	_readonly = readonly;
	
	if (!OpenObject(handle)) {
		Close();
		return SetErrorInherit;
	}
	
	if (!ReloadObjectHeader()) {
		Close();
		return SetErrorInherit;
	}

	if (!_imgbase.val64) {
		if (GetArch() == PE_32) {
			_imgbase.val32l = GetHOpt32()->ImageBase;
			_imgbase.val32h = 0;
		} else {
			_imgbase.val64 = GetHOpt64()->ImageBase;
		}
	}
	if (!_rel_imgbase.val64) {
		_rel_imgbase = _imgbase;
	}
	
	_opened = true;

	if (use_relocs && !SetRelocsModeState(true)) {
		Close();
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}

	return SetErrorOK;
}

void IPEManager::Close()
{
	CloseObject();
	CloseManager();
}

bool IPEManager::IsOpened()
{ 
	return _opened; 
}

bool IPEManager::IsReadOnly()
{
	if (!IsOpened()) {
		return SetError(E_NOT_FOUND, __LINE__, NULL);
	}
	return _readonly;
}

bool IPEManager::IsRuntimeObject()
{
	return _runtime_object;
}

bool IPEManager::ReloadHeader()
{
	if (!IsOpened()) {
		return SetError(E_NOT_FOUND, __LINE__, NULL);
	}
	return ReloadObjectHeader();
}

bool IPEManager::ReadHeaderData(void *pbuffer, unsigned int buf_size, unsigned int *readed, bool use_relocs)
{
	if (!IsOpened()) {
		return SetError(E_NOT_FOUND, __LINE__, NULL);
	}
	if (!ReadObjectHeaderData(pbuffer, buf_size, readed)) {
		return SetErrorInherit;
	}
	if (use_relocs && !CommitRelocs(pbuffer, *readed, 0, false, false)) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}
	return SetErrorOK;//inherit
}

bool IPEManager::WriteHeader(void *pbuffer, unsigned int  size, bool use_relocs)
{
	if (!IsOpened() || _readonly) {
		return SetError(E_NOT_FOUND, __LINE__, NULL);
	}
	if (use_relocs && !CommitRelocs(pbuffer, size, 0, false, true)) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}
	return WriteObjectHeader(pbuffer, size);
}

bool IPEManager::ReadRawData(DWORD roffset, void *pbuffer, unsigned int  size, bool use_relocs)
{
	if (!IsOpened()) {
		return SetError(E_NOT_FOUND, __LINE__, NULL);
	}
	if (!ReadObjectRawData(roffset, pbuffer, size)) {
		return SetErrorInherit;
	}
	if (use_relocs && !CommitRelocs(pbuffer, size, roffset, true, false)) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}
	return SetErrorOK;//inherit
}

bool IPEManager::WriteRawData(DWORD roffset, void *pbuffer, unsigned int  size, bool use_relocs)
{
	if (!IsOpened() || _readonly) {
		return SetError(E_NOT_FOUND, __LINE__, NULL);
	}
	if (use_relocs && !CommitRelocs(pbuffer, size, roffset, true, true)) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}
	return WriteObjectRawData(roffset, pbuffer, size);
}

bool IPEManager::ReadVirtualData(DWORD voffset, void *pbuffer, unsigned int  size, bool use_relocs)
{
	if (!IsOpened()) {
		return SetError(E_NOT_FOUND, __LINE__, NULL);
	}
	if (!ReadObjectVirtualData(voffset, pbuffer, size)) {
		return SetErrorInherit;
	}
	if (use_relocs && !CommitRelocs(pbuffer, size, voffset, false, false)) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}
	return SetErrorOK;//inherit
}

bool IPEManager::WriteVirtualData(DWORD voffset, void *pbuffer, unsigned int  size, bool use_relocs)
{
	if (!IsOpened() || _readonly) {
		return SetError(E_NOT_FOUND, __LINE__, NULL);
	}
	if (use_relocs && !CommitRelocs(pbuffer, size, voffset, false, true)) {
		return SetError(E_UNKNOWN, __LINE__, NULL);
	}
	return WriteObjectVirtualData(voffset, pbuffer, size);
}

bool IPEManager::ReloadRelocs()
{
	if (!GetRelocsModeState()) {
		return false;
	}
	return ReloadObjectRelocs();
}

bool IPEManager::GetRelocsModeState()
{
	if (!IsOpened()) {
		return false;
	}
	return _mode_relocs;
}

bool IPEManager::SetRelocsModeState(bool enable)
{
	if (!IsOpened()) {
		return false;
	}
	if (GetRelocsModeState() == enable) {
		return true;
	}
	if (enable) {//enable
		_mode_relocs = true;
		if (!ReloadRelocs()) {
			return _mode_relocs = false;
		}
	} else {//disable
		_rels.RemoveAll();
		_mode_relocs = false;
	}

	return true;
}

bool IPEManager::GetRelocsBase(UAddress *pbase)
{
	if (!GetRelocsModeState()) {
		return false;
	}
	*pbase = _rel_imgbase;
	return true;
}

bool IPEManager::SetRelocsBase(UAddress *pbase)
{
	if (!GetRelocsModeState()) {
		return false;
	}
	Swamp<UAddress>(*pbase, _rel_imgbase);
	return true;
}

unsigned int IPEManager::GetSize()
{
	if (!IsOpened()) {
		return 0;
	}
	return _module_size;
}

bool IPEManager::CommitRelocs(void *pbuffer, unsigned int  size, DWORD offset, bool offset_raw, bool rel_to_img)
{
	if (!_mode_relocs) {
		return true;//ignore
	}
	//raw to virtual
	if (offset_raw && ConvRawToVirtual(offset, &offset, 0) == PE_MAP_OUT_OF_RANGE) {
		return false;
	}
	//commit relocs
	if (GetArch() == PE_32) {
		_rels.Commit32(pbuffer, size, offset, 
			(rel_to_img ? _rel_imgbase.val32l : _imgbase.val32l), 
			(rel_to_img ? _imgbase.val32l : _rel_imgbase.val32l));
	} else {
		_rels.Commit64(pbuffer, size, offset, 
			(rel_to_img ? _rel_imgbase.val64 : _imgbase.val64), 
			(rel_to_img ? _imgbase.val64 : _rel_imgbase.val64));
	}
	return true;
}

void IPEManager::CloseManager()
{
	SetRelocsModeState(false);
	_opened = false;
}
