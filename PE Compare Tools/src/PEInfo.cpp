#include "stdafx.h"
#include "PEInfo.h"

CPEInfo::CPEInfo() : _loaded_flag(false)
{
}

CPEInfo::~CPEInfo()
{
}

bool CPEInfo::ParseHeader(void *buffer)
{
	char *data = (char *)buffer;
	DWORD offset = 0;

	_loaded_flag = false;

	// Set dos header
	_pdos = (PIMAGE_DOS_HEADER)data;
	if (_pdos->e_magic != IMAGE_DOS_SIGNATURE) {
		return SetError(E_UNKNOWN, __LINE__, "parse: incorrect dos signature");
	}
	offset = _pdos->e_lfanew;

	if (*(DWORD *)(data + offset) != (DWORD)IMAGE_NT_SIGNATURE) {
		return SetError(E_UNKNOWN, __LINE__, "parse: incorrect pe signature");
	}
	offset += sizeof(DWORD);

	// Set PE header
	_pimg = (PIMAGE_FILE_HEADER)(data + offset);
	offset += sizeof(IMAGE_FILE_HEADER);
	_sect_count = _pimg->NumberOfSections;

	// Detect PE architecture
	switch (_pimg->Machine) {
	case IMAGE_FILE_MACHINE_I386:
		_arch = PE_32;
		break;
	case IMAGE_FILE_MACHINE_IA64:
	case IMAGE_FILE_MACHINE_AMD64:
		_arch = PE_64;
		break;
	default:
		_arch = PE_UNK;
	}

	// Set PE optional header
	if (GetArch() == PE_32) {
		_popt32 = (PIMAGE_OPTIONAL_HEADER32)(data + offset);
		_popt64 = NULL;
		_pdir = _popt32->DataDirectory;
		_virt_aligm = _popt32->SectionAlignment;
		_raw_aligm = _popt32->FileAlignment;
		offset += sizeof(IMAGE_OPTIONAL_HEADER32);

		_virt_peak = _popt32->SizeOfImage;
	} else if (GetArch() == PE_64) {
		_popt32 = NULL;
		_popt64 = (PIMAGE_OPTIONAL_HEADER64)(data + offset);
		_pdir = _popt64->DataDirectory;
		_virt_aligm = _popt64->SectionAlignment;
		_raw_aligm = _popt64->FileAlignment;
		offset += sizeof(IMAGE_OPTIONAL_HEADER64);

		_virt_peak = _popt64->SizeOfImage;
	} else {
		return SetError(E_UNKNOWN, __LINE__, "parse: unknown architecture");
	}

	// Set PE sections
	_psects = (PIMAGE_SECTION_HEADER)(data + offset);
	offset += (sizeof(IMAGE_SECTION_HEADER) * _pimg->NumberOfSections);
	if (offset > _virt_aligm) {
		return SetError(E_OUT_OF_RANGE, __LINE__, "parse: section info out of range");
	}

	// Calc raw peak
	if (GetArch() == PE_32) {
		_raw_peak = CalcRawPeak<PIMAGE_OPTIONAL_HEADER32>(_popt32);
	} else {
		_raw_peak = CalcRawPeak<PIMAGE_OPTIONAL_HEADER64>(_popt64);
	}

	_loaded_flag = true;
	return SetErrorOK;
}

template<typename T>
unsigned int CPEInfo::CalcRawPeak(T popt)
{
	unsigned int peak, res = 0;
	for (unsigned int i = 0; i < _sect_count; i++) {
		peak = _psects[i].PointerToRawData + _psects[i].SizeOfRawData;
		if (res < peak) {
			res = peak;
		}
	}
	return res;
}

bool CPEInfo::HeaderIsLoaded()
{
	return _loaded_flag;
}

PE_Architecture CPEInfo::GetArch()
{
	return _arch;
}

DWORD CPEInfo::GetVirtualAligment()
{
	return _virt_aligm;
}

DWORD CPEInfo::GetRawAligment()
{
	return _raw_aligm;
}

PIMAGE_DOS_HEADER CPEInfo::GetHDos()
{
	return _pdos;
}

PIMAGE_FILE_HEADER CPEInfo::GetHImg()
{
	return _pimg;
}

PIMAGE_OPTIONAL_HEADER32 CPEInfo::GetHOpt32()
{
	return _popt32;
}

PIMAGE_OPTIONAL_HEADER64 CPEInfo::GetHOpt64()
{
	return _popt64;
}

PIMAGE_DATA_DIRECTORY CPEInfo::GetHDataDir()
{
	return _pdir;
}

PIMAGE_SECTION_HEADER CPEInfo::GetSectorPtr(unsigned int *pcount)
{
	if (pcount) {
		*pcount = _sect_count;
	}
	return _psects;
}

PIMAGE_SECTION_HEADER CPEInfo::GetSectorByPos(int pos)
{
	if (pos == PE_INVALID_SECTOR || pos >= (int)_sect_count) {
		return NULL;
	}
	return &_psects[pos];
}

int CPEInfo::FindSectorPosByName(char *pname)
{
	char buf[IMAGE_SIZEOF_SHORT_NAME + 1] = {0};
	for (unsigned int i = 0; i < _sect_count; i++) {
		memcpy(buf, _psects[i].Name, IMAGE_SIZEOF_SHORT_NAME);
		if (!strcmp(buf, pname)) {
			return i;
		}
	}
	return PE_INVALID_SECTOR;
}

int CPEInfo::FindSectorPosByVirtual(DWORD voffset)
{
	int num = PE_INVALID_SECTOR;
	for (unsigned int i = 0; i < _sect_count; i++) {
		if (_psects[i].VirtualAddress <= voffset 
		&& _psects[i].VirtualAddress + _psects[i].Misc.VirtualSize > voffset) {
			if (_psects[i].SizeOfRawData == 0) {
				num = i;
				continue;
			}
			return i;
		}
	}
	return num;
}

