#include <iostream>
#include <windows.h>

int main() {
	auto dllPath = L"inject.dll";


	auto mcWindow = FindWindowW(L"GLFW30", 0);

	DWORD mcPID;
	GetWindowThreadProcessId(mcWindow, &mcPID);

	auto mcProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, mcPID);

	

	auto alloced = VirtualAllocEx(mcProcess, 0, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(mcProcess, alloced, dllPath, 4096, 0);

	 
	auto kernel32Module = GetModuleHandleW(L"kernel32.dll");
	auto loadLibraryAddr = GetProcAddress(kernel32Module, "LoadLibraryW");


	auto remoteThread = CreateRemoteThread(mcProcess, 0, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, alloced, 0, 0);

	WaitForSingleObject(remoteThread, INFINITE);


	VirtualFreeEx(mcProcess, alloced, 0, MEM_RELEASE);
	CloseHandle(remoteThread);
	CloseHandle(mcProcess);


	return 0;
}
