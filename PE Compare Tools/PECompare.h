#pragma once

#include "src/PEManagerFile.h"
#include "src/PESandbox.h"
#include <Windows.h>
#include <list>


typedef bool (*enum_ranges_proc)(DWORD offset, unsigned int size, void *param);

class CPECompare {
private:
	struct Range_Elem {
		Range_Elem *next;
		Range_Elem *prev;
		DWORD offset;
		unsigned int size;
	};

	struct Range_Header {
		Range_Elem *first;
		DWORD offset;
		unsigned int align;
		unsigned int size;
	};

	CRITICAL_SECTION _csect;
	CRITICAL_SECTION _csect_io;

	bool _loaded;

	CPESandbox *_sbox;
	IPEManager *_mngr;

	std::list<Range_Header> _ranges;

	bool _active_enum;
	std::list<Range_Header>::iterator _enum_it;
	Range_Elem *_enum_elem;

	unsigned int _segment;

	unsigned int _diff_count;

	void RemoveList(Range_Header *phead);
	void RemoveRange(std::list<Range_Header>::iterator &it, Range_Elem *&pelem, DWORD offset, unsigned int size);

	template <typename T>
	inline bool NextDiff(char *src, char *dest, DWORD &offset, unsigned int &size, DWORD &diff_ofst, unsigned int &diff_size);

public:
	CPECompare();
	~CPECompare();

	bool Init(CPESandbox *src_sbox, IPEManager *dest_mngr, unsigned int segment_size = 0x10000);
	void Clear();

	bool AddRange(DWORD offset, unsigned int size, unsigned int align);
	bool DeleteRange(DWORD offset);
	void DeleteAllRanges();

	//Mt-safe
	bool CompareInit();
	int CompareNext();
	void CompareClear();

	bool EnumRanges(DWORD offset, enum_ranges_proc callback, void *param);
	bool EnumDiffRanges(DWORD offset, enum_ranges_proc callback, void *param);

	DWORD GetFirstDiffRange(unsigned int *psize);
	DWORD GetNextDiffRange(unsigned int *psize);

	unsigned int GetDiffCount();
};