#include "stdafx.h"
#include "CacheMapping.h"


// ======================= CCacheSandbox :: PUBLIC =======================

CCacheMapping::CCacheMapping() : 
	_is_allocated(false), 
	_callback_load(NULL),
	_callback_unload(NULL)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	_aligm = si.dwPageSize;
}

CCacheMapping::~CCacheMapping()
{
	DestroyRegion();
}

bool CCacheMapping::AllocRegion(unsigned int size)
{
	if (_is_allocated) {
		return false;
	}

	_pages_peak = _pages = Pages(size, _aligm);

	_region_map.clear();
	_region_map.reserve(_pages);
	_region_map.insert(_region_map.begin(), _pages, false);

	_region_base = VirtualAlloc(NULL, _aligm * _pages, MEM_RESERVE, PAGE_READWRITE);
	if (!_region_base) {
		return false;
	}

	return _is_allocated = true;
}

bool CCacheMapping::ReallocRegion(unsigned int size)
{
	unsigned int new_pages;
	void *region_ptr, *page_ptr, *dest_ptr, *src_ptr;
	if (!_is_allocated) {
		return false;
	}

	new_pages = Pages(size, _aligm);
	if (new_pages == _pages) {
		return true;
	} 
	
	if (new_pages > _pages_peak) {//realloc and copy
		region_ptr = VirtualAlloc(NULL, _aligm * new_pages, MEM_RESERVE, PAGE_READWRITE);
		if (!region_ptr) {
			return false;
		}
		//copy pages
		for (unsigned int i = 0; i < _pages; i++) {
			if (!_region_map[i]) {
				continue;
			}

			dest_ptr = (void *)((uintptr_t)region_ptr + (i * _aligm));
			src_ptr = (void *)((uintptr_t)_region_base + (i * _aligm));

			page_ptr = VirtualAlloc(dest_ptr, _aligm, MEM_COMMIT, PAGE_READWRITE);
			if (!page_ptr) {
				VirtualFree(region_ptr, 0, MEM_RELEASE);
				return false;
			}

			memcpy(dest_ptr, src_ptr, _aligm);
		}

		VirtualFree(_region_base, 0, MEM_RELEASE);
		_region_base = region_ptr;
		_pages_peak = _pages;
	}

	if (new_pages < _pages) {//free some pages
		for (unsigned int i = new_pages; i < _pages; i++) {
			if (!_region_map[i]) {
				continue;
			}
			src_ptr = (void *)((uintptr_t)_region_base + (i * _aligm));
			VirtualFree(src_ptr, _aligm, MEM_DECOMMIT);
		}
	}

	_region_map.resize(new_pages, false);
	_pages = new_pages;
 
	return true;
}

void CCacheMapping::DestroyRegion()
{
	if (!_is_allocated) {
		return;
	}
	_callback_load = NULL;
	_callback_unload = NULL;
	_region_map.clear();
	VirtualFree(_region_base, 0, MEM_RELEASE);
	_is_allocated = false;
}

bool CCacheMapping::RegDataLoadingCallback(_page_walk_callback callback, void *param)
{
	if (!_is_allocated) {
		return false;
	}
	_callback_load = callback;
	_callback_load_param = param;
	return true;
}

bool CCacheMapping::RegDataUnloadingCallback(_page_walk_callback callback, void *param)
{
	if (!_is_allocated) {
		return false;
	}
	_callback_unload = callback;
	_callback_unload_param = param;
	return true;
}

unsigned int CCacheMapping::GetRegionAligment()
{
	if (!_is_allocated) {
		return 0;
	}
	return _aligm;
}

unsigned int CCacheMapping::GetRegionSize()
{
	if (!_is_allocated) {
		return 0;
	}
	return _pages * _aligm;
}

unsigned int CCacheMapping::GetRegionPeakSize()
{
	if (!_is_allocated) {
		return 0;
	}
	return _pages_peak * _aligm;
}

