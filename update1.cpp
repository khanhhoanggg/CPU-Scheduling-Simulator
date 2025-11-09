#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <string>
#include <queue>
#include <sstream>
#include <cmath>
using namespace std;

// Cấu trúc tiến trình
struct Process {
    string id;
    int arrival;
    int burst;
    int priority;
    int start;
    int finish;
    int waiting;
    int turnaround;
    int remainingBurst;
    int queueLevel;    // Cho MLQ/MLFQ
    int lastRunTime;   // Thời điểm cuối cùng tiến trình rời CPU hoặc thời điểm Arrival

    Process(string i, int a, int b, int p = 0) 
        : id(i), arrival(a), burst(b), priority(p), 
          start(-1), finish(0), waiting(0), turnaround(0), 
          remainingBurst(b), queueLevel(0), lastRunTime(a) {}
};

vector<Process> readProcesses(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);    
    if (!file.is_open()) {
        cout << "Khong the mo file: " << filename << endl;
        return processes;
    }
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;    
        stringstream ss(line);
        string id;
        int arrival, burst, priority = 0;
        
        ss >> id >> arrival >> burst;
        if (ss >> priority) {
            processes.push_back(Process(id, arrival, burst, priority));
        } else {
            processes.push_back(Process(id, arrival, burst, 0));
        }
    }
    
    file.close();
    cout << "Da doc " << processes.size() << " tien trinh tu file.\n" << endl;
    return processes;
}


pair<double, double> calculateAverages(const vector<Process>& processes) {
    if (processes.empty()) return {0.0, 0.0};
    double totalWaiting = 0, totalTurnaround = 0;
    for (const auto& p : processes) {
        totalWaiting += p.waiting;
        totalTurnaround += p.turnaround;
    }
    return {totalWaiting / processes.size(), totalTurnaround / processes.size()};
}

void printResults(const string& algorithmName, vector<Process>& processes, bool showPriority = false, bool showQueue = false) {
    // Sắp xếp lại theo Process ID để dễ xem
    sort(processes.begin(), processes.end(), 
         [](const Process& a, const Process& b) { 
             return a.id < b.id; 
         });
         
    cout << "\n========================================" << endl;
    cout << "  " << algorithmName << endl;
    cout << "========================================" << endl;
    
    cout << left << setw(8) << "PID" 
         << setw(10) << "Arrival" 
         << setw(10) << "Burst";
    
    if (showPriority) {
        cout << setw(10) << "Priority";
    }
    
    if (showQueue) {
        cout << setw(10) << "Queue";
    }
    
    cout << setw(10) << "Start" 
         << setw(10) << "Finish" 
         << setw(10) << "Waiting" 
         << setw(12) << "Turnaround" << endl;
    
    int lineLength = 70;
    if (showPriority) lineLength += 10;
    if (showQueue) lineLength += 10;
    cout << string(lineLength, '-') << endl;
    
    for (const auto& p : processes) {
        cout << left << setw(8) << p.id 
             << setw(10) << p.arrival 
             << setw(10) << p.burst;
        
        if (showPriority) {
            cout << setw(10) << p.priority;
        }
        
        if (showQueue) {
            cout << setw(10) << p.queueLevel;
        }
        
        cout << setw(10) << p.start 
             << setw(10) << p.finish 
             << setw(10) << p.waiting 
             << setw(12) << p.turnaround << endl;
    }
    
    auto [avgWaiting, avgTurnaround] = calculateAverages(processes);
    
    cout << "\nThoi gian cho trung binh (Avg Waiting Time): " 
         << fixed << setprecision(2) << avgWaiting << endl;
    cout << "Thoi gian quay vong trung binh (Avg Turnaround Time): " 
         << fixed << setprecision(2) << avgTurnaround << endl;
}

