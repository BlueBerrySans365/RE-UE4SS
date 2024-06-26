#include <CrashDumper.hpp>

#include <chrono>
#include <string>
#include <format>
#include <filesystem>
#include <bit>

#define NOMINMAX
#include <Windows.h>
#include <dbghelp.h>

namespace fs = std::filesystem;

using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::time_point_cast;

namespace RC
{
    const int DumpType = MiniDumpNormal | MiniDumpWithThreadInfo | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithModuleHeaders | MiniDumpWithAvxXStateContext;

    static bool FullMemoryDump = false;

    LONG WINAPI ExceptionHandler(_EXCEPTION_POINTERS* exception_pointers)
    {
        const auto now = time_point_cast<seconds>(system_clock::now());
        const std::wstring dump_path = (fs::current_path() / std::format("crash_{:%Y_%m_%d_%H_%M_%S}.dmp", now)).wstring();

        const HANDLE file = CreateFileW(dump_path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (file == INVALID_HANDLE_VALUE)
        {
            const std::wstring message = std::format(L"Failed to create crashdump file, reason: {:x}", GetLastError());
            MessageBoxW(NULL, message.c_str(), L"Fatal Error!", MB_OK);
            return EXCEPTION_CONTINUE_SEARCH;
        }

        _MINIDUMP_EXCEPTION_INFORMATION exception_information{};
        exception_information.ThreadId = GetCurrentThreadId();
        exception_information.ExceptionPointers = exception_pointers;
        exception_information.ClientPointers = NULL;

        const int additional_dump_flags = FullMemoryDump ? MiniDumpWithFullMemory | MiniDumpIgnoreInaccessibleMemory : 0;
        bool ok = MiniDumpWriteDump(GetCurrentProcess(),
                                    GetCurrentProcessId(),
                                    file,
                                    static_cast<MINIDUMP_TYPE>(DumpType | additional_dump_flags),
                                    &exception_information,
                                    NULL,
                                    NULL);
        CloseHandle(file);

        if (!ok)
        {
            const std::wstring message = std::format(L"Failed to write crashdump file, reason: {:x}", GetLastError());
            MessageBoxW(NULL, message.c_str(), L"Fatal Error!", MB_OK);
            return EXCEPTION_CONTINUE_SEARCH;
        }

        const std::wstring message = std::format(L"Crashdump written to: {}", dump_path);
        MessageBoxW(NULL, message.c_str(), L"Fatal Error!", MB_OK);

        return EXCEPTION_EXECUTE_HANDLER;
    }

    LPTOP_LEVEL_EXCEPTION_FILTER WINAPI HookedSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER filter)
    {
        return nullptr;
    }

    CrashDumper::CrashDumper()
    {
    }

    CrashDumper::~CrashDumper()
    {
        m_set_unhandled_exception_filter_hook = {};
        SetUnhandledExceptionFilter(reinterpret_cast<LPTOP_LEVEL_EXCEPTION_FILTER>(m_previous_exception_filter));
    }

    void CrashDumper::enable()
    {
        SetErrorMode(SEM_FAILCRITICALERRORS);
        m_previous_exception_filter = SetUnhandledExceptionFilter(ExceptionHandler);
        m_set_unhandled_exception_filter_hook = safetyhook::create_inline(SetUnhandledExceptionFilter, HookedSetUnhandledExceptionFilter);
        this->enabled = true;
    }

    void CrashDumper::set_full_memory_dump(bool enabled)
    {
        FullMemoryDump = enabled;
    }

} // namespace RC
