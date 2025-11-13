#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;
namespace fs = std::filesystem;

// Мьютекс для логов
mutex logMutex;

// Функция логирования
void log_message(const string& msg) {
    lock_guard<mutex> lock(logMutex);
    ofstream log("cpp.log", ios_base::app);
    if (log.is_open()) {
        log << msg << endl;
    }
}

// Функция удаления папки
void remove_folder(const fs::path& p) {
    try {
        if (fs::exists(p)) {
            fs::remove_all(p);
            log_message("Cache folder: " + p.string() + " successfully removed.");
            cout << "Cache folder: " << p << " successfully removed." << endl;
        } else {
            log_message("Cache folder: " + p.string() + " not found.");
            cout << "Cache folder: " << p << " not found." << endl;
        }
    } catch (const exception& e) {
        log_message("Error removing folder " + p.string() + ": " + e.what());
        cerr << "Error removing folder " << p << ": " << e.what() << endl;
    }
}

int main() {
    // Чистим старый лог
    fs::remove("cpp.log");

    fs::path current = fs::current_path();

    // Папки для удаления
    vector<fs::path> folders = {
        current / "Games" / "Steam" / "steam" / "cached",
        current / "Games" / "Steam" / "userdata",
        current / "Games" / "Steam" / "steamapps" / "shadercache",
        current / "Games" / "Steam" / "steamapps" / "temp",
        current / "Games" / "Steam" / "steamapps" / "workshop",
        current / "Games" / "Steam" / "steamapps" / "downloading",
        current / "Games" / "Steam" / "steamapps" / "common" / "Counter-Strike Global Offensive" / "csgo" / "maps" / "workshop"
    };

    vector<thread> threads;

    // Запускаем потоки
    for (auto& folder : folders) {
        threads.emplace_back(remove_folder, folder);
    }

    // Ждём завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    log_message("All folders processed.");
    cout << "All folders processed." << endl;

    return 0;
}
