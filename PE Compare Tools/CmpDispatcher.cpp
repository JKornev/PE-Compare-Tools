#include "stdafx.h"
#include "CmpDispatcher.h"
#include "src/PEManagerFile.h"
#include "src/PEManagerRemoteVirtual.h"
#include <process.h>

using namespace std;


CCmpDispatcher::CCmpDispatcher() :
	_base(NULL),
	_opened(false),
	_cmp_started(false),
	_thr_work(false),
	_guid(0),
	_diff_start_time(0),
	_diff_count(0)
{
	_scan_vect.reserve(20);
	_scan_align.reserve(20);
	InitializeCriticalSection(&_csect);
}

CCmpDispatcher::~CCmpDispatcher()
{
	StopCompare();
	//ClearResults();
	CloseBaseApp();
	DeleteCriticalSection(&_csect);
}

bool CCmpDispatcher::OpenBaseApp(LPCWSTR name)
{
	unsigned int count = 0;

	if (_opened || _cmp_started) {
		return false;
	}

	_base = new CPEManagerFile();

	if (!_base->Open((void *)name, true, true)) {
		delete _base;
		_base = NULL;
		return false;
	}

	_base->GetSectorPtr(&count);
	_scan_vect.insert(_scan_vect.begin(), count + 1, false);
	_scan_align.insert(_scan_align.begin(), count + 1, 1);

	return _opened = true;
}

bool CCmpDispatcher::OpenBaseApp(UINT pid, HMODULE hmod)
{
	PEOpenRemoteParams params;
	unsigned int count = 0;

	if (_opened || _cmp_started) {
		return false;
	}

	_base = new CPEManagerRemoteVirtual();
	params.pid = pid;
	params.hmod = hmod;

	if (!_base->Open(&params, true, true)) {
		delete _base;
		_base = NULL;
		return false;
	}

	_base->GetSectorPtr(&count);
	_scan_vect.insert(_scan_vect.begin(), count + 1, false);
	_scan_align.insert(_scan_align.begin(), count + 1, 1);

	return _opened = true;
}

void CCmpDispatcher::CloseBaseApp()
{
	RemoveAllDiffApps();
	
	if(_cmp_started) {
		return;
	}

	if (_base) {
		delete _base;
		_base = NULL;
	}

	_scan_vect.clear();
	_scan_align.clear();
	_opened = false;
}

bool CCmpDispatcher::IsBaseAppOpened() const
{
	return _opened;
}

IPEManager *CCmpDispatcher::GetBaseInstance() const
{
	if (!_opened) {
		return NULL;
	}
	return _base;
}

bool CCmpDispatcher::SetCompareSector(unsigned int inx, bool enable)
{
	if (!_opened || inx >= _scan_vect.size()) {
		return false;
	}
	_scan_vect[inx] = enable;
	return true;
}

bool CCmpDispatcher::GetCompareSector(unsigned int inx)
{
	if (!_opened || inx >= _scan_vect.size()) {
		return false;
	}
	return _scan_vect[inx];
}

bool CCmpDispatcher::SetCompareAlign(unsigned int inx, unsigned int align)
{
	if (!_opened || inx >= _scan_align.size()) {
		return false;
	}
	_scan_align[inx] = align;
	return true;
}

unsigned int CCmpDispatcher::GetCompareAlign(unsigned int inx)
{
	if (!_opened || inx >= _scan_align.size()) {
		return 0;
	}
	return _scan_align[inx];
}

unsigned int CCmpDispatcher::AddDiffApp(LPCWSTR name)
{
	list<Cmp_Elem>::iterator it = _diff_apps.begin();
	Cmp_Elem elem;

	if (!_opened || _cmp_started) {
		return CMP_DISP_INVALID_ID;
	}

	if (FindDiffApp(name, it)) {
		return CMP_DISP_INVALID_ID;
	}

	elem.id = GetGuid();
	elem.type = CAT_FILE;
	elem.path = name;
	elem.pmngr = new CPEManagerFile();
	elem.cmp = NULL;

	if (!elem.pmngr->Open((void *)name, true, true)) {
		delete elem.pmngr;
		return CMP_DISP_INVALID_ID;
	}

	if (elem.pmngr->GetArch() != _base->GetArch()) {
		delete elem.pmngr;
		return CMP_DISP_INVALID_ID;
	}

	_diff_apps.push_back(elem);
	return elem.id;
}

