/*
PE异或壳装载器（32位）
----------------------------------------
本装载器用于运行时对PE文件进行异或解密，并跳转到原始入口点。
1. 获取自身基址，定位加密数据区。
2. 用密钥对加密数据区进行异或解密。
3. 跳转到原始入口点执行。
----------------------------------------
本代码仅用于学习和研究PE加壳技术，禁止用于非法用途。
*/
#include <Windows.h>
#include <stdint.h>

// 壳参数结构体（加壳器写入到PE某节）
typedef struct {
    DWORD originalEntryRVA; // 原始入口点RVA
    DWORD encryptedDataRVA; // 加密数据RVA
    DWORD encryptedDataSize;// 加密数据大小
    BYTE  xorKey;           // 异或密钥
} XorShellParam;

// 获取PE基址
static DWORD GetImageBase() {
    return (DWORD)GetModuleHandleA(NULL);
}

// 异或解密
void XorDecrypt(BYTE* buf, DWORD size, BYTE key) {
    for (DWORD i = 0; i < size; ++i) buf[i] ^= key;
}

// 装载器入口（加壳器设置为PE入口点）
extern "C" __declspec(dllexport) void __stdcall XorLoaderMain() {
    DWORD imageBase = GetImageBase();
    // 假设参数结构体放在最后一个节
    XorShellParam* param = (XorShellParam*)(imageBase + /*加壳器需填充此RVA*/ 0xDEADBEEF);
    BYTE* encData = (BYTE*)(imageBase + param->encryptedDataRVA);
    XorDecrypt(encData, param->encryptedDataSize, param->xorKey);
    // 跳转到原始入口点
    ((void(*)())(imageBase + param->originalEntryRVA))();
}

// 入口点（实际加壳器会设置为XorLoaderMain）
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    return TRUE;
}

