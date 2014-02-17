#pragma once

#include "src/PEManagerFile.h"
#include "src/PEManagerRemoteVirtual.h"
#include "src/PESandbox.h"
#include "PECompare.h"
#include <Windows.h>
#include <vector>
#include <list>

#define CMP_DISP_INVALID_ID -1

enum CmpAppType {
	CAT_FILE,
	CAT_MOD,
	CAT_NONE
};

enum CmpDispRepeatMode {
	DRM_ONCE,
	DRM_INFINITE,
};

typedef void (*async_signal_proc)(void *params);
typedef bool (*enum_diff_callback)(DWORD offset, UINT size, unsigned int id, void *params);

class CCmpDispatcher {
	typedef struct _Cmp_Elem {
		unsigned int id;
		IPEManager *pmngr;
		CmpAppType type;
		CString path;
		DWORD pid;
		HMODULE hmod;
		CPECompare *cmp;
	} Cmp_Elem, *PCmp_Elem;

	CRITICAL_SECTION _csect;

	std::list<Cmp_Elem> _diff_apps;

	IPEManager *_base;
	volatile bool _opened;
	volatile bool _cmp_started;
	volatile bool _thr_work;

	std::vector<bool> _scan_vect;
	std::vector<unsigned int> _scan_align;

	CPESandbox _sbox;

	HANDLE _hpool_sync;
	HANDLE _hstop_sync;
	unsigned int _pool_count;
	unsigned int _pool_loaded;

	std::list<Cmp_Elem>::iterator _diff_it;

	unsigned int _diff_count;
	unsigned int _diff_cycles;
	time_t _diff_start_time;
	time_t _diff_end_time;

	async_signal_proc _async_stop;
	void *_async_stop_params;

	std::list<Cmp_Elem>::iterator _enum_it;
	enum_diff_callback _enum_proc;
	void *_enum_proc_params;

	unsigned int _guid;
	unsigned int GetGuid();

	bool FindDiffApp(unsigned int id, std::list<Cmp_Elem>::iterator &it);
	bool FindDiffApp(DWORD pid, HMODULE hmod, std::list<Cmp_Elem>::iterator &it);
	bool FindDiffApp(LPCWSTR path, std::list<Cmp_Elem>::iterator &it);

	void CloseAsyncCompare();

	CmpDispRepeatMode _scan_mode;
	bool _init_next_app;
	unsigned int _cycle_diff_count;

	static unsigned int __stdcall ThreadProc(void *param);
	inline bool DoThreadWork();

	static bool enum_ranges(DWORD offset, unsigned int size, void *param);

public:
	CCmpDispatcher();
	~CCmpDispatcher();

	bool OpenBaseApp(LPCWSTR name);
	bool OpenBaseApp(UINT pid, HMODULE hmod);
	void CloseBaseApp();
	bool IsBaseAppOpened() const;

	IPEManager *GetBaseInstance() const;

	bool SetCompareSector(unsigned int inx, bool enable);
	bool GetCompareSector(unsigned int inx);
	bool SetCompareAlign(unsigned int inx, unsigned int align);
	unsigned int GetCompareAlign(unsigned int inx);

	unsigned int AddDiffApp(LPCWSTR name);
	unsigned int AddDiffApp(UINT pid, HMODULE hmod);
	bool RemoveDiffApp(unsigned int app_id);
	void RemoveAllDiffApps();
	bool IsDiffAppsOpened() const;

	IPEManager *GetDiffInstance(unsigned int app_id);

	bool StartCompare(CmpDispRepeatMode repeat_mode, unsigned int th_pool_size, unsigned int seg_size, 
		async_signal_proc callback, void *params);
	bool StopCompare();
	bool StopCompareAsync(async_signal_proc callback, void *params);
	int GetCompareState();

	bool EnumDiffElem(enum_diff_callback callback, void *param);

	unsigned int GetDiffCount();
	unsigned int GetDiffCycles();
	time_t GetScanTime();

	void ClearResults();

};