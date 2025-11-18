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

//----------------------------
// ЛОГИРОВАНИЕ
//----------------------------
void log_message(const string& msg) {
    lock_guard<mutex> lock(logMutex);
    ofstream log("cpp.log", ios_base::app);
    if (log.is_open()) {
        log << msg << endl;
    }
    cout << msg << endl;
}

//----------------------------
// УДАЛЕНИЕ ПАПКИ
//----------------------------
void remove_folder(const fs::path& p) {
    auto start = chrono::steady_clock::now();

    try {
        if (fs::exists(p)) {
            fs::remove_all(p);
            auto sec = chrono::duration<double>(chrono::steady_clock::now() - start).count();

            log_message("Removed: " + p.string() + " (" + to_string(sec) + " sec)");
        } else {
            auto sec = chrono::duration<double>(chrono::steady_clock::now() - start).count();

            log_message("Not found: " + p.string() + " (" + to_string(sec) + " sec)");
        }
    }
    catch (const exception& e) {
        auto sec = chrono::duration<double>(chrono::steady_clock::now() - start).count();

        log_message("Error removing " + p.string() +
                    " (" + to_string(sec) + " sec) : " + e.what());
    }
}

//----------------------------
// CURL ЗАГРУЗКА
//----------------------------
bool download_file(const string& url, const string& outFile) {
    string cmd = "curl -L \"" + url + "\" -o \"" + outFile + "\"";
    int ret = system(cmd.c_str());
    return (ret == 0 && fs::exists(outFile));
}

//----------------------------
// УМНАЯ ОЧИСТКА ПО WHITELIST
//----------------------------
void smart_clean_directory(const fs::path& targetDir, const vector<string>& whitelist) {
    if (!fs::exists(targetDir) || !fs::is_directory(targetDir)) {
        log_message("Target directory does not exist: " + targetDir.string());
        return;
    }

    log_message("Starting smart clean in: " + targetDir.string());

    for (const auto& entry : fs::directory_iterator(targetDir)) {
        if (!entry.is_directory()) continue;

        string name = entry.path().filename().string();

        bool allowed = false;
        for (const auto& w : whitelist) {
            if (w == name) {
                allowed = true;
                break;
            }
        }

        if (!allowed) {
            log_message("Deleting: " + name);
            remove_folder(entry.path());
        } else {
            log_message("Keeping: " + name);
        }
    }
}

//----------------------------
// MAIN
//----------------------------
int main() {
    fs::remove("cpp.log");

    cout << "Выберите режим:\n";
    cout << "1 — Очистить кеш (удалить директории из folders.txt)\n";
    cout << "2 — Умная очистка директории по whitelist (удалить всё, кроме списка)\n";
    cout << "3 — Выполнить оба режима\n";
    cout << "Введите номер: ";

    int mode;
    cin >> mode;
    cin.ignore(); // чтобы пути с пробелами читались корректно

    if (mode < 1 || mode > 3) {
        cout << "Неверный режим!" << endl;
        return 1;
    }

    bool doClearCache = (mode == 1 || mode == 3);
    bool doSmartClean = (mode == 2 || mode == 3);

    //-------------------------------------------------------------
    // 1) ОЧИСТКА ПО folders.txt
    //-------------------------------------------------------------
    if (doClearCache) {
        const string url = "https://raw.githubusercontent.com/shhh1ra/clear-cache/main/folders.txt";
        const string localFile = "folders.txt";

        log_message("Downloading folders.txt...");

        if (!download_file(url, localFile)) {
            log_message("Failed to download folders.txt");
        } else {
            log_message("Downloaded folders.txt");

            vector<fs::path> folders;
            ifstream f(localFile);
            string line;

            while (getline(f, line)) {
                if (!line.empty())
                    folders.emplace_back(line);
            }

            fs::remove(localFile);

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

    //-------------------------------------------------------------
    // 2) УМНАЯ ОЧИСТКА ПО WHITELIST
    //-------------------------------------------------------------
    if (doSmartClean) {
        cout << "\nВведите путь каталога для умной очистки:\n> ";

        string dir;
        getline(cin, dir);

        fs::path targetDir = dir;

        const string whitelistFile = "whitelist.txt";
        const string whitelistURL = "https://raw.githubusercontent.com/shhh1ra/clear-cache/main/whitelist.txt";

        log_message("Downloading whitelist.txt...");

        if (!download_file(whitelistURL, whitelistFile)) {
            log_message("Failed to download whitelist.txt");
        } else {
            vector<string> whitelist;
            ifstream w(whitelistFile);
            string item;

            while (getline(w, item)) {
                if (!item.empty())
                    whitelist.push_back(item);
            }

            fs::remove(whitelistFile);

            if (whitelist.empty()) {
                log_message("Whitelist is empty!");
            } else {
                smart_clean_directory(targetDir, whitelist);
            }
        }
    }

    log_message("All operations completed.");
    return 0;
}