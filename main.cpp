#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <cstdlib>

using namespace std;
namespace fs = std::filesystem;

mutex logMutex;

// -------------------------
// ЛОГИРОВАНИЕ
// -------------------------
void log_message(const string& msg) {
    lock_guard<mutex> lock(logMutex);
    ofstream log("cpp.log", ios::app);
    if (log.is_open()) log << msg << endl;
    cout << msg << endl;
}

// -------------------------
// УДАЛЕНИЕ ПАПКИ
// -------------------------
void remove_folder(const fs::path& p) {
    auto start = chrono::steady_clock::now();
    try {
        if (fs::exists(p)) {
            fs::remove_all(p);
            double s = chrono::duration<double>(chrono::steady_clock::now() - start).count();
            log_message("Removed: " + p.string() + " (" + to_string(s) + " sec)");
        } else {
            double s = chrono::duration<double>(chrono::steady_clock::now() - start).count();
            log_message("Not found: " + p.string() + " (" + to_string(s) + " sec)");
        }
    }
    catch (const exception& e) {
        double s = chrono::duration<double>(chrono::steady_clock::now() - start).count();
        log_message("Error removing " + p.string() + ": " + e.what());
    }
}

// -------------------------
// C URL СКАЧИВАНИЕ
// -------------------------
bool download_file(const string& url, const string& outFile) {
    string cmd = "curl -L \"" + url + "\" -o \"" + outFile + "\"";
    int ret = system(cmd.c_str());
    return (ret == 0 && fs::exists(outFile));
}

// -------------------------
// SMART CLEAN (WHITELIST)
// -------------------------
void smart_clean_directory(const fs::path& targetDir, const vector<string>& whitelist) {
    if (!fs::exists(targetDir)) {
        log_message("Target directory does not exist: " + targetDir.string());
        return;
    }

    log_message("Smart clean in: " + targetDir.string());

    for (const auto& entry : fs::directory_iterator(targetDir)) {
        if (!entry.is_directory()) continue;

        string name = entry.path().filename().string();
        bool allowed = false;

        for (auto& w : whitelist)
            if (name == w) { allowed = true; break; }

        if (!allowed) {
            log_message("Deleting folder: " + name);
            remove_folder(entry.path());
        } else {
            log_message("Keeping: " + name);
        }
    }
}

// -------------------------
// ОСНОВНАЯ ПРОГРАММА
// -------------------------
int main() {
    fs::remove("cpp.log");

    cout << "Select mode:\n";
    cout << "1 - Clear cache (folders.txt)\n";
    cout << "2 - Smart clean (whitelist.txt)\n";
    cout << "3 - Both\n";
    cout << "Enter number: ";

    int mode = 0;
    cin >> mode;

    if (mode < 1 || mode > 3) {
        cout << "Wrong mode!\n";
        return 1;
    }

    bool doClearCache = (mode == 1 || mode == 3);
    bool doSmartClean = (mode == 2 || mode == 3);

    // -------------------------------------------------------
    // 1) Clear cache (folders.txt)
    // -------------------------------------------------------
    if (doClearCache) {
        const string url = "https://raw.githubusercontent.com/shhh1ra/clear-cache/main/folders.txt";
        const string file = "folders.txt";

        log_message("Downloading folders.txt...");

        if (!download_file(url, file)) {
            log_message("Failed to download folders.txt");
        } else {
            log_message("Downloaded folders.txt");

            vector<fs::path> folders;
            ifstream f(file);
            string line;

            while (getline(f, line)) {
                if (line.empty()) continue;

                // remove \r and quotes
                line.erase(remove(line.begin(), line.end(), '\r'), line.end());
                line.erase(remove(line.begin(), line.end(), '"'), line.end());

                if (!line.empty())
                    folders.emplace_back(line);
            }

            fs::remove(file);

            if (!folders.empty()) {
                log_message("Starting deletion...");
                vector<thread> threads;

                for (auto& folder : folders)
                    threads.emplace_back(remove_folder, folder);

                for (auto& t : threads)
                    t.join();
            }
        }
    }

    // -------------------------------------------------------
    // 2) Smart clean (whitelist.txt)
    // -------------------------------------------------------
    if (doSmartClean) {
        const string whitelistURL = "https://raw.githubusercontent.com/shhh1ra/clear-cache/main/whitelist.txt";
        const string whitelistFile = "whitelist.txt";

        log_message("Downloading whitelist.txt...");

        if (!download_file(whitelistURL, whitelistFile)) {
            log_message("Failed to download whitelist.txt");
        } else {
            vector<string> whitelist;
            ifstream w(whitelistFile);
            string item;

            while (getline(w, item)) {
                if (item.empty()) continue;

                item.erase(remove(item.begin(), item.end(), '\r'), item.end());
                item.erase(remove(item.begin(), item.end(), '"'), item.end());

                if (!item.empty())
                    whitelist.push_back(item);
            }

            fs::remove(whitelistFile);

            if (whitelist.empty()) {
                log_message("Whitelist is empty!");
            } else {
                // ТУТ СТАВИШЬ СВОЙ ПУТЬ
                fs::path targetDir = "D:/Games/Steam/steamapps/common";
                smart_clean_directory(targetDir, whitelist);
            }
        }
    }

    log_message("All operations completed.");
    return 0;
}