#ifndef __PEMANAGER_H
#define __PEMANAGER_H

#include "PEInfo.h"
#include "PEDirRelocs.h"
#include "CacheMapping.h"


//PE I\O Manager Interface
class IPEManager : public CPEInfo {
protected:
	bool _opened;

	bool _readonly;
	bool _runtime_object;

	bool _mode_relocs;
	unsigned int _module_size;

	UAddress _imgbase;
	UAddress _rel_imgbase;

	CPEDirRelocs _rels;

//Virtual interface for native managers
private:
	/*Open PE object*/
	virtual bool OpenObject(void *handle) = 0;
	/*Close PE object*/
	virtual void CloseObject() = 0;

	/*Reloading header from PE object*/
	virtual bool ReloadObjectHeader() = 0;
	/*Read PE header to buffer from PE object*/
	virtual bool ReadObjectHeaderData(void *pbuffer, unsigned int buf_size, unsigned int *readed) = 0;
	/*Write PE header from buffer to PE object*/
	virtual bool WriteObjectHeader(void *pbuffer = NULL, unsigned int  size = PE_HEADER_RAW_SIZE) = 0;

	/*Read raw data from PE object*/
	virtual bool ReadObjectRawData(DWORD roffset, void *pbuffer, unsigned int  size) = 0;//don't work for virtual managers
	/*Write raw data to PE object*/
	virtual bool WriteObjectRawData(DWORD roffset, void *pbuffer, unsigned int  size) = 0;//don't work for virtual managers

	/*Read raw data from PE object*/
	virtual bool ReadObjectVirtualData(DWORD voffset, void *pbuffer, unsigned int  size) = 0;
	/*Write raw data from PE object*/
	virtual bool WriteObjectVirtualData(DWORD voffset, void *pbuffer, unsigned int  size) = 0;

	/*Reload relocs from PE object*/
	virtual bool ReloadObjectRelocs() = 0;

//Manager core
public:
	IPEManager();
	virtual ~IPEManager();

//Main
	/*Open PE object by specific handle*/
	bool Open(void *handle, bool use_relocs, bool readonly = false);
	/*Close PE object*/
	void Close();
	/*Check opened state*/
	inline bool IsOpened();
	bool IsReadOnly();
	bool IsRuntimeObject();

	/*Reloading header from PE object*/
	bool ReloadHeader();
	/*Read PE header to buffer from PE object*/
	bool ReadHeaderData(void *pbuffer, unsigned int buf_size, unsigned int *readed, bool use_relocs = false);
	/*Write PE header from buffer to PE object*/
	bool WriteHeader(void *pbuffer = NULL, unsigned int  size = PE_HEADER_RAW_SIZE, bool use_relocs = false);

	/*Read raw data from PE object*/
	bool ReadRawData(DWORD roffset, void *pbuffer, unsigned int  size, bool use_relocs = false);//don't work for virtual managers
	/*Write raw data to PE object*/
	bool WriteRawData(DWORD roffset, void *pbuffer, unsigned int  size, bool use_relocs = false);//don't work for virtual managers

	/*Read raw data from PE object*/
	bool ReadVirtualData(DWORD voffset, void *pbuffer, unsigned int  size, bool use_relocs = false);
	/*Write raw data from PE object*/
	bool WriteVirtualData(DWORD voffset, void *pbuffer, unsigned int  size, bool use_relocs = false);

	/*Reload relocs from PE object*/
	bool ReloadRelocs();

	/*Get relocs mode state*/
	inline bool GetRelocsModeState();
	/*Set relocs mode state*/
	bool SetRelocsModeState(bool enable);
	/*Get current imagebase for relocs mode*/
	bool GetRelocsBase(UAddress *pbase);
	/*Set new imagebase for relocs*/
	bool SetRelocsBase(UAddress *pbase);

	/*Get module raw size*/
	unsigned int GetSize();

private:
	void CloseManager();
	inline bool CommitRelocs(void *pbuffer, unsigned int  size, DWORD offset, bool offset_raw, bool rel_to_img);
};

#endif