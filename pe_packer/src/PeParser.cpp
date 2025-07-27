#include "PeParser.hpp"

//默认构造函数
PeParser::PeParser() {}

bool PeParser::loadFile(std::string filePath) {
    //hFile = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    //创建内存映射文件  
    if (!(hMapping = CreateFileMapping(hFile, 0, PAGE_READWRITE | SEC_COMMIT, 0, 0, 0)))
    {
        CloseHandle(hFile);
        return FALSE;
    }
    //把文件映像存入pBaseAddr  
    if (!(pBaseAddr = MapViewOfFile(hMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0)))
    {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return FALSE;
    }
    return TRUE;
}
//返回载入地址
DWORD PeParser::getBaseAddr() {
    return (DWORD)pBaseAddr;
}
//获取NT头
IMAGE_NT_HEADERS* PeParser::getNtHeader() {
    IMAGE_DOS_HEADER* dosHead = (IMAGE_DOS_HEADER*)pBaseAddr;
    return (IMAGE_NT_HEADERS*)((DWORD)pBaseAddr + dosHead->e_lfanew);
}

//获取DOS头
IMAGE_DOS_HEADER* PeParser::getDosHeader() {
    return (IMAGE_DOS_HEADER*)pBaseAddr;
}
//获取节表起始
IMAGE_SECTION_HEADER* PeParser::getSectionStart() {
    return (IMAGE_SECTION_HEADER*)((DWORD)getNtHeader() + sizeof(IMAGE_NT_HEADERS));
}
//获取导入表
IMAGE_IMPORT_DESCRIPTOR* PeParser::getImport() {
    DWORD importTable = RvaToVa(getNtHeader()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    return (IMAGE_IMPORT_DESCRIPTOR*)importTable;
}
//获取导出表
IMAGE_EXPORT_DIRECTORY* PeParser::getExport() {
    DWORD exportTable = RvaToVa(getNtHeader()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    return (IMAGE_EXPORT_DIRECTORY*)exportTable;
}
//获取重定位
IMAGE_BASE_RELOCATION* PeParser::getReLocation() {
    DWORD reLocation = RvaToVa(getNtHeader()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    return (IMAGE_BASE_RELOCATION*)reLocation;
}
//获取资源表
IMAGE_RESOURCE_DIRECTORY* PeParser::getResource() {
    DWORD source = RvaToVa(getNtHeader()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
    return (IMAGE_RESOURCE_DIRECTORY*)source;
}

//计算 RvaToVa
DWORD PeParser::RvaToVa(DWORD rva) {
    IMAGE_SECTION_HEADER* sectionHeader;
    int secNum = getNtHeader()->FileHeader.NumberOfSections;
    sectionHeader = getSectionStart();
    for (int i = 0; i < secNum; i++)
    {
        if ((sectionHeader->VirtualAddress <= rva) && rva < (sectionHeader->VirtualAddress + sectionHeader->SizeOfRawData))
            return (rva - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData) + (DWORD)pBaseAddr;
        sectionHeader++;
    }
    return 0;
}
//清除内存映射和关闭文件 
void PeParser::UnFile() {

    if (pBaseAddr != NULL)
    {
        UnmapViewOfFile(pBaseAddr);
    }
    if (hMapping != NULL)
    {
        CloseHandle(hMapping);
    }
    if (hFile != NULL)
    {
        CloseHandle(hFile);
    }


}
//验证是否是PE文件
bool PeParser::isPeFile() {
    if (getDosHeader()->e_magic != IMAGE_DOS_SIGNATURE)
    {
        return FALSE;
    }
    if (getNtHeader()->Signature != IMAGE_NT_SIGNATURE)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL IsPeFile(LPVOID imageBase)
{
    IMAGE_DOS_HEADER* pDosHeader = NULL;
    IMAGE_NT_HEADERS* pNtHeader = NULL;

    if (imageBase == NULL) {
        return FALSE;
    }
    // 判断mz
    pDosHeader = (IMAGE_DOS_HEADER*)imageBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }
    // 判断pe
    pNtHeader = (IMAGE_NT_HEADERS*)pDosHeader + pDosHeader->e_lfanew;
    if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
        return FALSE;
    }
    return TRUE;
}