int CPEInfo::FindSectorPosByRaw(DWORD roffset)
{
	for (unsigned int i = 0; i < _sect_count; i++) {
		if (_psects[i].SizeOfRawData && _psects[i].PointerToRawData <= roffset 
		&& _psects[i].PointerToRawData + _psects[i].SizeOfRawData > roffset) {
			return i;
		}
	}
	return PE_INVALID_SECTOR;
}

PE_Mem_Map CPEInfo::ConvRawToVirtual(DWORD roffset, DWORD *pvoffset, unsigned int *pblock_size)
{
	PIMAGE_SECTION_HEADER psect;
	DWORD offset;

	if (roffset >= _raw_peak) {
		return PE_MAP_OUT_OF_RANGE;
	}

	//check header
	unsigned int header_size = (GetArch() == PE_32 ? GetHOpt32()->SizeOfHeaders : GetHOpt64()->SizeOfHeaders);
	if (roffset < header_size) {//TODO
		*pvoffset = roffset;
		if (pblock_size) {
			*pblock_size = _virt_aligm - roffset;
		}
		return PE_MAP_HEADER;
	}

	//check section
	psect = GetSectorByPos(FindSectorPosByRaw(roffset));
	if (psect) {
		offset = roffset - psect->PointerToRawData;
		*pvoffset = offset + psect->VirtualAddress;

		unsigned int vsize = Alignment32(psect->Misc.VirtualSize, _virt_aligm);
		unsigned int peak_virt = psect->VirtualAddress + vsize;
		if (*pvoffset > peak_virt) {
			return PE_MAP_OUT_OF_RANGE;
		}
		if (pblock_size) {
			*pblock_size = psect->VirtualAddress + vsize - offset;
		}
		return PE_MAP_SECTOR;
	}

	return PE_MAP_OUT_OF_RANGE;
}

PE_Mem_Map CPEInfo::ConvVirtualToRaw(DWORD voffset, DWORD *proffset, unsigned int *pblock_size)
{
	PIMAGE_SECTION_HEADER psect;
	DWORD offset;

	//check header
	if (voffset < _virt_aligm) {
		unsigned int header_size = (GetArch() == PE_32 ? GetHOpt32()->SizeOfHeaders : GetHOpt64()->SizeOfHeaders);
		if (voffset >= header_size) {
			return PE_MAP_OUT_OF_RANGE;
		}
		*proffset = voffset;
		if (pblock_size) {
			*pblock_size = header_size - voffset;
		}
		return PE_MAP_HEADER;
	}

	//check section
	psect = GetSectorByPos(FindSectorPosByVirtual(voffset));
	if (psect) {
		offset = voffset - psect->VirtualAddress;
		*proffset = offset + psect->PointerToRawData;

		unsigned int peak_raw = psect->PointerToRawData + psect->SizeOfRawData/*Aligment32(psect->SizeOfRawData, _raw_aligm)*/;
		if (*proffset >= peak_raw) {
			return PE_MAP_OUT_OF_RANGE;
		}
		if (pblock_size) {
			*pblock_size = peak_raw - *proffset;
		}
		return PE_MAP_SECTOR;
	}

	return PE_MAP_OUT_OF_RANGE;
}

PE_Mem_Map CPEInfo::GetRawBlockSize(DWORD roffset, unsigned int *pblock_size)
{
	PIMAGE_SECTION_HEADER psect;

	if (roffset >= _raw_peak) {
		return PE_MAP_OUT_OF_RANGE;
	}

	//check header
	unsigned int header_size = (GetArch() == PE_32 ? GetHOpt32()->SizeOfHeaders : GetHOpt64()->SizeOfHeaders);
	if (roffset < header_size) {
		*pblock_size = header_size - roffset;
		return PE_MAP_HEADER;
	}

	//check section
	psect = GetSectorByPos(FindSectorPosByRaw(roffset));
	if (psect) {
		unsigned int peak_raw = psect->PointerToRawData + psect->SizeOfRawData;
		if (roffset >= peak_raw) {
			return PE_MAP_OUT_OF_RANGE;
		}
		*pblock_size = peak_raw - roffset;
		return PE_MAP_SECTOR;
	}

	return PE_MAP_OUT_OF_RANGE;
}

PE_Mem_Map CPEInfo::GetVirtualBlockSize(DWORD voffset, unsigned int *pblock_size)
{
	PIMAGE_SECTION_HEADER psect;

	//check header
	if (voffset < _virt_aligm) {
		unsigned int header_size = (GetArch() == PE_32 ? GetHOpt32()->SizeOfHeaders : GetHOpt64()->SizeOfHeaders);
		if (voffset >= header_size) {
			return PE_MAP_OUT_OF_RANGE;
		}
		*pblock_size = header_size - voffset;
		return PE_MAP_HEADER;
	}

	//check section
	psect = GetSectorByPos(FindSectorPosByVirtual(voffset));
	if (psect) {
		unsigned int vsize = Alignment32(psect->Misc.VirtualSize, _virt_aligm);
		unsigned int peak_virt = psect->VirtualAddress + vsize;
		if (voffset > peak_virt) {
			return PE_MAP_OUT_OF_RANGE;
		}
		*pblock_size = psect->VirtualAddress + vsize - voffset;
		return PE_MAP_SECTOR;
	}

	return PE_MAP_OUT_OF_RANGE;
}

unsigned int CPEInfo::GetPeakVirtualSize()
{
	return _virt_peak;
}

unsigned int CPEInfo::GetPeakRawFileSize()
{
	return _raw_peak;
}