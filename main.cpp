#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <rapidcheck.h>
#include <set>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

// Измерение времени
class Timer {
public:
  Timer() { start = std::chrono::high_resolution_clock::now(); }
  void stop() {
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count();
  }
  long long ms() const { return elapsed; }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  long long elapsed = 0;
};

// Измерение памяти
size_t getMemoryUsageKb() {
#ifdef _WIN32
  PROCESS_MEMORY_COUNTERS memCounter;
  GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
  return memCounter.WorkingSetSize / 1024;
#else
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return usage.ru_maxrss;
#endif
}

// Функция пузырьковой сортировки для тестирования
void bubbleSort(std::vector<int> &arr) {
  int n = arr.size();
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        std::swap(arr[j], arr[j + 1]);
      }
    }
  }
}

int main() {
  Timer timer;

  setlocale(LC_ALL, "Russian");

  rc::check("Корректная сортировка", [](std::vector<int> vec) {
    std::vector<int> original = vec;
    std::sort(vec.begin(), vec.end());

    RC_ASSERT(vec.size() == original.size());

    RC_ASSERT(std::is_sorted(vec.begin(), vec.end()));

    std::vector<int> copy = vec;
    std::sort(copy.begin(), copy.end());
    RC_ASSERT(copy == vec);
  });

  timer.stop();

  size_t memKb = getMemoryUsageKb();

  std::cout << "\n=== Benchmark Results ===\n";
  std::cout << "Time: " << timer.ms() << " ms\n";
  std::cout << "Memory: " << memKb << " KB\n";

  return 0;
}
