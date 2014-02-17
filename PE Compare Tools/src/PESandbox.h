#ifndef __PESANDBOX_H
#define __PESANDBOX_H

#include "PEDefs.h"
#include "PEInfo.h"
#include "PEManager.h"
#include "PEDirRelocs.h"
#include "CacheMapping.h"


class CPESandbox : protected CCacheMapping {
private:
	bool _attached;
	bool _use_relocs;

	unsigned int _sandbox_size;

	IPEManager *_pmngr;

	static bool DataLoadingProc(DWORD offset, void *buffer, unsigned int size, void *param);
	static bool DataUnloadingProc(DWORD offset, void *buffer, unsigned int size, void *param);

public:
	CPESandbox();
	~CPESandbox();

	bool Attach(IPEManager *pmngr, unsigned int size, bool use_relocs);
	void Detach();
	bool IsAttached();

	unsigned int GetSize();
	bool SetSize(unsigned int size);

	//bool ReloadHeader();
	//bool ReloadRelocs(void *pbuffer = NULL, unsigned int size = 0);

	char *GetRawDataPtr(DWORD roffset, unsigned int size);
	char *GetVirtualDataPtr(DWORD voffset, unsigned int size);

	bool FlushBuffer();
	bool ClearBuffer();
};

#endif