bool CCacheMapping::AssignPages(DWORD offset, unsigned int size, void *buf)
{
	unsigned int start_page, end_page;
	void *ptr;

	if (!_is_allocated) {
		return false;
	}

	start_page = PagesToLow(offset, _aligm);
	end_page = Pages(offset + size, _aligm);
	if (!size || start_page >= _pages || end_page > _pages) {//out of range
		return false;
	}

	for (unsigned int i = start_page; i < end_page; i++) {
		if (_region_map[i]) {
			continue;
		}
		ptr = VirtualAlloc((void *)((uintptr_t)_region_base + (i * _aligm)), _aligm, MEM_COMMIT, PAGE_READWRITE);
		if (!ptr) {
			return false;
		}
		_region_map[i] = true;
	}

	return true;
}

bool CCacheMapping::AssignAllPages()
{
	void *ptr;

	if (!_is_allocated) {
		return false;
	}
	
	for (unsigned int i = 0; i < _pages; i++) {
		if (_region_map[i]) {
			continue;
		}
		ptr = VirtualAlloc((void *)((uintptr_t)_region_base + (i * _aligm)), _aligm, MEM_COMMIT, PAGE_READWRITE);
		if (!ptr) {
			return false;
		}
		_region_map[i] = true;
	}

	return true;
}

bool CCacheMapping::UnassignPages(DWORD offset, unsigned int size)
{
	unsigned int start_page, end_page;

	if (!_is_allocated) {
		return false;
	}

	start_page = PagesToLow(offset, _aligm);
	end_page = Pages(offset + size, _aligm);
	if (!size || start_page >= _pages || end_page > _pages) {//out of range
		return false;
	}

	for (unsigned int i = start_page; i < end_page; i++) {
		if (!_region_map[i]) {
			continue;
		}
		VirtualFree((void *)((uintptr_t)_region_base + (i * _aligm)), _aligm, MEM_DECOMMIT);
		_region_map[i] = false;
	}

	return true;
}

bool CCacheMapping::UnassignAllPages()
{
	if (!_is_allocated) {
		return false;
	}
	
	for (unsigned int i = 0; i < _pages; i++) {
		if (!_region_map[i]) {
			continue;
		}
		VirtualFree((void *)((uintptr_t)_region_base + (i * _aligm)), _aligm, MEM_DECOMMIT);
		_region_map[i] = false;
	}

	return true;
}

void *CCacheMapping::GetMappedData(DWORD offset, unsigned int size)
{
	if (!_is_allocated) {
		return false;
	}
	return GetAndCheckMappedData(offset, size);
}

void *CCacheMapping::GetAllMappedData()
{
	if (!_is_allocated) {
		return false;
	}
	return GetAndCheckMappedData(0, _aligm * _pages);
}

void *CCacheMapping::RenewMappedData(DWORD offset, unsigned int size)
{
	if (!_is_allocated) {
		return false;
	}
	return GetAndRenewMappedData(offset, size);
}

void *CCacheMapping::RenewAllMappedData()
{
	if (!_is_allocated) {
		return false;
	}
	return GetAndRenewMappedData(0, _aligm * _pages);
}

bool CCacheMapping::FlushMappedData(DWORD offset, unsigned int size)
{
	if (!_is_allocated) {
		return false;
	}
	return FlushAndCheckMappedData(offset, size);
}

bool CCacheMapping::FlushAllMappedData()
{
	if (!_is_allocated) {
		return false;
	}
	return FlushAndCheckMappedData(0, _aligm * _pages);
}

// ======================= CCacheSandbox :: PRIVATE =======================

void *CCacheMapping::GetAndCheckMappedData(DWORD offset, unsigned int size)
{
	unsigned int start_page, 
		end_page,
		callback_offset;
	void *callback_buf;
	char rel_fix_buf[sizeof(ULONGLONG) * 2];

	if (!_callback_load_param) {
		return NULL;
	}

	start_page = PagesToLow(offset, _aligm);
	end_page = Pages(offset + size, _aligm);
	if (!size || start_page >= _pages || end_page > _pages) {//out of range
		return NULL;
	}

	for (unsigned int i = start_page, last_inx = end_page - 1; i < end_page; i++) {
		callback_offset = i * _aligm;
		callback_buf = (void *)((uintptr_t)_region_base + callback_offset);

		if (!_region_map[i]) {
			callback_buf = VirtualAlloc(callback_buf, _aligm, MEM_COMMIT, PAGE_READWRITE);
			if (!callback_buf) {
				return NULL;
			}
			_region_map[i] = true;
		} else {
			continue;
		}

		if (!_callback_load(callback_offset, callback_buf, _aligm, _callback_load_param)) {
			return NULL;
		}
		//relocs fix
		if (_callback_load(callback_offset + _aligm - sizeof(ULONGLONG), rel_fix_buf, sizeof(rel_fix_buf), _callback_load_param)) {
			memcpy((void *)((uintptr_t)callback_buf + _aligm - sizeof(ULONGLONG)), rel_fix_buf, sizeof(ULONGLONG));
		}
	}

	return (void *)((uintptr_t)_region_base + offset);
}

