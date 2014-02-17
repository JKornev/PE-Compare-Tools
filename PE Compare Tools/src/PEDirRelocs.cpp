#include "stdafx.h"
#include "PEDirRelocs.h"
#include "PEManager.h"
#include "PEBuffer.h"


CPEDirRelocs::CPEDirRelocs()
{
}

CPEDirRelocs::~CPEDirRelocs()
{
}

bool CPEDirRelocs::LoadDir(IPEManager *pmngr)
{
	PIMAGE_DATA_DIRECTORY pdir;
	CPEBuffer buf(pmngr);
	void *dir_block;

	if (!pmngr->IsOpened()) {
		return false;
	}

	pdir = &pmngr->GetHDataDir()[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	if (!pdir->VirtualAddress || !pdir->Size) {
		return true;//not found OK
	}

	dir_block = buf.GetVirtualDataBlock(pdir->VirtualAddress, pdir->Size);
	if (!dir_block) {
		return false;
	}

	return LoadDirBuffer(dir_block, pdir->Size);
}

/*
bool CPEDirRelocs::LoadDirCached(IPEManager *pmngr)
{
	/ *CPEInfo pe;
	PIMAGE_DATA_DIRECTORY pdir;
	void *dir_block;
	//TOFIX
	header_block = pmngr->GetRawDataPtr(0, 
		(pmngr->GetArch() == PE_32 ? pmngr->GetHOpt32()->SizeOfHeaders : pmngr->GetHOpt64()->SizeOfHeaders));
	if (!header_block) {
		return false;
	}

	if (!pe.ParseHeader(header_block)) {
		return false;
	}

	pdir = &pe.GetHDataDir()[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	if (!pdir->VirtualAddress || !pdir->Size) {
		return true;//not found OK
	}

	dir_block = pmngr->GetVirtualDataPtr(pdir->VirtualAddress, pdir->Size);
	if (!dir_block) {
		return false;
	}

	return LoadDirBuffer(dir_block, pdir->Size);* /
	return false;
}*/

bool CPEDirRelocs::LoadDirBuffer(void *pbuf, unsigned int size)
{
	PIMAGE_BASE_RELOCATION prel;
	PWORD prels;
	DWORD count, type;
	std::map<DWORD, PE_Rels_Type> relocs;

	for (unsigned int i = 0; i < size;) {
		prel = (PIMAGE_BASE_RELOCATION)((uintptr_t)pbuf + i);
		i += prel->SizeOfBlock;

		if (prel->SizeOfBlock == 0) {
			break;
		}

		if (i <= size) {//add to list
			count = prel->SizeOfBlock / sizeof(WORD);
			prels = (PWORD)prel;
			for (unsigned int a = 4; a < count; a++) {
				type = prels[a] >> 12;
				switch (type) {
				case Rel_Absolute:
				case Rel_HighLow:
				case Rel_Dir64:
					relocs.insert(
						std::pair<DWORD, PE_Rels_Type>(prel->VirtualAddress + (0x00000FFF & prels[a]),
						(PE_Rels_Type)type));
					break;
				default:
					return false;
				}
			}
		}
	}

	if (!relocs.size()) {
		return false;
	}

	_rels.insert(relocs.begin(), relocs.end());
	return true;
}

bool CPEDirRelocs::AddRel(DWORD offset, PE_Rels_Type type)
{
	return _rels.insert(std::pair<DWORD, PE_Rels_Type>(offset, type)).second;
}

bool CPEDirRelocs::RemoveRel(DWORD offset)
{
	std::map<DWORD, PE_Rels_Type>::iterator it = _rels.find(offset);
	if (it == _rels.end()) {
		return false;
	}
	_rels.erase(it);
	return true;
}

void CPEDirRelocs::RemoveRange(DWORD offset, unsigned int range)
{
	std::map<DWORD, PE_Rels_Type>::iterator 
		it = _rels.lower_bound(offset),
		it_end = _rels.lower_bound(offset + range);
	_rels.erase(it, it_end);
}

void CPEDirRelocs::RemoveAll()
{
	_rels.clear();
}

unsigned int CPEDirRelocs::GetCount()
{
	return _rels.size();
}

unsigned int CPEDirRelocs::GetDirSize()
{
	std::map<DWORD, PE_Rels_Type>::iterator it = _rels.begin();
	DWORD ubound;
	unsigned int ofst_count = 0, size = 0;

	if (it != _rels.end()) {
		ubound = Alignment32(it->first, RelBlockAligm);
	} else {
		return 0;
	}

	while (it != _rels.end()) {
		if (it->first >= ubound) {//next block
			ubound = Alignment32(it->first, RelBlockAligm);
			if (ubound == it->first) {
				ubound += RelBlockAligm;
			}
			size += sizeof(IMAGE_BASE_RELOCATION) + (sizeof(WORD) * ofst_count);
			if (ofst_count & 1) {//aligment
				size += sizeof(WORD);
			}
			ofst_count = 0;
		}
		ofst_count++;
		it++;
	}

	size += sizeof(IMAGE_BASE_RELOCATION) + (sizeof(WORD) * ofst_count);
	if (ofst_count & 1) {//aligment
		size += sizeof(WORD);
	}
	return size;
}

bool CPEDirRelocs::BuildDir(void *pbuf, unsigned int size, unsigned int *pdir_size)
{//need optimization, TOTEST
	std::map<DWORD, PE_Rels_Type>::iterator it = _rels.begin();
	unsigned int res_size = GetDirSize(), offset = 0, a = 0;
	PIMAGE_BASE_RELOCATION prel;
	PWORD prels;
	DWORD ubound;

	if (!_rels.size() || res_size > size) {
		return false;
	}

	ubound = Alignment32(it->first, RelBlockAligm);
	prel = (PIMAGE_BASE_RELOCATION)pbuf;
	prel->VirtualAddress = AlignmentToLow<DWORD>(it->first, RelBlockAligm);
	a = 0;

	while (it != _rels.end()) {
		if (it->first >= ubound) {//next block
			ubound = Alignment32(it->first, RelBlockAligm);
			if (ubound == it->first) {
				ubound += RelBlockAligm;
			}

			prel->SizeOfBlock = sizeof(IMAGE_BASE_RELOCATION) + (sizeof(WORD) * a);
			if (a & 1) {//aligment
				prel->SizeOfBlock += sizeof(WORD);
			}

			offset += prel->SizeOfBlock;

			//set new
			prel = (PIMAGE_BASE_RELOCATION)((uintptr_t)pbuf + offset);
			prel->VirtualAddress = AlignmentToLow<DWORD>(it->first, RelBlockAligm);
			prels = (PWORD)&prel[1];
			a = 0;
		}
		
		prels[a] = (WORD)(it->first & 0x00000FFF) | (it->second << 12);
		a++, it++;
	}

	if (pdir_size) {
		*pdir_size = res_size;
	}

	return true;
}

unsigned int CPEDirRelocs::Commit32(void *pbuf, unsigned int size, DWORD offset, DWORD old_imgbase, DWORD new_imgbase)
{
	return CommitRelocs<DWORD>(pbuf, size, offset, old_imgbase, new_imgbase);
}

unsigned int CPEDirRelocs::Commit64(void *pbuf, unsigned int size, DWORD offset, ULONGLONG old_imgbase, ULONGLONG new_imgbase)
{
	return CommitRelocs<ULONGLONG>(pbuf, size, offset, old_imgbase, new_imgbase);
}

template<typename T>
unsigned int CPEDirRelocs::CommitRelocs(void *pbuf, unsigned int size, DWORD offset, T old_imgbase, T new_imgbase)
{
	std::map<DWORD, PE_Rels_Type>::iterator it = _rels.lower_bound(offset);
	unsigned int count = 0, 
		peak_offset = offset + size,
		offset_diff;
	T diff = new_imgbase - old_imgbase;

	while (it != _rels.end()) {
		if (it->first >= peak_offset) {
			break;
		}

		offset_diff = it->first - offset;

		switch (it->second) {
		case Rel_Absolute:
			//*(PDWORD)((uintptr_t)pbuf + offset_diff) += diff;
			break;
		case Rel_HighLow:
			if (offset_diff + sizeof(DWORD) > size) {
				break;
			}
			*(PDWORD)((uintptr_t)pbuf + offset_diff) += (DWORD)diff;
			break;
		case Rel_Dir64:
			if (offset_diff + sizeof(ULONGLONG) > size) {
				break;
			}
			*(PULONGLONG)((uintptr_t)pbuf + offset_diff) += (ULONGLONG)diff;
			break;
		default://unknown
			continue;
			break;
		}

		count++, it++;
	}

	return count;
}