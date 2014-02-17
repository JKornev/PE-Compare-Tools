#include "stdafx.h"
#include "PECompare.h"

using namespace std;


CPECompare::CPECompare() : 
	_loaded(false), 
	_active_enum(false),
	_diff_count(0)
{
	InitializeCriticalSection(&_csect);
	InitializeCriticalSection(&_csect_io);
}

CPECompare::~CPECompare()
{
	Clear();
	DeleteCriticalSection(&_csect);
	DeleteCriticalSection(&_csect_io);
}

bool CPECompare::Init(CPESandbox *src_sbox, IPEManager *dest_mngr, unsigned int segment_size)
{
	if (_loaded) {
		return false;
	}
	if (!src_sbox->IsAttached() || !dest_mngr->IsOpened()) {
		return false;
	}
	if (segment_size % 0x10 > 0) {
		return false;
	}

	_segment = segment_size;
	_sbox = src_sbox;
	_mngr = dest_mngr;
	return _loaded = true;
}

void CPECompare::Clear()
{
	DeleteAllRanges();
	_diff_count = 0;
	_active_enum = false;
	_loaded = false;
}

bool CPECompare::AddRange(DWORD offset, unsigned int size, unsigned int align)
{
	Range_Header header;
	if (!_loaded || _active_enum) {
		return false;
	}
	if (align != 1 && align != 2 && align != 4 && align != 8) {
		return false;
	}

	header.first = new Range_Elem;
	header.first->next = NULL;
	header.first->prev = NULL;
	header.first->offset = 0;
	header.first->size = size;

	header.offset = offset;
	header.size = size;
	header.align = align;

	_ranges.push_back(header);

	return true;
}

bool CPECompare::DeleteRange(DWORD offset)
{
	list<Range_Header>::iterator it = _ranges.begin();
	if (!_loaded || _active_enum) {
		return false;
	}
	while (it != _ranges.end()) {
		if (it->offset == offset) {
			RemoveList(&*it);
			_ranges.erase(it);
			return true;
		}
		it++;
	}
	return false;
}

void CPECompare::DeleteAllRanges()
{
	list<Range_Header>::iterator it = _ranges.begin();
	if (!_loaded || _active_enum) {
		return;
	}
	while (it != _ranges.end()) {
		RemoveList(&*it);
		it++;
	}
	_ranges.clear();
}

bool CPECompare::CompareInit()
{
	if (!_loaded) {
		return false;
	}

	EnterCriticalSection(&_csect);

	if (_ranges.size() == 0) {
		LeaveCriticalSection(&_csect);
		return false;
	}

	_enum_it = _ranges.begin();
	_active_enum = true;

	LeaveCriticalSection(&_csect);

	return true;
}