void *CCacheMapping::GetAndRenewMappedData(DWORD offset, unsigned int size)
{
	unsigned int start_page, 
		end_page,
		diff, 
		callback_offset, 
		callback_size,
		base_offset;
	void *callback_buf;

	if (!_callback_load_param) {
		return NULL;
	}

	start_page = PagesToLow(offset, _aligm);
	end_page = Pages(offset + size, _aligm);
	if (!size || start_page >= _pages || end_page > _pages) {//out of range
		return NULL;
	}

	for (unsigned int i = start_page, last_inx = end_page - 1; i < end_page; i++) {
		base_offset = (i * _aligm);
		if (i == start_page) {//first
			diff = offset - base_offset;
			callback_offset = offset;
			callback_size = _aligm - diff;
			if (size < callback_size) {
				callback_size = size;
			}
			callback_buf = (void *)((uintptr_t)_region_base + base_offset + diff);
		} else if (i == last_inx) {//last
			diff = (end_page * _aligm) - offset - size;
			callback_offset = i * _aligm;
			callback_size = _aligm - diff;
			callback_buf = (void *)((uintptr_t)_region_base + base_offset);
		} else {//middle
			callback_offset = i * _aligm;
			callback_size = _aligm;
			callback_buf = (void *)((uintptr_t)_region_base + base_offset);
		}

		if (!_region_map[i]) {
			callback_buf = VirtualAlloc((void *)((uintptr_t)_region_base + base_offset), _aligm, MEM_COMMIT, PAGE_READWRITE);
			if (!callback_buf) {
				return NULL;
			}
			_region_map[i] = true;
		}

		if (!_callback_load(callback_offset, callback_buf, callback_size, _callback_load_param)) {
			return NULL;
		}
	}

	return (void *)((uintptr_t)_region_base + offset);
}

bool CCacheMapping::FlushAndCheckMappedData(DWORD offset, unsigned int size)
{
	unsigned int start_page, 
		end_page,
		diff, 
		callback_offset, 
		callback_size;
	void *callback_buf;

	if (!_callback_unload) {
		return false;
	}

	start_page = PagesToLow(offset, _aligm);
	end_page = Pages(offset + size, _aligm);
	if (!size || start_page >= _pages || end_page > _pages) {//out of range
		return false;
	}

	for (unsigned int i = start_page, last_inx = end_page - 1, finished = 0; i < end_page && !finished; i++) {
		if (!_region_map[i]) {
			continue;
		}

		if (i == start_page) {//first
			diff = offset - (i * _aligm);
			callback_offset = offset;
			callback_size = _aligm - diff;
			if (size < callback_size) {
				callback_size = size;
			}
			callback_buf = (void *)((uintptr_t)_region_base + (i * _aligm) + diff);
		} else if (i == last_inx) {//last
			diff = (end_page * _aligm) - offset - size;
			callback_offset = i * _aligm;
			callback_size = _aligm - diff;
			callback_buf = (void *)((uintptr_t)_region_base + (i * _aligm));
		} else {//middle
			callback_offset = i * _aligm;
			callback_size = _aligm;
			callback_buf = (void *)((uintptr_t)_region_base + (i * _aligm));
		}

		if (!_callback_unload(callback_offset, callback_buf, callback_size, _callback_unload_param)) {
			finished = 1;
		}

		if (callback_size == _aligm) {
			VirtualFree(callback_buf, _aligm, MEM_DECOMMIT);
			_region_map[i] = false;
		}
	}
	return true;
}