#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;

mutex logMutex;

// Логирование в файл с защитой mutex
void log_message(const string& msg) {
    lock_guard<mutex> lock(logMutex);
    ofstream log("cpp.log", ios_base::app);
    if (log.is_open()) {
        log << msg << endl;
    }
}

// Удаление папки с замером времени
void remove_folder(const fs::path& p) {
    auto start_time = chrono::steady_clock::now();

    try {
        if (fs::exists(p)) {
            fs::remove_all(p);
            auto end_time = chrono::steady_clock::now();
            chrono::duration<double> elapsed = end_time - start_time;

            string msg = "Cache folder: " + p.string() + " successfully removed in " +
                         to_string(elapsed.count()) + " seconds.";
            log_message(msg);
            cout << msg << endl;
        } else {
            auto end_time = chrono::steady_clock::now();
            chrono::duration<double> elapsed = end_time - start_time;

            string msg = "Cache folder: " + p.string() + " not found (checked in " +
                         to_string(elapsed.count()) + " seconds).";
            log_message(msg);
            cout << msg << endl;
        }
    } catch (const exception& e) {
        auto end_time = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end_time - start_time;

        string msg = "Error removing folder " + p.string() + " (" + to_string(elapsed.count()) +
                     " seconds), error: " + e.what();
        log_message(msg);
        cerr << msg << endl;
    }
}

int main() {
    // Чистим старый лог
    fs::remove("cpp.log");

    fs::path current = fs::current_path();

    vector<fs::path> folders = {
        current / "Games" / "Steam" / "steam" / "cached",
        current / "Games" / "Steam" / "userdata",
        current / "Games" / "Steam" / "steamapps" / "shadercache",
        current / "Games" / "Steam" / "steamapps" / "temp",
        current / "Games" / "Steam" / "steamapps" / "workshop",
        current / "Games" / "Steam" / "steamapps" / "downloading",
        current / "Games" / "Steam" / "steamapps" / "common" / "Counter-Strike Global Offensive" / "csgo" / "maps" / "workshop"
    };

    // Замер времени начала всего процесса
    auto start_total = chrono::steady_clock::now();

    vector<thread> threads;
    for (auto& folder : folders) {
        threads.emplace_back(remove_folder, folder);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end_total = chrono::steady_clock::now();
    chrono::duration<double> total_elapsed = end_total - start_total;

    string total_msg = "All folders processed in " + to_string(total_elapsed.count()) + " seconds.";
    log_message(total_msg);
    cout << total_msg << endl;

    return 0;
}