unsigned int CCmpDispatcher::AddDiffApp(UINT pid, HMODULE hmod)
{
	list<Cmp_Elem>::iterator it = _diff_apps.begin();
	PEOpenRemoteParams params;
	Cmp_Elem elem;

	if (!_opened || _cmp_started) {
		return CMP_DISP_INVALID_ID;
	}

	if (FindDiffApp(pid, hmod, it)) {
		return CMP_DISP_INVALID_ID;
	}

	elem.id = GetGuid();
	elem.type = CAT_MOD;
	elem.pid = pid;
	elem.hmod = hmod;
	elem.pmngr = new CPEManagerRemoteVirtual();
	elem.cmp = NULL;

	params.pid = pid;
	params.hmod = hmod;

	if (!elem.pmngr->Open(&params, true, true)) {
		delete elem.pmngr;
		return CMP_DISP_INVALID_ID;
	}

	if (elem.pmngr->GetArch() != _base->GetArch()) {
		delete elem.pmngr;
		return CMP_DISP_INVALID_ID;
	}

	_diff_apps.push_back(elem);
	return elem.id;
}

bool CCmpDispatcher::RemoveDiffApp(unsigned int app_id)
{
	list<Cmp_Elem>::iterator it = _diff_apps.begin();

	if (!_opened || _cmp_started) {
		return false;
	}

	if (!FindDiffApp(app_id, it)) {
		return false;
	}

	if (it->cmp) {
		delete it->cmp;
	}
	delete it->pmngr;
	_diff_apps.erase(it);

	return true;
}

void CCmpDispatcher::RemoveAllDiffApps()
{
	list<Cmp_Elem>::iterator it = _diff_apps.begin();

	while (it != _diff_apps.end()) {
		if (it->cmp) {
			delete it->cmp;
		}
		delete it->pmngr;
		it++;
	}

	_diff_apps.clear();
}

bool CCmpDispatcher::IsDiffAppsOpened() const
{
	if (!_opened) {
		return false;
	}
	return (_diff_apps.size() > 0 ? true : false);
}

IPEManager *CCmpDispatcher::GetDiffInstance(unsigned int app_id)
{
	list<Cmp_Elem>::iterator it = _diff_apps.begin();

	if (!_opened) {
		return false;
	}

	if (!FindDiffApp(app_id, it)) {
		return false;
	}

	return it->pmngr;
}

bool CCmpDispatcher::StartCompare(CmpDispRepeatMode repeat_mode, unsigned int th_pool_size, unsigned int seg_size, 
	async_signal_proc callback, void *params)
{
	std::list<Cmp_Elem>::iterator it;
	PIMAGE_SECTION_HEADER psect;
	unsigned int sect_count = 0, rsize;
	HANDLE hthread;
	DWORD roffset;

	if (!_opened || _cmp_started || th_pool_size == 0 || _diff_apps.size() == 0) {
		return false;
	}

	_scan_mode = repeat_mode;
	if (_scan_mode == DRM_ONCE) {
		_async_stop = callback;
		_async_stop_params = params;
	} else {
		_async_stop = NULL;
	}

	_pool_count = th_pool_size;
	_pool_loaded = 0;
	_cycle_diff_count = 0;
	
	_hstop_sync = NULL;
	_hpool_sync = NULL;

	_sbox.ClearBuffer();
	ClearResults();

	_diff_start_time = time(NULL);
	_diff_it = _diff_apps.begin();

	psect = _base->GetSectorPtr(&sect_count);

	if (!_sbox.Attach(_base, 0, true)) {
		return false;
	}

	it = _diff_apps.begin();
	try {
		//init diff
		while (it != _diff_apps.end()) {
			if (!it->cmp) {
				it->cmp = new CPECompare();
			}
			if (!it->cmp->Init(&_sbox, it->pmngr, 0x10000)) {
				throw 1;
			}

			for (unsigned int a = 0; a < sect_count + 1; a++) {
				if (!_scan_vect[a]) {
					continue;
				}

				if (a == 0) {//header
					if (!it->cmp->AddRange(0, PE_HEADER_RAW_SIZE, _scan_align[a])) {
						throw 5;
					}
				} else {//sectors
					roffset = psect[a - 1].VirtualAddress;
					rsize = psect[a - 1].SizeOfRawData;
					if (rsize > psect[a - 1].Misc.VirtualSize) {
						rsize = psect[a - 1].Misc.VirtualSize;
					}
					if (roffset == 0 || rsize == 0) {
						continue;
					}

					if (!it->cmp->AddRange(roffset, rsize, _scan_align[a])) {
						throw 6;
					}
				}
			}
			it++;
		}

		//init sync
		_hpool_sync = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!_hpool_sync) {
			throw 2;
		}

		_hstop_sync = CreateEvent(NULL, true, false, NULL);
		if (!_hpool_sync) {
			throw 3;
		}

		//init thread pool
		for (unsigned int i = 0; i < _pool_count; i++) {
			hthread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, NULL);
			if (hthread == (HANDLE)-1L) {
				throw 4;
			}
			CloseHandle(hthread);
		}

		//start
		_thr_work = true;
		_init_next_app = true;
		SetEvent(_hpool_sync);
		ResetEvent(_hpool_sync);
		_cmp_started = true;
	} catch (int) {
		if (it != _diff_apps.end()) {
			ClearResults();
		}
		if (_hstop_sync) {
			CloseHandle(_hstop_sync);
		}
		if (_hpool_sync) {
			SetEvent(_hpool_sync);//mb not need
			CloseHandle(_hpool_sync);
		}
		_sbox.Detach();
	}

	return _cmp_started;
}

