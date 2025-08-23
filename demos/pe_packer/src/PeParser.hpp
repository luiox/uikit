#pragma once

// PeParser
#include <Windows.h>
#include <ImageHlp.h>
#include <string>

class PeParser
{
private:
	HANDLE hFile;
	HANDLE hMapping;
	PVOID pBaseAddr;
public:
	//构造函数
	PeParser();
	//加载文件
	bool loadFile(std::string filePath);
	//返回载入的地址
	DWORD getBaseAddr();
	//判断是否是PE文件
	bool isPeFile();
	//获取DOS头
	IMAGE_DOS_HEADER* getDosHeader();
	//获取NT头
	IMAGE_NT_HEADERS* getNtHeader();
	//获取节表起始
	IMAGE_SECTION_HEADER* getSectionStart();
	//获取导入表
	IMAGE_IMPORT_DESCRIPTOR* getImport();
	//获取导出表
	IMAGE_EXPORT_DIRECTORY* getExport();
	//获取重定位
	IMAGE_BASE_RELOCATION* getReLocation();
	//获取资源
	IMAGE_RESOURCE_DIRECTORY* getResource();
	//计算RvaToVa
	DWORD RvaToVa(DWORD rva);
	//关闭一系列操作
	void UnFile();
};


BOOL IsPeFile(LPVOID imageBase);
