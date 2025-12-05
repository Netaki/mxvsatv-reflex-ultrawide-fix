#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <string>
#include <filesystem>
#include <vector>
#include <limits>

using namespace std;

#ifndef PATCHER_VERSION
#define PATCHER_VERSION "dev"
#endif

struct Patch {
    uint32_t offset;
    const uint8_t* original;
    const uint8_t* patched;
    size_t length;
};

// Offsets and patch bytes
constexpr uint32_t OFFSET_AR_CHECK = 0x3AC7FC;
constexpr uint32_t OFFSET_VIEWPORT = 0x3AC818;

constexpr uint8_t ORIGINAL_AR[]  = { 0x76 };
constexpr uint8_t PATCH_AR[]     = { 0xEB };

constexpr uint8_t ORIGINAL_VIEWPORT[] = { 0x8D,0x2C,0xDB,0xC1,0xE3,0x04 };
constexpr uint8_t PATCH_VIEWPORT[]    = { 0x89,0xF3,0x90,0x89,0xFD,0x90 };

// Reads a sequence of bytes from a given offset
bool ReadBytes(fstream& file, const uint32_t offset, uint8_t* buffer, const size_t len) {
    file.clear();
    file.seekg(offset);
    if (!file.good()) return false;
    file.read(reinterpret_cast<char*>(buffer), len);
    return file.good();
}

// Writes a sequence of bytes to a given offset
bool PatchBytes(fstream& file, const uint32_t offset, const uint8_t* data, const size_t len) {
    file.clear();
    file.seekp(offset);
    if (!file.good()) return false;
    file.write(reinterpret_cast<const char*>(data), len);
    return file.good();
}

// Applies a single patch
bool ApplyPatch(fstream& file, const Patch& patch) {
    vector<uint8_t> buffer(patch.length);

    if (!ReadBytes(file, patch.offset, buffer.data(), patch.length))
        return false;

    if (memcmp(buffer.data(), patch.patched, patch.length) == 0) {
        cout << "    -> Already patched.\n";
        return true;
    }

    if (memcmp(buffer.data(), patch.original, patch.length) != 0) {
        cout << "    [!] WARNING: Bytes do not match expected original.\n";
        cout << "        Continue anyway? (y/n): ";
        char c; cin >> c;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (c != 'y' && c != 'Y') return false;
    }

    if (!PatchBytes(file, patch.offset, patch.patched, patch.length))
        return false;

    cout << "    -> Patched.\n";
    return true;
}

// Checks whether all patches are already applied
bool IsFileAlreadyPatched(fstream& file, const vector<Patch>& patches) {
    for (const auto& patch : patches) {
        vector<uint8_t> buffer(patch.length);
        if (!ReadBytes(file, patch.offset, buffer.data(), patch.length))
            return false;
        if (memcmp(buffer.data(), patch.patched, patch.length) != 0)
            return false;
    }
    return true;
}

void WaitForInput() {
    cout << "\nPress Enter to exit...";
    cin.get();
}

int main() {
    cout << "=== MX vs ATV Reflex Ultrawide Patcher v" << PATCHER_VERSION << " ===\n";

    string filename;
    cout << "Path to MXReflex.exe: ";
    getline(cin, filename);

    if (filename.empty()) {
        cout << "ERROR: No file path entered.\n";
        WaitForInput();
        return 0;
    }

    if (filename.front() == '"' && filename.back() == '"')
        filename = filename.substr(1, filename.size() - 2);

    const vector<Patch> patches = {
        { OFFSET_AR_CHECK, ORIGINAL_AR, PATCH_AR, 1 },
        { OFFSET_VIEWPORT, ORIGINAL_VIEWPORT, PATCH_VIEWPORT, 6 }
    };

    fstream file(filename, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        cout << "ERROR: Cannot open file.\n";
        WaitForInput();
        return 0;
    }

    if (IsFileAlreadyPatched(file, patches)) {
        cout << "[!] File is already fully patched. Nothing to do.\n";
        WaitForInput();
        return 0;
    }

    cout << "[*] Creating backup...\n";
    filesystem::copy_file(filename, filename + ".bak", filesystem::copy_options::overwrite_existing);
    cout << "    -> Backup saved as: " << filename << ".bak\n";

    bool failure = false;
    cout << "[1/2] Applying Aspect Ratio Patch...\n";
    if (!ApplyPatch(file, patches[0]))
        failure = true;

    cout << "[2/2] Applying Viewport Patch...\n";
    if (!ApplyPatch(file, patches[1]))
        failure = true;

    if (failure) {
        cout << "\n[!] Patch failed. Restoring backup...\n";
        file.close();
        filesystem::copy_file(filename + ".bak", filename, filesystem::copy_options::overwrite_existing);
        cout << "    -> Backup restored.\n";
    } else {
        cout << "\nUltrawide patch applied successfully.\n";
    }

    WaitForInput();
    return 0;
}