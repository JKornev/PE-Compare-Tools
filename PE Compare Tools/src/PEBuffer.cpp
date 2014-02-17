#include "stdafx.h"
#include "PEBuffer.h"
#include <list>


CPEBuffer::CPEBuffer(IPEManager *pmngr)
{
	manager = pmngr;
}

CPEBuffer::~CPEBuffer()
{
	FreeAllBlocks();
}

void *CPEBuffer::GetRawDataBlock(DWORD roffset, unsigned int size)
{
	if (!size) {
		return NULL;
	}

	void *buf = (void *)malloc(size);
	if (!buf) {
		return NULL;
	}

	if (!manager->ReadRawData(roffset, buf, size)) {
		free(buf);
		return NULL;
	}
	_buf_set.insert(buf);
	return buf;
}

void *CPEBuffer::GetVirtualDataBlock(DWORD voffset, unsigned int size)
{
	if (!size) {
		return NULL;
	}

	void *buf = (void *)malloc(size);
	if (!buf) {
		return NULL;
	}

	if (!manager->ReadVirtualData(voffset, buf, size)) {
		free(buf);
		return NULL;
	}
	_buf_set.insert(buf);
	return buf;
}

bool CPEBuffer::FreeDataBlock(void *pbuf)
{
	if (_buf_set.find(pbuf) == _buf_set.end()) {
		return false;
	}
	_buf_set.erase(pbuf);
	free(pbuf);
	return true;
}

void CPEBuffer::FreeAllBlocks()
{
	std::set<void *>::iterator it = _buf_set.begin();
	while (it != _buf_set.end()) {
		free(*it);
		it++;
	}
	_buf_set.clear();
}