bool CCmpDispatcher::StopCompare()
{
	if (!_opened || !_cmp_started || !_thr_work) {
		return false;
	}
	_thr_work = false;

	SetEvent(_hpool_sync);
	WaitForSingleObject(_hstop_sync, INFINITE);

	_sbox.Detach();
	CloseHandle(_hstop_sync);
	CloseHandle(_hpool_sync);

	_diff_end_time = time(NULL);
	_cmp_started = false;
	return true;
}

bool CCmpDispatcher::StopCompareAsync(async_signal_proc callback, void *params)
{
	if (!_opened || !_cmp_started || !_thr_work) {
		return false;
	}

	if (callback) {
		_async_stop = callback;
		_async_stop_params = params;
	}
	_thr_work = false;

	//pcmp->CompareClear();


	SetEvent(_hpool_sync);

	return true;
}

int CCmpDispatcher::GetCompareState()
{
	if (!_opened) {
		return -1;
	}
	return (_cmp_started ? 1 : 0);
}

bool CCmpDispatcher::EnumDiffElem(enum_diff_callback callback, void *param)
{
	DWORD offset;
	unsigned int size;
	if (!_opened || _cmp_started) {
		return false;
	}

	_enum_proc = callback;
	_enum_proc_params = param;

	_enum_it = _diff_apps.begin();
	while (_enum_it != _diff_apps.end()) {
		if (!_enum_it->cmp) {
			continue;
		}
		
		offset = _enum_it->cmp->GetFirstDiffRange(&size);
		if (offset == -1) {
			return false;
		}

		do {
			if (!_enum_it->cmp->EnumDiffRanges(offset, enum_ranges, this)) {
				return false;
			}

			offset = _enum_it->cmp->GetNextDiffRange(&size);
		} while (offset != -1);

		_enum_it++;
	}

	return true;
}

unsigned int CCmpDispatcher::GetDiffCount()
{
	unsigned int count;
	if (!_opened) {
		return 0;
	}
	EnterCriticalSection(&_csect);
	count = _diff_count;
	LeaveCriticalSection(&_csect);
	return count;
}

unsigned int CCmpDispatcher::GetDiffCycles()
{
	unsigned int cycles;
	if (!_opened) {
		return 0;
	}
	EnterCriticalSection(&_csect);
	cycles = _diff_cycles;
	LeaveCriticalSection(&_csect);
	return cycles;
}

time_t CCmpDispatcher::GetScanTime()
{
	time_t end_time;
	if (!_opened) {
		return 0;
	}

	if (_diff_start_time == 0) {
		return 0;
	}

	EnterCriticalSection(&_csect);
	end_time = _diff_end_time;
	LeaveCriticalSection(&_csect);

	if (end_time == 0) {
		end_time = time(NULL);
	}

	return end_time - _diff_start_time;
}

