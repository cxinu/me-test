#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

std::mutex mtx; // global mutex
bool lock_ = false; // global lock variable

vector<double> overallAvgTimes; // [avgTimeTAS, avgTimeCAS, avgTimeBounded]
vector<double> overallMaxTimes;

void entry_sec_tas() {
    while (true) {
        if (!lock_) {
            lock_ = true;
            break;
        }
    }
}

void entry_sec_cas() {
    bool expected = false;
    while (true) {
        if (lock_ == expected) {
            lock_ = true;
            break;
        }
    }
}

const int MAX_RETRIES = 10;

void entry_sec_bounded() {
    int retries = 0;
    bool expected = false;
    while (true) {
        if (lock_ == expected) {
            lock_ = true;
            break;
        }

        retries++;
        if (retries >= MAX_RETRIES) {
            // sleep for a short duration before retrying
            this_thread::sleep_for(chrono::milliseconds(10));
            retries = 0; // reset retries counter
        }
    }
}

void exit_sec() {
    lock_ = false;
}

void overallAppend(int n, int p, const string& filename) {
    freopen(filename.c_str(), "a", stdout);
    double avgTime = 0;
    double maxTime = 0;
    for (int i = n * p; i < (p + 1) * n; i++){
        avgTime += overallAvgTimes[i]/n;
        maxTime = max(maxTime, overallMaxTimes[i]);
    }
    cout << "Overall Average time to enter CS for all threads is " << avgTime << " ms" << endl;
    cout << "Overall Maximum time to enter CS for all threads is " << maxTime << " ms" << endl;
}

void testCS(int id, int k, int lambda1, int lambda2, void (*entry_sec)(), const string& filename) {
    freopen(filename.c_str(), "w", stdout);
    double totalEntryTime = 0;
    double maxEntryTime = 0;
    for (int i = 0; i < k; ++i) {
        
        auto reqEnterTime = chrono::system_clock::now();
        mtx.lock();
        cout << i << "th CS Requested by thread " << id << endl;
        mtx.unlock();

        // Entry Section
        entry_sec();

        auto actEnterTime = chrono::system_clock::now();
        mtx.lock();
        cout << i << "th CS Entered by thread " << id << endl;
        mtx.unlock();
        
        totalEntryTime += chrono::duration_cast<chrono::milliseconds>(actEnterTime - reqEnterTime).count();
        double entryTime = std::chrono::duration_cast<std::chrono::milliseconds>(actEnterTime - reqEnterTime).count();
        if (entryTime > maxEntryTime) {
            maxEntryTime = entryTime;
        }

        // simulation of critical section
        this_thread::sleep_for(chrono::milliseconds(100));

        exit_sec(); // Exit Section

        mtx.lock();
        cout << i << "th CS Exited by thread " << id << endl;
        mtx.unlock();

        // Simulation of Reminder Section
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    double avgEntryTime = totalEntryTime / k;
    overallAvgTimes.push_back(avgEntryTime);
    overallMaxTimes.push_back(maxEntryTime);
    mtx.lock();
    cout << "Average time to enter CS for thread " << id << " is " << avgEntryTime << " ms" << endl;
    cout << "Maximum time to enter CS for thread " << id << " is " << maxEntryTime << " ms" << endl;
    mtx.unlock();
}

int main() {
    ifstream inputFile("inp-params.txt");
    if (!inputFile) {
        cerr << "Error: input file could not be opened" << endl;
        return 1;
    }

    int n; // number of threads
    int k; // number of times each thread enters the critical section, fix at 10
    int lambda1, lambda2;

    inputFile >> n >> k >> lambda1 >> lambda2;
    inputFile.close();

    vector<thread> threads_tas;
    vector<thread> threads_cas;
    vector<thread> threads_bounded;
    
    for (int i = 0; i < n; ++i)
        threads_tas.push_back(thread(testCS, i, k, lambda1, lambda2, entry_sec_tas, "out-tas.txt"));
    for (auto& thread : threads_tas)
        thread.join();
    overallAppend(n, 0, "out-tas.txt");

    for (int i = 0; i < n; ++i)
        threads_cas.push_back(thread(testCS, i, k, lambda1, lambda2, entry_sec_cas, "out-cas.txt"));
    for (auto& thread : threads_cas)
        thread.join();
    overallAppend(n, 1, "out-cas.txt");

    for (int i = 0; i < n; ++i)
        threads_bounded.push_back(thread(testCS, i, k, lambda1, lambda2, entry_sec_bounded, "out-bounded.txt"));
    for (auto& thread : threads_bounded)
        thread.join();
    overallAppend(n, 2, "out-bounded.txt");
        
    return 0;
}
