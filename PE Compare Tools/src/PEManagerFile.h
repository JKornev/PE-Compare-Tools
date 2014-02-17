#ifndef __PEMANAGERFILE_H
#define __PEMANAGERFILE_H

#include "PEDefs.h"
#include "PEInfo.h"
#include "PEManager.h"
#include "PEDirRelocs.h"


class CPEManagerFile : public IPEManager {
protected:
	enum { HeaderSize = PE_HEADER_SIZE };

	HANDLE _hfile;
	void *_header_buf;

public:
	CPEManagerFile();
	~CPEManagerFile();

protected:
	bool OpenObject(void *handle);
	void CloseObject();

	bool ReloadObjectHeader();
	bool ReadObjectHeaderData(void *pbuffer, unsigned int buf_size, unsigned int *readed);
	bool WriteObjectHeader(void *pbuffer = NULL, unsigned int  size = PE_HEADER_RAW_SIZE);

	bool ReadObjectRawData(DWORD roffset, void *pbuffer, unsigned int  size);
	bool WriteObjectRawData(DWORD roffset, void *pbuffer, unsigned int  size);

	bool ReadObjectVirtualData(DWORD voffset, void *pbuffer, unsigned int  size);
	bool WriteObjectVirtualData(DWORD voffset, void *pbuffer, unsigned int  size);

	bool ReloadObjectRelocs();
};

#endif