vector<Process> fcfs(vector<Process> processes) {
    sort(processes.begin(), processes.end(), 
         [](const Process& a, const Process& b) { return a.arrival < b.arrival; });
    
    int currentTime = 0;
    for (auto& p : processes) {
        p.start = max(currentTime, p.arrival);
        p.finish = p.start + p.burst;
        p.waiting = p.start - p.arrival;
        p.turnaround = p.finish - p.arrival;
        currentTime = p.finish;
    }
    
    return processes;
}

vector<Process> sjf(vector<Process> processes) {
    vector<Process> result;
    vector<bool> completed(processes.size(), false);
    int currentTime = 0;
    int completedCount = 0;
    
    while (completedCount < processes.size()) {
        int idx = -1;
        int minBurst = INT_MAX;
        
        for (int i = 0; i < processes.size(); i++) {
            if (!completed[i] && processes[i].arrival <= currentTime) {
                if (processes[i].burst < minBurst) {
                    minBurst = processes[i].burst;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            int minArrival = INT_MAX;
            for (int i = 0; i < processes.size(); i++) {
                if (!completed[i] && processes[i].arrival < minArrival) {
                    minArrival = processes[i].arrival;
                }
            }
            currentTime = minArrival;
        } else {
            Process& p = processes[idx];
            p.start = currentTime;
            p.finish = p.start + p.burst;
            p.waiting = p.start - p.arrival;
            p.turnaround = p.finish - p.arrival;
            currentTime = p.finish;
            completed[idx] = true;
            completedCount++;
            result.push_back(p);
        }
    }
    
    return result;
}

vector<Process> priorityScheduling(vector<Process> processes) {
    vector<Process> result;
    vector<bool> completed(processes.size(), false);
    int currentTime = 0;
    int completedCount = 0;
    
    while (completedCount < processes.size()) {
        int idx = -1;
        int highestPriority = INT_MAX;
        
        for (int i = 0; i < processes.size(); i++) {
            if (!completed[i] && processes[i].arrival <= currentTime) {
                // Priority: số nhỏ = ưu tiên cao
                if (processes[i].priority < highestPriority) {
                    highestPriority = processes[i].priority;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            int minArrival = INT_MAX;
            for (int i = 0; i < processes.size(); i++) {
                if (!completed[i] && processes[i].arrival < minArrival) {
                    minArrival = processes[i].arrival;
                }
            }
            currentTime = minArrival;
        } else {
            Process& p = processes[idx];
            p.start = currentTime;
            p.finish = p.start + p.burst;
            p.waiting = p.start - p.arrival;
            p.turnaround = p.finish - p.arrival;
            currentTime = p.finish;
            completed[idx] = true;
            completedCount++;
            result.push_back(p);
        }
    }
    
    return result;
}

vector<Process> roundRobin(vector<Process> processes, int quantum) {
    vector<Process> procs = processes;
    queue<int> readyQueue;
    vector<bool> inQueue(procs.size(), false);
    vector<Process> result;
    
    int currentTime = 0;
    int completedCount = 0;
    int procIndex = 0;

    // Sắp xếp theo Arrival Time để dễ dàng đưa vào Queue
    sort(procs.begin(), procs.end(), 
         [](const Process& a, const Process& b) { return a.arrival < b.arrival; });
    
    // Bắt đầu từ thời gian đến sớm nhất
    if (!procs.empty()) {
        currentTime = procs[0].arrival;
    }

    while (completedCount < procs.size()) {
        
        // B1: Đưa các tiến trình đến mới vào Queue (theo thứ tự đến)
        while (procIndex < procs.size() && procs[procIndex].arrival <= currentTime) {
            int i = procIndex;
            if (procs[i].remainingBurst > 0 && !inQueue[i]) {
                readyQueue.push(i);
                inQueue[i] = true;
            }
            procIndex++;
        }

        if (readyQueue.empty()) {
            // CPU Idle: Nhảy thời gian đến tiến trình kế tiếp
            if (procIndex < procs.size()) {
                currentTime = procs[procIndex].arrival;
            } else {
                break; // Hoàn thành
            }
            continue;
        }

        int i = readyQueue.front();
        readyQueue.pop();
        inQueue[i] = false;
        
        Process& p = procs[i];
        
        // B2: Tính toán thời gian chờ và cập nhật thời gian bắt đầu
        p.waiting += currentTime - p.lastRunTime;
        
        if (p.start == -1) {
            p.start = currentTime;
        }
        
        int executeTime = min(quantum, p.remainingBurst);
        
        // B3: Thực thi
        currentTime += executeTime;
        p.remainingBurst -= executeTime;
        p.lastRunTime = currentTime; // Cập nhật thời gian cuối cùng rời CPU

        // B4: Tải thêm tiến trình mới đến trong thời gian chạy của quantum (Quan trọng!)
        int tempProcIndex = procIndex;
        while (tempProcIndex < procs.size() && procs[tempProcIndex].arrival <= currentTime) {
            int j = tempProcIndex;
            if (procs[j].remainingBurst > 0 && !inQueue[j]) {
                readyQueue.push(j);
                inQueue[j] = true;
            }
            tempProcIndex++;
        }
        procIndex = tempProcIndex;
        
        // B5: Phản hồi/Hoàn thành
        if (p.remainingBurst > 0) {
            // Bị gián đoạn, đưa trở lại cuối queue
            readyQueue.push(i);
            inQueue[i] = true;
        } else {
            // Hoàn thành
            p.finish = currentTime;
            p.turnaround = p.finish - p.arrival;
            // p.waiting đã được tính trong B2
            result.push_back(p);
            completedCount++;
        }
    }
    
    return result;
}

// Queue 0: Priority cao (FCFS)
// Queue 1: Priority trung bình (Round Robin q=2)
// Queue 2: Priority thấp (Round Robin q=4)
vector<Process> mlq(vector<Process> processes) {
    // MLQ là Non-preemptive giữa các Queue: Q0 phải hoàn thành -> Q1 chạy -> Q2 chạy
    vector<vector<Process>> queues(3);
    
    // Phân loại tiến trình vào các queue dựa trên priority (Tương tự code gốc)
    for (auto p : processes) {
        if (p.priority <= 1) {
            p.queueLevel = 0;   // Foreground (FCFS)
            queues[0].push_back(p);
        } else if (p.priority == 2) {
            p.queueLevel = 1;   // Interactive (RR q=2)
            queues[1].push_back(p);
        } else {
            p.queueLevel = 2;   // Background (RR q=4)
            queues[2].push_back(p);
        }
    }
    
    vector<Process> result;
    int currentTime = 0;
    
    // 1. Xử lý Queue 0 (FCFS)
    auto q0Result = fcfs(queues[0]);
    for (auto& p : q0Result) {
        currentTime = max(currentTime, p.finish); // Cập nhật thời gian kết thúc của Q0
        result.push_back(p);
    }
    
    // 2. Xử lý Queue 1 (RR q=2)
    if (!queues[1].empty()) {
        auto q1Processes = queues[1];
        
        // Chỉnh sửa thời gian đến để tất cả bắt đầu sau khi Q0 xong
        int q1StartTime = INT_MAX;
        if (!q0Result.empty()) {
            q1StartTime = q0Result.back().finish;
        } else {
            // Nếu Q0 rỗng, bắt đầu từ Arrival Time sớm nhất của Q1
            for (const auto& p : q1Processes) q1StartTime = min(q1StartTime, p.arrival);
        }

        // Tạo một bản sao tiến trình Q1 với thời gian đến được điều chỉnh
        for (auto& p : q1Processes) {
            // Giả định tiến trình Q1 chỉ có thể đến CPU sau khi Q0 hoàn thành
            p.arrival = max(p.arrival, q1StartTime); 
            p.lastRunTime = p.arrival;
        }

        auto q1Result = roundRobin(q1Processes, 2);

        // Tính toán lại finish/waiting/turnaround dựa trên thời gian thực tế
        for (auto& p : q1Result) {
            p.queueLevel = 1;
            // Chỉnh sửa lại kết quả để Start Time thực tế không nhỏ hơn thời gian kết thúc của Q0
            // Logic RR đã lo việc tính toán Start/Finish/Waiting/Turnaround dựa trên Arrival Time mới (max(Arrival, q1StartTime))
            
            // Cập nhật currentTime dựa trên thời gian kết thúc thực tế của tiến trình RR trong Q1
            currentTime = max(currentTime, p.finish); 
            result.push_back(p);
        }
    }
    
    // 3. Xử lý Queue 2 (RR q=4)
    if (!queues[2].empty()) {
        auto q2Processes = queues[2];

        // Queue 2 chỉ chạy sau khi Q0 và Q1 hoàn thành
        int q2StartTime = currentTime;
        
        for (auto& p : q2Processes) {
            p.arrival = max(p.arrival, q2StartTime); 
            p.lastRunTime = p.arrival;
        }

        auto q2Result = roundRobin(q2Processes, 4);

        for (auto& p : q2Result) {
            p.queueLevel = 2;
            currentTime = max(currentTime, p.finish); 
            result.push_back(p);
        }
    }
    
    return result;
}

vector<Process> mlfq(vector<Process> processes) {
    // MLFQ là Preemptive giữa các Queue: Q0 > Q1 > Q2
    vector<Process> procs = processes;
    vector<queue<int>> queues(3);
    vector<Process> result;
    
    int currentTime = 0;
    int completedCount = 0;
    
    // Khởi tạo trạng thái
    for (int i = 0; i < procs.size(); i++) {
        procs[i].queueLevel = 0;
        procs[i].lastRunTime = procs[i].arrival; // Đặt lại lastRunTime = Arrival Time
    }
    
    // Bắt đầu từ thời gian đến sớm nhất
    if (!procs.empty()) {
        int firstArrival = INT_MAX;
        for (const auto& p : procs) {
            firstArrival = min(firstArrival, p.arrival);
        }
        currentTime = firstArrival;
    }

    // Biến theo dõi xem tiến trình đã được đưa vào Queue lần đầu chưa
    vector<bool> initiallyQueued(procs.size(), false);
    
    vector<int> quantums = {2, 4, INT_MAX}; // Q2 dùng FCFS

    while (completedCount < procs.size()) {
        
        // B1: Đưa tiến trình mới đến (hoặc lần đầu) vào Queue 0
        for (int i = 0; i < procs.size(); i++) {
            if (procs[i].remainingBurst > 0 && procs[i].arrival <= currentTime && !initiallyQueued[i]) {
                queues[0].push(i);
                initiallyQueued[i] = true;
            }
        }
        
        int i = -1;
        int qLevel = -1;
        
        // B2: Chọn tiến trình từ Queue có ưu tiên cao nhất (Q0 > Q1 > Q2)
        for (int level = 0; level < 3; level++) {
            if (!queues[level].empty()) {
                i = queues[level].front();
                queues[level].pop();
                qLevel = level;
                break;
            }
        }
        
        if (i == -1) {
            // CPU Idle: Nhảy thời gian đến tiến trình kế tiếp chưa được xử lý
            int minArrival = INT_MAX;
            for (int j = 0; j < procs.size(); j++) {
                if (procs[j].remainingBurst > 0 && procs[j].arrival < minArrival && !initiallyQueued[j]) {
                    minArrival = procs[j].arrival;
                }
            }
            if (minArrival != INT_MAX) {
                currentTime = minArrival;
            } else {
                break; // Hoàn thành
            }
            continue;
        }

        Process& p = procs[i];
        
        // B3: Tính toán thời gian chờ và cập nhật thời gian bắt đầu
        p.waiting += currentTime - p.lastRunTime;
        
        if (p.start == -1) {
            p.start = currentTime;
        }
        
        int executeTime = min(quantums[qLevel], p.remainingBurst);
        
        // B4: Thực thi
        currentTime += executeTime;
        p.remainingBurst -= executeTime;
        p.lastRunTime = currentTime; // Cập nhật thời gian cuối cùng rời CPU

        // B5: Phản hồi/Di chuyển
        if (p.remainingBurst > 0) {
            // Bị gián đoạn, hạ xuống queue thấp hơn (nếu chưa phải Q2)
            int nextLevel = min(qLevel + 1, 2);
            p.queueLevel = nextLevel;
            queues[nextLevel].push(i);
        } else {
            // Hoàn thành
            p.finish = currentTime;
            p.turnaround = p.finish - p.arrival;
            // p.waiting đã được tính trong B3
            result.push_back(p);
            completedCount++;
        }
    }
    
    return result;
}

void writeResultsToFile(const string& filename, 
                        const vector<Process>& fcfsResult,
                        const vector<Process>& sjfResult,
                        const vector<Process>& priorityResult,
                        const vector<Process>& rrResult,
                        const vector<Process>& mlqResult,
                        const vector<Process>& mlfqResult,
                        int quantum) {
    ofstream file(filename);
    
    if (!file.is_open()) {
        cout << "Khong the tao file output!" << endl;
        return;
    }
    
    file << "=== KET QUA MO PHONG THUAT TOAN LAP LICH CPU ===" << endl << endl;
    
    // FCFS
    file << "\n--- FCFS (First-Come, First-Served) ---" << endl;
    file << "PID\tArrival\tBurst\tStart\tFinish\tWaiting\tTurnaround" << endl;
    for (const auto& p : fcfsResult) {
        file << p.id << "\t" << p.arrival << "\t" << p.burst << "\t" 
             << p.start << "\t" << p.finish << "\t" << p.waiting << "\t" << p.turnaround << endl;
    }
    auto [avgW1, avgT1] = calculateAverages(fcfsResult);
    file << "Average Waiting Time: " << fixed << setprecision(2) << avgW1 << endl;
    file << "Average Turnaround Time: " << fixed << setprecision(2) << avgT1 << endl;
    
    // SJF
    file << "\n--- SJF (Shortest Job First) ---" << endl;
    file << "PID\tArrival\tBurst\tStart\tFinish\tWaiting\tTurnaround" << endl;
    for (const auto& p : sjfResult) {
        file << p.id << "\t" << p.arrival << "\t" << p.burst << "\t" 
             << p.start << "\t" << p.finish << "\t" << p.waiting << "\t" << p.turnaround << endl;
    }
    auto [avgW2, avgT2] = calculateAverages(sjfResult);
    file << "Average Waiting Time: " << fixed << setprecision(2) << avgW2 << endl;
    file << "Average Turnaround Time: " << fixed << setprecision(2) << avgT2 << endl;
    
    // Priority
    file << "\n--- Priority Scheduling ---" << endl;
    file << "PID\tArrival\tBurst\tPriority\tStart\tFinish\tWaiting\tTurnaround" << endl;
    for (const auto& p : priorityResult) {
        file << p.id << "\t" << p.arrival << "\t" << p.burst << "\t" << p.priority << "\t"
             << p.start << "\t" << p.finish << "\t" << p.waiting << "\t" << p.turnaround << endl;
    }
    auto [avgW3, avgT3] = calculateAverages(priorityResult);
    file << "Average Waiting Time: " << fixed << setprecision(2) << avgW3 << endl;
    file << "Average Turnaround Time: " << fixed << setprecision(2) << avgT3 << endl;
    
    // Round Robin
    file << "\n--- Round Robin (Quantum=" << quantum << ") ---" << endl;
    file << "PID\tArrival\tBurst\tStart\tFinish\tWaiting\tTurnaround" << endl;
    for (const auto& p : rrResult) {
        file << p.id << "\t" << p.arrival << "\t" << p.burst << "\t" 
             << p.start << "\t" << p.finish << "\t" << p.waiting << "\t" << p.turnaround << endl;
    }
    auto [avgW4, avgT4] = calculateAverages(rrResult);
    file << "Average Waiting Time: " << fixed << setprecision(2) << avgW4 << endl;
    file << "Average Turnaround Time: " << fixed << setprecision(2) << avgT4 << endl;
    
    // MLQ
    file << "\n--- MLQ (Multilevel Queue) ---" << endl;
    file << "PID\tArrival\tBurst\tQueue\tStart\tFinish\tWaiting\tTurnaround" << endl;
    for (const auto& p : mlqResult) {
        file << p.id << "\t" << p.arrival << "\t" << p.burst << "\t" << p.queueLevel << "\t"
             << p.start << "\t" << p.finish << "\t" << p.waiting << "\t" << p.turnaround << endl;
    }
    auto [avgW5, avgT5] = calculateAverages(mlqResult);
    file << "Average Waiting Time: " << fixed << setprecision(2) << avgW5 << endl;
    file << "Average Turnaround Time: " << fixed << setprecision(2) << avgT5 << endl;
    
    // MLFQ
    file << "\n--- MLFQ (Multilevel Feedback Queue) ---" << endl;
    file << "PID\tArrival\tBurst\tFinal_Queue\tStart\tFinish\tWaiting\tTurnaround" << endl;
    for (const auto& p : mlfqResult) {
        file << p.id << "\t" << p.arrival << "\t" << p.burst << "\t" << p.queueLevel << "\t"
             << p.start << "\t" << p.finish << "\t" << p.waiting << "\t" << p.turnaround << endl;
    }
    auto [avgW6, avgT6] = calculateAverages(mlfqResult);
    file << "Average Waiting Time: " << fixed << setprecision(2) << avgW6 << endl;
    file << "Average Turnaround Time: " << fixed << setprecision(2) << avgT6 << endl;
    
    // Phân tích
    file << "\n\n=== PHAN TICH ===" << endl;
    
    file << "\nFCFS:" << endl;
    file << "  Uu diem: Don gian, cong bang, khong gay starvation" << endl;
    file << "  Nhuoc diem: Convoy effect" << endl;
    
    file << "\nSJF:" << endl;
    file << "  Uu diem: Toi uu ve thoi gian cho trung binh" << endl;
    file << "  Nhuoc diem: Co the gay starvation cho tien trinh dai" << endl;
    
    file << "\nPriority:" << endl;
    file << "  Uu diem: Linh hoat, phu hop voi he thong co uu tien" << endl;
    file << "  Nhuoc diem: Co the gay starvation" << endl;
    
    file << "\nRound Robin:" << endl;
    file << "  Uu diem: Cong bang, responsive" << endl;
    file << "  Nhuoc diem: Context switch overhead" << endl;
    
    file << "\nMLQ (Multilevel Queue):" << endl;
    file << "  Uu diem: Phan loai ro rang, phu hop cho cac loai tien trinh khac nhau" << endl;
    file << "  Nhuoc diem: Khong linh hoat, tien trinh khong the chuyen queue" << endl;
    file << "  Cau truc: Q0(FCFS), Q1(RR q=2), Q2(RR q=4)" << endl;
    
    file << "\nMLFQ (Multilevel Feedback Queue):" << endl;
    file << "  Uu diem: Linh hoat, tu dong dieu chinh priority, phat hien I/O-bound vs CPU-bound" << endl;
    file << "  Nhuoc diem: Phuc tap, overhead cao" << endl;
    file << "  Cau truc: Q0(RR q=2), Q1(RR q=4), Q2(FCFS)" << endl;
    
    file.close();
    cout << "\n==> Da ghi ket qua vao file: " << filename << endl;
}

void compareAlgorithms(const vector<Process>& fcfsResult,
                       const vector<Process>& sjfResult,
                       const vector<Process>& priorityResult,
                       const vector<Process>& rrResult,
                       const vector<Process>& mlqResult,
                       const vector<Process>& mlfqResult,
                       int quantum) {
    cout << "\n\n========================================" << endl;
    cout << "  SO SANH TONG QUAN" << endl;
    cout << "========================================" << endl;
    
    auto [fcfsW, fcfsT] = calculateAverages(fcfsResult);
    auto [sjfW, sjfT] = calculateAverages(sjfResult);
    auto [prioW, prioT] = calculateAverages(priorityResult);
    auto [rrW, rrT] = calculateAverages(rrResult);
    auto [mlqW, mlqT] = calculateAverages(mlqResult);
    auto [mlfqW, mlfqT] = calculateAverages(mlfqResult);
    
    cout << left << setw(25) << "Thuat toan" 
         << setw(20) << "Avg Waiting" 
         << setw(20) << "Avg Turnaround" << endl;
    cout << string(65, '-') << endl;
    
    cout << left << setw(25) << "FCFS" 
         << setw(20) << fixed << setprecision(2) << fcfsW
         << setw(20) << fcfsT << endl;
    
    cout << left << setw(25) << "SJF" 
         << setw(20) << sjfW
         << setw(20) << sjfT << endl;
    
    cout << left << setw(25) << "Priority" 
         << setw(20) << prioW
         << setw(20) << prioT << endl;
    
    cout << left << setw(25) << ("Round Robin (q=" + to_string(quantum) + ")")
         << setw(20) << rrW
         << setw(20) << rrT << endl;
    
    cout << left << setw(25) << "MLQ" 
         << setw(20) << mlqW
         << setw(20) << mlqT << endl;
    
    cout << left << setw(25) << "MLFQ" 
         << setw(20) << mlfqW
         << setw(20) << mlfqT << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "  MO PHONG THUAT TOAN LAP LICH CPU" << endl;
    cout << "========================================" << endl;
    
    string inputFile = "data1.txt";
    cout << "\nDoc du lieu tu file: " << inputFile << endl;
    
    vector<Process> processes = readProcesses(inputFile);
    
    if (processes.empty()) {
        cout << "Khong co du lieu tien trinh!" << endl;
        return 1;
    }
    
    int quantum;
    cout << "Nhap time quantum cho Round Robin (vi du: 2): ";
    cin >> quantum;
    
    cout << "\n\nBat dau mo phong..." << endl;
    
    // Chạy các thuật toán
    vector<Process> fcfsResult = fcfs(processes);
    vector<Process> sjfResult = sjf(processes);
    vector<Process> priorityResult = priorityScheduling(processes);
    vector<Process> rrResult = roundRobin(processes, quantum);
    vector<Process> mlqResult = mlq(processes);
    vector<Process> mlfqResult = mlfq(processes);

    printResults("FCFS (First-Come, First-Served)", fcfsResult);
    printResults("SJF (Shortest Job First)", sjfResult);
    printResults("Priority Scheduling", priorityResult, true);
    printResults("Round Robin (Quantum=" + to_string(quantum) + ")", rrResult);
    printResults("MLQ (Multilevel Queue)", mlqResult, false, true);
    printResults("MLFQ (Multilevel Feedback Queue)", mlfqResult, false, true);
    
    compareAlgorithms(fcfsResult, sjfResult, priorityResult, rrResult, mlqResult, mlfqResult, quantum);
    
    writeResultsToFile("output1.txt",
                       fcfsResult,
                       sjfResult,
                       priorityResult,
                       rrResult,
                       mlqResult,
                       mlfqResult,
                       quantum);
    cout<<"da ghi ket qua thanh cong vao file output1.txt"<<endl;
    return 0;

}
