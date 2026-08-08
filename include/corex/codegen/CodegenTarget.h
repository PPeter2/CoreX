#pragma once
#include <string>

enum class CallingConvention {
    SystemV,
    Win64
};

enum class ObjectFormat {
    Elf,
    MachO,
    Pe
};

struct CodegenTarget {
    CallingConvention callingConvention;
    ObjectFormat objectFormat;

    static CodegenTarget linuxX64() {
        return {CallingConvention::SystemV, ObjectFormat::Elf};
    }
    static CodegenTarget macosX64() {
        return {CallingConvention::SystemV, ObjectFormat::MachO};
    }
    static CodegenTarget windowsX64() {
        return {CallingConvention::Win64, ObjectFormat::Pe};
    }

    static CodegenTarget hostDefault() {
#if defined(_WIN32)
        return windowsX64();
#elif defined(__APPLE__)
        return macosX64();
#else
        return linuxX64();
#endif
    }

    static bool fromName(const std::string& name, CodegenTarget& out) {
        if (name == "linux")   { out = linuxX64();   return true; }
        if (name == "macos")   { out = macosX64();   return true; }
        if (name == "windows") { out = windowsX64(); return true; }
        return false;
    }
};