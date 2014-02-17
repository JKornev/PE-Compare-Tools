#ifndef __PEINFO_H
#define __PEINFO_H

#include "PEDefs.h"
#include "ErrorHandler.h"


enum PE_Architecture {
	PE_32,//PE
	PE_64,//PE+
	PE_ROM,//not support
	PE_UNK
};

enum PE_Mem_Map {
	PE_MAP_HEADER,
	PE_MAP_SECTOR,
	PE_MAP_OUT_OF_RANGE
};

#define PE_INVALID_SECTOR -1

class CPEInfo : public CErrorCtrl {
private:
	bool _loaded_flag;

	PE_Architecture _arch;

	// PE structures
	PIMAGE_DOS_HEADER _pdos;
	PIMAGE_FILE_HEADER _pimg;
	PIMAGE_OPTIONAL_HEADER32 _popt32;
	PIMAGE_OPTIONAL_HEADER64 _popt64;
	PIMAGE_DATA_DIRECTORY _pdir;
	PIMAGE_SECTION_HEADER _psects;
	unsigned int _sect_count;

	DWORD _virt_aligm;
	DWORD _raw_aligm;

	unsigned int _virt_peak;
	unsigned int _raw_peak;

	template<typename T>
	inline unsigned int CalcRawPeak(T popt);

public:
	CPEInfo();
	~CPEInfo();

	bool ParseHeader(void *buffer);
	bool HeaderIsLoaded();

	// Information
	PE_Architecture GetArch();

	PIMAGE_DOS_HEADER GetHDos();
	PIMAGE_FILE_HEADER GetHImg();
	PIMAGE_OPTIONAL_HEADER32 GetHOpt32();
	PIMAGE_OPTIONAL_HEADER64 GetHOpt64();
	PIMAGE_DATA_DIRECTORY GetHDataDir();
	PIMAGE_SECTION_HEADER GetSectorPtr(unsigned int *pcount);
	PIMAGE_SECTION_HEADER GetSectorByPos(int pos);

	DWORD GetVirtualAligment();
	DWORD GetRawAligment();

	unsigned int GetPeakVirtualSize();
	unsigned int GetPeakRawFileSize();

	// Section
	int FindSectorPosByName(char *pname);
	int FindSectorPosByVirtual(DWORD voffset);
	int FindSectorPosByRaw(DWORD roffset);

	// Convert
	PE_Mem_Map ConvRawToVirtual(DWORD roffset, DWORD *pvoffset, unsigned int *pblock_size = NULL);
	PE_Mem_Map ConvVirtualToRaw(DWORD voffset, DWORD *proffset, unsigned int *pblock_size = NULL);

	PE_Mem_Map GetRawBlockSize(DWORD roffset, unsigned int *pblock_size);
	PE_Mem_Map GetVirtualBlockSize(DWORD voffset, unsigned int *pblock_size);
};

#endif