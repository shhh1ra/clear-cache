#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <cstdlib> // для system()

using namespace std;
namespace fs = std::filesystem;

mutex logMutex;

void log_message(const string& msg) {
    lock_guard<mutex> lock(logMutex);
    ofstream log("cpp.log", ios_base::app);
    if (log.is_open()) {
        log << msg << endl;
    }
}

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

bool download_file(const string& url, const string& output) {
    // Используем curl через system, -L для редиректов
    string command = "curl -L \"" + url + "\" -o \"" + output + "\"";
    int ret = system(command.c_str());
    return ret == 0 && fs::exists(output);
}

int main() {
    fs::remove("cpp.log");

    const string url = "https://raw.githubusercontent.com/shhh1ra/clear-cache/main/folders.txt";
    const string localFile = "folders.txt";

    cout << "Downloading folders list..." << endl;
    if (!download_file(url, localFile)) {
        cerr << "Failed to download folders.txt" << endl;
        return 1;
    }
    cout << "Downloaded folders.txt" << endl;

    vector<fs::path> folders;

    // Читаем пути из файла
    ifstream infile(localFile);
    if (!infile.is_open()) {
        cerr << "Cannot open " << localFile << endl;
        return 1;
    }

    string line;
    while (getline(infile, line)) {
        if (!line.empty()) {
            folders.push_back(line);
        }
    }

    if (folders.empty()) {
        cerr << "folders.txt is empty" << endl;
        return 1;
    }

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

    // Удаляем скачанный файл
    try {
        fs::remove(localFile);
        cout << "Removed " << localFile << endl;
    } catch (const exception& e) {
        cerr << "Failed to remove " << localFile << ": " << e.what() << endl;
    }

    return 0;
}