int CPECompare::CompareNext()
{//0 - ok, -1 - error, 1 - end
	list<Range_Header>::iterator it;
	int res = 0;
	char *buf, *orig_buf;
	Range_Elem *pelem;
	bool bres;
	DWORD offset;

	EnterCriticalSection(&_csect);
	if (!_loaded || !_active_enum) {
		res = -1;
	} else if (_enum_it == _ranges.end()) {
		_active_enum = false;
		res = 1;
	} else {
		it = _enum_it;
		_enum_it++;
	}
	LeaveCriticalSection(&_csect);
	if (res != 0) {
		return res;
	}

	//data load
	EnterCriticalSection(&_csect_io);
	orig_buf = _sbox->GetVirtualDataPtr(it->offset, it->size);
	LeaveCriticalSection(&_csect_io);
	if (!orig_buf) {
		return -1;
	}

	buf = (char *)VirtualAlloc(NULL, _segment, MEM_COMMIT, PAGE_READWRITE);
	if (!buf) {
		return -1;
	}

	pelem = it->first;
	while (pelem) {
		unsigned int 
			count = pelem->size / _segment,
			stub_size = pelem->size % _segment,
			size, diff_size;
		DWORD diff_offset, roffset;

		//compare segment
		offset = pelem->offset;
		for (unsigned int i = 0; i < count; i++, offset += _segment) {
			size = _segment;
			roffset = 0;

			EnterCriticalSection(&_csect_io);
			bres = _mngr->ReadVirtualData(it->offset + offset, buf, size, true);
			LeaveCriticalSection(&_csect_io);
			if (!bres) {
				continue;
			}

			switch (it->align) {
			case 1:
				while (NextDiff<unsigned __int8>(orig_buf + offset, buf, roffset, size, diff_offset, diff_size)) {
					RemoveRange(it, pelem, (offset + diff_offset), diff_size);
				}
				break;
			case 2:
				while (NextDiff<unsigned __int16>(orig_buf + offset, buf, roffset, size, diff_offset, diff_size)) {
					RemoveRange(it, pelem, (offset + diff_offset), diff_size);
				}
				break;
			case 4:
				while (NextDiff<unsigned __int32>(orig_buf + offset, buf, roffset, size, diff_offset, diff_size)) {
					RemoveRange(it, pelem, (offset + diff_offset), diff_size);
				}
				break;
			case 8:
				while (NextDiff<unsigned __int64>(orig_buf + offset, buf, roffset, size, diff_offset, diff_size)) {
					RemoveRange(it, pelem, (offset + diff_offset), diff_size);
				}
				break;
			default:
				VirtualFree(buf, 0, MEM_RELEASE);
				return -1;
			}
		}
		if (!pelem) {
			break;
		}

		//compare 
		size = stub_size;
		if (size != 0) {
			roffset = 0;

			EnterCriticalSection(&_csect_io);
			bres = _mngr->ReadVirtualData(it->offset + offset, buf, size, true);
			LeaveCriticalSection(&_csect_io);
			if (bres) {
				switch (it->align) {
				case 1:
					while (NextDiff<unsigned __int8>(orig_buf + offset, buf, roffset, size, diff_offset, diff_size)) {
						RemoveRange(it, pelem, (offset + diff_offset), diff_size);
					}
					break;
				case 2:
					//set zero-end elem
					diff_size = size % sizeof(unsigned __int16);
					if (diff_size > 0) {
						memcpy(buf + size, orig_buf + offset + size, sizeof(unsigned __int16) - diff_size);
					}

					size = Alignment32(size, sizeof(unsigned __int16));
					while (NextDiff<unsigned __int16>(orig_buf + offset, buf, roffset, size, diff_offset, diff_size)) {
						RemoveRange(it, pelem, (offset + diff_offset), diff_size);
					}
					break;
				case 4:
					//set zero-end elem
					diff_size = size % sizeof(unsigned __int32);
					if (diff_size > 0) {
						memcpy(buf + size, orig_buf + offset + size, sizeof(unsigned __int32) - diff_size);
					}

					size = Alignment32(size, sizeof(unsigned __int32));
					while (NextDiff<unsigned __int32>(orig_buf + offset, buf, roffset, size, diff_offset, diff_size)) {
						RemoveRange(it, pelem, (offset + diff_offset), diff_size);
					}
					break;
				case 8:
					//set zero-end elem
					diff_size = size % sizeof(unsigned __int64);
					if (diff_size > 0) {
						memcpy(buf + size, orig_buf + offset + size, sizeof(unsigned __int64) - diff_size);
					}

					size = Alignment32(size, sizeof(unsigned __int64));
					while (NextDiff<unsigned __int64>(orig_buf + offset, buf, roffset, size, diff_offset, diff_size)) {
						RemoveRange(it, pelem, (offset + diff_offset), diff_size);
					}
					break;
				default:
					VirtualFree(buf, 0, MEM_RELEASE);
					return -1;
				}
			}
		}
		if (!pelem) {
			break;
		}

		pelem = pelem->next;
	}

	VirtualFree(buf, 0, MEM_RELEASE);
	return 0;
}

void CPECompare::CompareClear()
{
	_active_enum = false;
}

bool CPECompare::EnumRanges(DWORD offset, enum_ranges_proc callback, void *param)
{
	list<Range_Header>::iterator it = _ranges.begin();
	Range_Elem *pelem;

	if (!_loaded || _active_enum) {
		return false;
	}

	while (it != _ranges.end()) {
		if (it->offset == offset) {
			pelem = it->first;

			while (pelem) {
				if (!callback(it->offset + pelem->offset, pelem->size, param)) {
					return false;
				}
				pelem = pelem->next;
			}
		}
		it++;
	}

	return true;
}

