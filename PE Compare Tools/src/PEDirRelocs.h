#ifndef __PEDIRRELOCS_H
#define __PEDIRRELOCS_H

#include "PEDefs.h"
#include "PEInfo.h"
//#include "PEManager.h"
#include <Windows.h>
#include <map>

//Based relocation types
//#define IMAGE_REL_BASED_ABSOLUTE              0
//#define IMAGE_REL_BASED_HIGH                  1
//#define IMAGE_REL_BASED_LOW                   2
//#define IMAGE_REL_BASED_HIGHLOW               3
//#define IMAGE_REL_BASED_HIGHADJ               4
//#define IMAGE_REL_BASED_MIPS_JMPADDR          5
//#define IMAGE_REL_BASED_IA64_IMM64            9
//#define IMAGE_REL_BASED_DIR64                 10

enum PE_Rels_Type {
	Rel_Absolute = IMAGE_REL_BASED_ABSOLUTE,
	Rel_HighLow = IMAGE_REL_BASED_HIGHLOW,
	Rel_Dir64 = IMAGE_REL_BASED_DIR64,
};

class IPEManager;//fix

class CPEDirRelocs {
private:
	enum { RelBlockAligm = 0x1000 };

	std::map<DWORD, PE_Rels_Type> _rels;

	template<typename T>
	inline unsigned int CommitRelocs(void *pbuf, unsigned int size, DWORD offset, T old_imgbase, T new_imgbase);

public:
	CPEDirRelocs();
	~CPEDirRelocs();

	bool LoadDir(IPEManager *pmngr);
	//bool LoadDirCached(IPEManager *pmngr);
	bool LoadDirBuffer(void *pbuf, unsigned int size);

	bool AddRel(DWORD offset, PE_Rels_Type type);
	bool RemoveRel(DWORD offset);
	void RemoveRange(DWORD offset, unsigned int range);
	void RemoveAll();

	unsigned int GetCount();

	unsigned int GetDirSize();
	bool BuildDir(void *pbuf, unsigned int size, unsigned int *pdir_size);

	unsigned int Commit32(void *pbuf, unsigned int size, DWORD offset, DWORD old_imgbase, DWORD new_imgbase);
	unsigned int Commit64(void *pbuf, unsigned int size, DWORD offset, ULONGLONG old_imgbase, ULONGLONG new_imgbase);
};

#endif