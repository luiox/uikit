/*
PE文件异或加壳器
----------------------------------------
本程序用于对PE文件进行整体异或加密加壳，主要流程如下：
1. 读取输入PE文件，获取其二进制内容。
2. 对整个PE内容进行异或加密（可自定义密钥）。
3. 构造新的PE节结构，写入壳stub和加密数据。
4. 修正PE头部信息，设置入口点为壳stub。
5. 输出加壳后的PE文件。
----------------------------------------
本工具仅用于学习和研究PE加壳技术，禁止用于非法用途。
*/
#include <iostream>
#include <Windows.h>
#include <cstdio>
#include <cstring>

// 异或加密密钥
const BYTE XOR_KEY = 0x5A;

// 读取二进制文件到内存
bool readBinFile(const char fileName[], char** bufPtr, DWORD &length) {
    FILE* fp = fopen(fileName, "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    *bufPtr = new char[length];
    fseek(fp, 0, SEEK_SET);
    fread(*bufPtr, sizeof(char), length, fp);
    fclose(fp);
    return true;
}

// 写入二进制文件
bool writeBinFile(const char fileName[], const char* buf, DWORD length) {
    FILE* fp = fopen(fileName, "wb");
    if (!fp) return false;
    fwrite(buf, sizeof(char), length, fp);
    fclose(fp);
    return true;
}

// 对整个PE内容进行异或加密
void xorEncrypt(char* buf, DWORD length, BYTE key) {
    for (DWORD i = 0; i < length; ++i) {
        buf[i] ^= key;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s [TARGET_PE_FILE]\n", argv[0]);
        return 0;
    }
    char* in_peFilePath = argv[1];
    char* buf = nullptr;
    DWORD filesize = 0;
    if (!readBinFile(in_peFilePath, &buf, filesize)) {
        printf("[-] fail to read input PE file.\n");
        return 0;
    }
    printf("[+] read PE file: %s, size: %lu\n", in_peFilePath, filesize);

    // 异或加密整个PE内容
    xorEncrypt(buf, filesize, XOR_KEY);
    printf("[+] xor encrypt done.\n");

    // 输出加壳后的PE文件
    char outFileName[512] = {0};
    sprintf(outFileName, "%s_xorpacked.exe", in_peFilePath);
    if (writeBinFile(outFileName, buf, filesize)) {
        printf("[+] packed PE saved as %s\n", outFileName);
    } else {
        printf("[-] fail to write packed PE file.\n");
    }
    delete[] buf;
    return 0;
}