bool CPECompare::EnumDiffRanges(DWORD offset, enum_ranges_proc callback, void *param)
{
	list<Range_Header>::iterator it = _ranges.begin();
	Range_Elem *pelem;
	DWORD roffset;

	if (!_loaded || _active_enum) {
		return false;
	}

	while (it != _ranges.end()) {
		if (it->offset == offset) {
			roffset = 0;

			pelem = it->first;
			while (pelem) {
				if (roffset != pelem->offset) {
					if (!callback(it->offset + roffset, pelem->offset - roffset, param)) {
						return false;
					}
					roffset = pelem->offset + pelem->size;
				} else {
					roffset += pelem->size;
				}

				pelem = pelem->next;
			}
			if (roffset < it->size - 1) {
				if (!callback(it->offset + roffset, Alignment32(it->size - roffset, it->align), param)) {
					return false;
				}
			}
		}
		it++;
	}

	return true;
}

DWORD CPECompare::GetFirstDiffRange(unsigned int *psize)
{
	_enum_it = _ranges.begin();
	return GetNextDiffRange(psize);
}

DWORD CPECompare::GetNextDiffRange(unsigned int *psize)
{
	DWORD offset;
	if (_enum_it == _ranges.end()) {
		return -1L;
	}

	offset = _enum_it->offset;
	*psize = _enum_it->size;
	_enum_it++;

	return offset;
}

unsigned int CPECompare::GetDiffCount()
{
	unsigned int count;
	if (!_loaded) {
		return 0;
	}
	EnterCriticalSection(&_csect);
	count = _diff_count;
	LeaveCriticalSection(&_csect);
	return _diff_count;
}

void CPECompare::RemoveList(Range_Header *phead)
{
	Range_Elem *pelem, *ptemp;
	if (!phead->first) {
		return;
	}

	pelem = phead->first;
	while (pelem) {
		ptemp = pelem;
		pelem = pelem->next;
		delete ptemp;
	}
}

void CPECompare::RemoveRange(list<Range_Header>::iterator &it, Range_Elem *&pelem, DWORD offset, unsigned int size)
{
	Range_Elem *ptemp;

	if (pelem->offset == offset) {
		if (pelem->size <= size) {
			//full delete
			ptemp = pelem;

			if (pelem->prev) {
				pelem->prev->next = pelem->next;
			} else {
				it->first = pelem->next;
			}

			if (pelem->next) {
				pelem->next->prev = pelem->prev;
				pelem = pelem->next;
			} else {
				pelem = NULL;
			}

			delete ptemp;
		} else {
			//shift and shink
			pelem->offset += size;
			pelem->size -= size;
		}
	} else if (pelem->offset + pelem->size <= offset + size) {
		pelem->size = offset - pelem->offset;
	} else {
		ptemp = new Range_Elem;
		ptemp->prev = pelem;
		ptemp->next = pelem->next;

		ptemp->size = (pelem->offset + pelem->size) - (offset + size);
		ptemp->offset = offset + size;

		pelem->size = offset - pelem->offset;
		pelem->next = ptemp;
		pelem = ptemp;
	}
}

template <typename T>
bool CPECompare::NextDiff(char *src, char *dest, DWORD &offset, unsigned int &size, DWORD &diff_offset, unsigned int &diff_size)
{
	T *pval_src = (T *)src,
		*pval_dest = (T *)dest;
	unsigned int count = size / sizeof(T);
	bool diff_found = false;

	for (unsigned int i = offset / sizeof(T); i < count; i++) {
		if (pval_src[i] != pval_dest[i]) {
			if (!diff_found) {
				diff_offset = i * sizeof(T);
				diff_size = 0;
				diff_found = true;

				EnterCriticalSection(&_csect);
				_diff_count++;
				LeaveCriticalSection(&_csect);
			}
			diff_size += sizeof(T);
		} else if (diff_found) {
			break;
		}
	}

	if (diff_found) {
		offset = diff_offset + diff_size;
	}

	return diff_found;
}
