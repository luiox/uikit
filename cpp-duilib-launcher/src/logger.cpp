#include "logger.h"

#include <Windows.h>
#include <shlobj.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace launcher::log {
namespace {

std::mutex& LoggerMutex() {
    static std::mutex lock;
    return lock;
}

std::filesystem::path ResolveLogPath() {
    PWSTR local_app_data = nullptr;
    std::filesystem::path base = std::filesystem::current_path() / "data";
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_app_data)) && local_app_data != nullptr) {
        base = std::filesystem::path(local_app_data) / "nassistant";
        CoTaskMemFree(local_app_data);
    }
    std::error_code ec;
    std::filesystem::create_directories(base, ec);
    return base / "nassistant.log";
}

std::string BuildLine(const char* level, const std::string& message) {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm_value{};
    localtime_s(&tm_value, &tt);

    std::ostringstream stream;
    stream << std::put_time(&tm_value, "%Y-%m-%d %H:%M:%S") << " [" << level << "] " << message;
    return stream.str();
}

void WriteLine(const char* level, const std::string& message) {
    const std::string line = BuildLine(level, message);
    const std::string debug = line + "\n";
    ::OutputDebugStringA(debug.c_str());

    std::lock_guard<std::mutex> guard(LoggerMutex());
    std::ofstream out(ResolveLogPath(), std::ios::binary | std::ios::app);
    if (!out.is_open()) {
        return;
    }
    out << line << "\n";
}

} // namespace

void Info(const std::string& message) {
    WriteLine("INFO", message);
}

void Warn(const std::string& message) {
    WriteLine("WARN", message);
}

void Error(const std::string& message) {
    WriteLine("ERROR", message);
}

void Debug(const std::string& message) {
    WriteLine("DEBUG", message);
}

} // namespace launcher::log
