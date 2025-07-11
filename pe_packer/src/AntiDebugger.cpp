#include <Windows.h>

// https://bbs.kanxue.com/thread-225740.htm

// https://github.com/houjingyi233/test-debug/blob/gh-pages/Tencent2016D.cpp

// 定义 PROCESSINFOCLASS 枚举
typedef enum _PROCESSINFOCLASS
{
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,   // Note: this is an obsolete field and does not appear in recent
                             // versions of Windows
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    ProcessDeviceMap,
    ProcessSessionInformation,
    ProcessForegroundInformation,
    ProcessWow64Information,
    ProcessImageFileName,
    ProcessLUIDDeviceMapsEnabled,
    ProcessBreakOnTermination,
    ProcessDebugObjectHandle,
    ProcessDebugFlags,
    ProcessHandleTracing,
    ProcessIoPriority,
    ProcessExecuteFlags,
    ProcessTlsInformation,
    ProcessCookie,
    ProcessImageInformation,
    ProcessCycleTime,
    ProcessPagePriority,
    ProcessInstrumentationCallback,
    ProcessThreadStackAllocation,
    ProcessWorkingSetWatchEx,
    ProcessImageFileNameWin32,
    ProcessImageFileMapping,
    ProcessAffinityUpdateMode,
    ProcessMemoryAllocationMode,
    ProcessGroupInformation,
    ProcessTokenVirtualizationEnabled,
    ProcessConsoleHostProcess,
    ProcessWindowInformation,
    ProcessHandleInformation,
    ProcessMitigationPolicy,
    ProcessDynamicFunctionTableInformation,
    ProcessHandleCheckingMode,
    ProcessKeepAliveCount,
    ProcessRevokeFileHandles,
    ProcessWorkingSetControl,
    ProcessHandleTable,
    ProcessCheckStackExtentsMode,
    ProcessCommandLineInformation,
    ProcessProtectionInformation,
    ProcessMemoryExhaustion,
    ProcessFaultInformation,
    ProcessTelemetryIdInformation,
    ProcessCommitReleaseInformation,
    ProcessDefaultCpuSetsInformation,
    ProcessAllowedCpuSetsInformation,
    ProcessSubsystemProcess,
    ProcessJobMemoryInformation,
    ProcessInPrivate,
    ProcessRaiseUMException,
    ProcessIumChallengeResponse,
    ProcessChildProcessInformation,
    ProcessHighGraphicsPriorityInformation,
    ProcessSubsystemInformation,
    ProcessEnergyValues,
    ProcessActivityThrottleState,
    ProcessActivityThrottlePolicy,
    ProcessWin32kSyscallFilter,
    ProcessDisableSystemAllowedCpuSets,
    ProcessWakeInformation,
    ProcessEnergyTrackingState,
    ProcessManageWritesToExecutableMemory,
    ProcessCaptureTrustletLiveDump,
    ProcessTelemetryCoverage,
    ProcessEnclaveInformation,
    ProcessEnableReadWriteVmLogging,
    ProcessUptimeInformation,
    ProcessImageSection,
    ProcessDebugAuthInformation,
    ProcessSystemResourceManagement,
    ProcessSequenceNumber,
    ProcessLoaderDetour,
    ProcessSecurityDomainInformation,
    ProcessCombineSecurityDomainsInformation,
    ProcessEnableLogging,
    ProcessLeapSecondInformation,
    ProcessFiberShadowStackAllocation,
    ProcessFreeFiberShadowStackAllocation,
    MaxProcessInfoClass
} PROCESSINFOCLASS;

// 声明 NtQueryInformationProcess 函数
typedef NTSTATUS(WINAPI* NtQueryInformationProcessPtr)(HANDLE           ProcessHandle,
                                                       PROCESSINFOCLASS ProcessInformationClass,
                                                       PVOID            ProcessInformation,
                                                       ULONG            ProcessInformationLength,
                                                       PULONG           ReturnLength);

// 在你的代码中获取 NtQueryInformationProcess 的地址


BOOL CheckDebug1()
{
    int                          debugPort = 0;
    HMODULE                      hModule   = LoadLibrary("Ntdll.dll");
    NtQueryInformationProcessPtr NtQueryInformationProcess =
        (NtQueryInformationProcessPtr)GetProcAddress(hModule, "NtQueryInformationProcess");
        NtQueryInformationProcess(
        GetCurrentProcess(), (PROCESSINFOCLASS)0x7, &debugPort, sizeof(debugPort), NULL);
    return debugPort != 0;
}

BOOL CheckDebug2()
{
    HANDLE                       hdebugObject = NULL;
    HMODULE                      hModule      = LoadLibrary("Ntdll.dll");
    NtQueryInformationProcessPtr NtQueryInformationProcess =
        (NtQueryInformationProcessPtr)GetProcAddress(hModule, "NtQueryInformationProcess");
    NtQueryInformationProcess(
        GetCurrentProcess(), (PROCESSINFOCLASS)0x1E, &hdebugObject, sizeof(hdebugObject), NULL);
    return hdebugObject != NULL;
}

BOOL CheckDebug3()
{
    BOOL                         bdebugFlag = TRUE;
    HMODULE                      hModule    = LoadLibrary("Ntdll.dll");
    NtQueryInformationProcessPtr NtQueryInformationProcess =
        (NtQueryInformationProcessPtr)GetProcAddress(hModule, "NtQueryInformationProcess");
    NtQueryInformationProcess(GetCurrentProcess(), 0x1E, &bdebugFlag, sizeof(bdebugFlag), NULL);
    return bdebugFlag != TRUE;
}

BOOL CheckDebug4()
{
    DWORD errorValue = 12345;
    SetLastError(errorValue);
    OutputDebugString("Test for debugger!");
    if (GetLastError() == errorValue) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

BOOL DetectDebuuger()
{
    // 第一种
    // IsDebuggerPresent查询进程环境块(PEB)中的IsDebugged标志。如果进程没有运行在调试器环境中，函数返回0；如果调试附加了进程，函数返回一个非零值。
    if (IsDebuggerPresent() != 0) {
        return TRUE;
    }
    // 第二种
    // CheckRemoteDebuggerPresent同IsDebuggerPresent几乎一致。它不仅可以探测系统其他进程是否被调试，通过传递自身进程句柄还可以探测自身是否被调试。
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), NULL) != 0) {
        return TRUE;
    }

    // 第三种
    // NtQueryInformationProcess
    // 这个函数是Ntdll.dll中一个API，它用来提取一个给定进程的信息。
    // 它的第一个参数是进程句柄，第二个参数告诉我们它需要提取进程信息的类型。
    // 为第二个参数指定特定值并调用该函数，相关信息就会设置到第三个参数。
    // 第二个参数是一个枚举类型，其中与反调试有关的成员有ProcessDebugPort(0x7)、ProcessDebugObjectHandle(0x1E)和ProcessDebugFlags(0x1F)。
    // 例如将该参数置为ProcessDebugPort，如果进程正在被调试，则返回调试端口，否则返回0。
    // if()

    // 第四种
    // GetLastError


    return CheckRemoteDebuggerPresent(GetCurrentProcess(), NULL) || IsDebuggerPresent();
}