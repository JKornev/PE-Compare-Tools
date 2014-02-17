#ifndef __PECACHEMNGR_H
#define __PECACHEMNGR_H

#include "PEDefs.h"
#include <Windows.h>
#include <vector>


typedef bool (*_page_walk_callback)(DWORD offset, void *buffer, unsigned int size, void *param);

class CCacheMapping {
private:
	bool _is_allocated;

	unsigned int _aligm;
	unsigned int _pages;
	unsigned int _pages_peak;
	
	void *_region_base;
	std::vector<bool> _region_map;

	_page_walk_callback _callback_load;
	void *_callback_load_param;

	_page_walk_callback _callback_unload;
	void *_callback_unload_param;

	void *GetAndCheckMappedData(DWORD offset, unsigned int size);
	void *GetAndRenewMappedData(DWORD offset, unsigned int size);
	bool FlushAndCheckMappedData(DWORD offset, unsigned int size);

public:
	CCacheMapping();
	~CCacheMapping();

	/*Зарезирвировать регион под отображение*/
	bool AllocRegion(unsigned int size);
	/*Перерезервировать регион под отображение,
		* при увеличинее возможна потеря старых указателей
		* при уменьшении указатели сохраняются */
	bool ReallocRegion(unsigned int size);
	/*Освобождаем регион и ресурсы*/
	void DestroyRegion();

	/*Регистрация коллбек-обработчика догружающего страницы*/
	bool RegDataLoadingCallback(_page_walk_callback callback, void *param);
	/*Регистрация коллбек-обработчика выгружающего страницы*/
	bool RegDataUnloadingCallback(_page_walk_callback callback, void *param);

	unsigned int GetRegionAligment();	//Размер выравнивания виртуальной страницы
	unsigned int GetRegionSize();		//Размер региона
	unsigned int GetRegionPeakSize();	//Пиковый размер региона

	/*Выделить страницы без догрузки*/
	bool AssignPages(DWORD offset, unsigned int size, void *buf);
	/*Выделить все страницы региона без догрузки*/
	bool AssignAllPages();
	/*Сбросить страницы без выгрузки*/
	bool UnassignPages(DWORD offset, unsigned int size);
	/*Сбросить страницы без выгрузки*/
	bool UnassignAllPages();

	/*Получить указатель на блок данных и догрузить их если необходимо*/
	void *GetMappedData(DWORD offset, unsigned int size);
	/*Получить указатель на все данные и догрузить их если необходимо*/
	void *GetAllMappedData();

	/*Обновить или догрузить данные*/
	void *RenewMappedData(DWORD offset, unsigned int size);
	/*Обновить или догрузить все данные*/
	void *RenewAllMappedData();

	/*Выгрузить данные и страницы*/
	bool FlushMappedData(DWORD offset, unsigned int size);
	/*Выгрузить все данные и страницы*/
	bool FlushAllMappedData();

	/*Mb TODEL*/
	/*bool FillEmptyPages(DWORD offset, unsigned int size, _page_walk_callback callback, void *param);
	bool RenewWorkedPages(DWORD offset, unsigned int size, _page_walk_callback callback, void *param);
	bool FlushWorkedPages(DWORD offset, unsigned int size, _page_walk_callback callback, void *param);*/
};

#endif