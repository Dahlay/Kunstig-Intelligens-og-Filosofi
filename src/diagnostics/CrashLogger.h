#pragma once
// src/diagnostics/CrashLogger.h
//
// Lightweight crash / fatal-error logger.
//
// Registers a POSIX signal handler (macOS/Linux) or a Windows
// UnhandledExceptionFilter that writes a plain-text crash report to disk
// before the process exits.
//
// Usage — call once from main() before the JUCE app object:
//   CrashLogger::install();
//
// The report is written to:
//   macOS / Linux  : ~/Library/Logs/ModularAudioPatcher/crash_<timestamp>.txt
//                    (falls back to /tmp if that directory can't be created)
//   Windows        : %APPDATA%\ModularAudioPatcher\crash_<timestamp>.txt

#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>

#if defined(_WIN32)
#  include <windows.h>
#  include <dbghelp.h>
#  pragma comment(lib, "dbghelp.lib")
#else
#  include <csignal>
#  include <execinfo.h>  // backtrace / backtrace_symbols (glibc / libSystem)
#  include <unistd.h>
#endif

namespace CrashLogger
{

// ── helpers ──────────────────────────────────────────────────────────────────

static std::filesystem::path logDirectory()
{
#if defined(_WIN32)
    const char* appdata = std::getenv("APPDATA");
    std::filesystem::path base = appdata ? appdata : "C:\\ProgramData";
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    std::filesystem::path base = home ? home : "/tmp";
    base /= "Library/Logs";
#else
    const char* home = std::getenv("HOME");
    std::filesystem::path base = home ? home : "/tmp";
    base /= ".local/share";
#endif
    return base / "ModularAudioPatcher";
}

static std::string timestamp()
{
    std::time_t t = std::time(nullptr);
    char buf[32]{};
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", std::localtime(&t));
    return buf;
}

static void writeReport(const std::string& reason,
                        const std::string& extraInfo = {})
{
    try {
        auto dir = logDirectory();
        std::filesystem::create_directories(dir);
        auto path = dir / ("crash_" + timestamp() + ".txt");

        std::ofstream f(path);
        if (!f) return;

        f << "=== Modular Audio Patcher – Crash Report ===\n";
        f << "Version : " << MAP_VERSION_STRING << "\n";
        f << "Time    : " << timestamp() << "\n";
        f << "Reason  : " << reason << "\n";
        if (!extraInfo.empty())
            f << "Detail  : " << extraInfo << "\n";

#if !defined(_WIN32)
        f << "\n--- Stack trace ---\n";
        void* frames[64];
        int n = backtrace(frames, 64);
        char** syms = backtrace_symbols(frames, n);
        if (syms) {
            for (int i = 0; i < n; ++i)
                f << "  " << syms[i] << "\n";
            free(syms);
        }
#endif
        f.flush();

        // Also print to stderr so CI logs capture it
        fprintf(stderr, "[CrashLogger] Report written to: %s\n",
                path.string().c_str());
    } catch (...) {}
}

// ── platform handlers ────────────────────────────────────────────────────────

#if defined(_WIN32)

static LONG WINAPI windowsExceptionFilter(EXCEPTION_POINTERS* ep)
{
    char detail[64]{};
    snprintf(detail, sizeof(detail), "ExceptionCode=0x%08X",
             static_cast<unsigned>(ep->ExceptionRecord->ExceptionCode));
    writeReport("Unhandled Windows exception", detail);
    return EXCEPTION_CONTINUE_SEARCH;  // let the OS handle further
}

#else

static const int kFatalSignals[] = { SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS };

static void posixSignalHandler(int sig)
{
    const char* name = strsignal(sig);
    writeReport("Fatal signal", name ? name : std::to_string(sig));
    // Re-raise with default handler so the OS generates a core dump
    struct sigaction sa{};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sigaction(sig, &sa, nullptr);
    raise(sig);
}

#endif

// ── public API ───────────────────────────────────────────────────────────────

/// Install the crash handler. Call once from main() before launching the app.
inline void install()
{
#if defined(_WIN32)
    SetUnhandledExceptionFilter(windowsExceptionFilter);
#else
    struct sigaction sa{};
    sa.sa_handler = posixSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;  // one-shot; prevents recursion
    for (int sig : kFatalSignals)
        sigaction(sig, &sa, nullptr);
#endif
}

/// Write a non-fatal diagnostic note (e.g. caught exception from audio thread).
inline void logWarning(const std::string& component, const std::string& msg)
{
    try {
        auto dir = logDirectory();
        std::filesystem::create_directories(dir);
        auto path = dir / "warnings.log";
        std::ofstream f(path, std::ios::app);
        if (f)
            f << "[" << timestamp() << "] [" << component << "] " << msg << "\n";
    } catch (...) {}
}

} // namespace CrashLogger
