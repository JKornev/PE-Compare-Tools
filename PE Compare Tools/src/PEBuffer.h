#ifndef __PEBUFFER_H
#define __PEBUFFER_H

#include "PEDefs.h"
#include "PEManager.h"
#include <Windows.h>
#include <set>


class CPEBuffer {
private:
	IPEManager *manager;
	std::set<void *> _buf_set;

public:
	CPEBuffer(IPEManager *);
	~CPEBuffer();

	void *GetRawDataBlock(DWORD roffset, unsigned int size);
	void *GetVirtualDataBlock(DWORD voffset, unsigned int size);

	bool FreeDataBlock(void *pbuf);
	void FreeAllBlocks();
};

#endif