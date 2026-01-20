#include <Windows.h>
#include <iostream>

using namespace std;

// ============================= 节信息结构体 ================================
struct SecInfo {
  DWORD m_dwOff;
  DWORD m_dwComSize;   // 压缩数据的偏移
  DWORD m_dwDecomOff;  // 解压缩数据的偏移，RVA
  DWORD m_dwDecomSize; // 解压缩后数据的大小
};

// ============================= 压缩信息结构体 ================================
struct ComDataInfo {
  DWORD m_dwSecOff;       // 压缩数据偏移
  DWORD m_dwSecInfoCount; // 压缩节的总个数
  DWORD m_dwSecInfoOff;   // 压缩节信息偏移
  DWORD m_dwImpInfoCount; //
  DWORD m_dwImpInfoOff;
  DWORD m_dwOep;
};

DWORD GetHash(char *fun_name) {
  DWORD digest = 0;
  while (*fun_name) {
    digest = ((digest << 25) | (digest >> 7));
    digest = digest + *fun_name;
    fun_name++;
  }
  return digest;
}

#define FORMAT_EXE 1
#define FORMAT_DLL 2

int checkFormat(char *filePath) {
  FILE *f = fopen(filePath, "rb");
  if (!f) {
    printf("Cannot open file\n");
    return 1;
  }

  IMAGE_DOS_HEADER dosHeader;
  fread(&dosHeader, sizeof(IMAGE_DOS_HEADER), 1, f);

  if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
    printf("Not a valid PE file\n");
    fclose(f);
    return -1;
  }

  fseek(f, dosHeader.e_lfanew, SEEK_SET);
  IMAGE_NT_HEADERS ntHeaders;
  fread(&ntHeaders, sizeof(IMAGE_NT_HEADERS), 1, f);

  if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
    printf("Not a valid PE file\n");
    fclose(f);
    return -1;
  }

  int flag;
  if (ntHeaders.FileHeader.Characteristics & IMAGE_FILE_DLL) {
    printf("This is a DLL file\n");
    flag = FORMAT_EXE;
    // DLL-specific code here
  } else {
    printf("This is an EXE file\n");
    // EXE-specific code here
    flag = FORMAT_DLL;
  }

  fclose(f);
  return flag;
}

int main(int argc, char **argv) {
  const char *filePath = "D:/WorkSpace/mc/CaPacker/build/windows/x64/release/Demo.exe";

  int format = checkFormat((char *)filePath);
  if (format == FORMAT_EXE) {
      
  }
  if (format == FORMAT_DLL) {
      
  }
  return 0;
}