void CCmpDispatcher::ClearResults()
{
	list<Cmp_Elem>::iterator it = _diff_apps.begin();
	while (it != _diff_apps.end()) {
		if (it->cmp) {
			it->cmp->Clear();
		}
		it++;
	}

	_diff_count = 0;
	_diff_cycles = 0;
	_diff_start_time = 0;
	_diff_end_time = 0;
}

unsigned int CCmpDispatcher::GetGuid()
{
	return _guid++;
}

bool CCmpDispatcher::FindDiffApp(unsigned int id, list<Cmp_Elem>::iterator &it)
{
	while (it != _diff_apps.end()) {
		if (it->id == id) {
			return true;
		}
		it++;
	}
	return false;
}

bool CCmpDispatcher::FindDiffApp(DWORD pid, HMODULE hmod, list<Cmp_Elem>::iterator &it)
{
	while (it != _diff_apps.end()) {
		if (it->type == CAT_MOD && it->pid == pid && it->hmod == hmod) {
			return true;
		}
		it++;
	}
	return false;
}

bool CCmpDispatcher::FindDiffApp(LPCWSTR path, list<Cmp_Elem>::iterator &it)
{
	while (it != _diff_apps.end()) {
		if (it->type == CAT_FILE && it->path == path) {
			return true;
		}
		it++;
	}
	return false;
}

void CCmpDispatcher::CloseAsyncCompare()
{
	list<Cmp_Elem>::iterator it = _diff_apps.begin();

	while (it != _diff_apps.end()) {
		it->cmp->CompareClear();
		it++;
	}

	CloseHandle(_hstop_sync);
	CloseHandle(_hpool_sync);
	_sbox.Detach();
	_diff_end_time = time(NULL);
	_cmp_started = false;
	
	_async_stop(_async_stop_params);
}

unsigned int __stdcall CCmpDispatcher::ThreadProc(void *param)
{
	CCmpDispatcher *pthis = (CCmpDispatcher *)param;
	bool last = false;

	EnterCriticalSection(&pthis->_csect);
	pthis->_pool_loaded++;
	LeaveCriticalSection(&pthis->_csect);

	WaitForSingleObject(pthis->_hpool_sync, 1000);

	while (pthis->_thr_work) {
		if (!pthis->DoThreadWork()) {
			break;
		}
	}

	EnterCriticalSection(&pthis->_csect);
	pthis->_pool_loaded--;
	if (pthis->_pool_loaded == 0) {
		last = true;
	}
	LeaveCriticalSection(&pthis->_csect);

	if (last) {
		WaitForSingleObject(pthis->_hpool_sync, INFINITE);
		SetEvent(pthis->_hstop_sync);
		if (pthis->_async_stop) {
			pthis->CloseAsyncCompare();
		}
		Sleep(1);
	}

	return 0;
}

bool CCmpDispatcher::DoThreadWork()
{
	CPECompare *pcmp;
	unsigned int curr_id;
	int res;

	EnterCriticalSection(&_csect);
	pcmp = _diff_it->cmp;
	if (_init_next_app) {
		pcmp->CompareInit();
		_init_next_app = false;
	}
	curr_id = _diff_it->id;
	//_cycle_diff_count = 0;
	LeaveCriticalSection(&_csect);
	
	res = pcmp->CompareNext();
	if (res != 1) {//next loop
		return true;
	}

	EnterCriticalSection(&_csect);
	_diff_it++;
	_init_next_app = true;

	_cycle_diff_count += pcmp->GetDiffCount();

	if (_diff_it == _diff_apps.end()) {
		_diff_cycles++;
		_diff_count = _cycle_diff_count;
		_cycle_diff_count = 0;

		if (_scan_mode == DRM_ONCE) {
			StopCompareAsync(NULL, 0);
			LeaveCriticalSection(&_csect);
			return false;
		} else {
			_diff_it = _diff_apps.begin();
		}
	}
	LeaveCriticalSection(&_csect);

	return true;
}

bool CCmpDispatcher::enum_ranges(DWORD offset, unsigned int size, void *param)
{
	CCmpDispatcher *pthis = (CCmpDispatcher *)param;
	return pthis->_enum_proc(offset, size, pthis->_enum_it->id, pthis->_enum_proc_params);
}