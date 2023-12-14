#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <set>

int kSideLength = 10;
int kRows;
int kCols;
std::string red = "\x1B[41m";
std::string green = "\x1B[42m";
int n = 4;
std::set<std::thread::id> threads_id;
bool displayMod;

bool isStable(const std::vector<std::vector<bool> >& current, const std::vector<std::vector<bool> >& next) {
  for (int i = 0; i < kRows; ++i) {
    for (int j = 0; j < kCols; ++j) {
      if (current[i][j] != next[i][j]) {
        return false;
      }
    }
  }
  return true;
}

void display(const std::vector<std::vector<std::vector<bool> > >& boards, std::string color) {
  const std::string kReset = "\x1B[0m";
  const std::string kRed = "\x1B[41m";
  const std::string kWhite = "\x1B[47m";

  for (int i = 0; i < kRows; ++i) {
    for (int sim = 0; sim < boards.size(); ++sim) {
      for (int j = 0; j < kCols; ++j) {
        if (boards[sim][i][j]) {
          std::cout << color << " " << kReset;
        } else {
          std::cout << kWhite << " " << kReset;
        }
      }
      if (sim < boards.size() - 1) {
        std::cout << '\t';
      }
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

void computenextGeneration(std::vector<std::vector<bool> >& current, std::vector<std::vector<bool> >& next) {
  for (int i = 0; i < kRows; ++i) {
    for (int j = 0; j < kCols; ++j) {
      int liveNeighbors = 0;

      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          if (dx == 0 && dy == 0) continue;
          int ni = i + dx;
          int nj = j + dy;

          if (ni >= 0 && ni < kRows && nj >= 0 && nj < kCols) {
            if (current[ni][nj]) {
              liveNeighbors++;
            }
          }
        }
      }

      if (current[i][j]) {
        if (liveNeighbors < 2 || liveNeighbors > 3) {
          next[i][j] = false;
        } else {
          next[i][j] = true;
        }
      } else {
        if (liveNeighbors == 3) {
          next[i][j] = true;
        }
      }
    }
  }
}

int main() {
  std::cout << "Size: ";
  std::cin >> kSideLength;

  std::cout << "Amount between 1 and 4: ";
  std::cin >> n;

  char mod;
  std::cout << "Display mod [Y] / [N]: ";
  std::cin >> mod;
  displayMod = (mod == 'Y');

  std::cout << "Main thread ID: " << std::this_thread::get_id() << "\n" << "\n<==========================>\n\n";

  kRows = kSideLength;
  kCols = kSideLength;

  std::vector<std::vector<bool> > tmpCurrent(kRows, std::vector<bool>(kCols, false));
  std::vector<std::vector<bool> > tmpNext(kRows, std::vector<bool>(kCols, false));

  std::vector<std::vector<std::vector<bool> > > currentGenerationThreads(n, tmpCurrent);
  std::vector<std::vector<std::vector<bool> > > nextGenerationThreads(n, tmpNext);

  std::vector<std::vector<std::vector<bool> > > currentGenerationBySteps(n, tmpCurrent);
  std::vector<std::vector<std::vector<bool> > > nextGenerationBySteps(n, tmpNext);

  std::srand(std::time(nullptr));

  for (int k = 0; k < n; k++) {
    for (int i = 0; i < kRows; ++i) {
      for (int j = 0; j < kCols; ++j) {
        currentGenerationThreads[k][i][j] = std::rand() % 2;
        nextGenerationThreads[k][i][j] = !currentGenerationThreads[k][i][j];
      }
    }
  }
  currentGenerationBySteps = currentGenerationThreads;
  nextGenerationBySteps = nextGenerationThreads;

  auto start_time_threads = std::chrono::high_resolution_clock::now();
  int i = 0;
  while (true) {
    ++i;
    if (displayMod) display(currentGenerationThreads, red);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    std::vector<std::thread> threads(n);
    for (int sim = 0; sim < n; ++sim) {
      threads[sim] = std::thread(computenextGeneration,
                      std::ref(currentGenerationThreads[sim]),
                      std::ref(nextGenerationThreads[sim]));
      threads_id.insert(threads[sim].get_id());
    }
    for (int sim = 0; sim < n; ++sim) {
      threads[sim].join();
    }

    bool allStable = true;
    for (int sim = 0; sim < n; ++sim) {
      if (!isStable(currentGenerationThreads[sim], nextGenerationThreads[sim])) {
        allStable = false;
        break;
      }
    }

    if (allStable) {
      std::cout << "All fields have stabilized. Exiting the program." << '\n';
      break;
    }

    if (i > 300) {
      std::cout << "Too many generations. Exiting the program." << '\n';
      break;
    }

    for (int sim = 0; sim < n; ++sim) {
      currentGenerationThreads[sim] = nextGenerationThreads[sim];
    }
  }
  auto end_time_threads = std::chrono::high_resolution_clock::now();
  auto execution_time = end_time_threads - start_time_threads;
  std::cout << "Threads time: " << execution_time.count() / 1e9 << " seconds" << '\n';

  std::cout << "Thread IDs used in the program:" << '\n';
  for (const std::thread::id& id : threads_id) {
    std::cout << id << '\n';
  }

  std::cout << "\n<==========================>\n\n";
  auto start_time_steps = std::chrono::high_resolution_clock::now();
  threads_id.clear();
  i = 0;
  while (true) {
    ++i;
    if (displayMod) display(currentGenerationBySteps, green);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    for (int sim = 0; sim < n; ++sim) {
      computenextGeneration(currentGenerationBySteps[sim], nextGenerationBySteps[sim]);
      threads_id.insert(std::this_thread::get_id());
    }

    bool allStable = true;
    for (int sim = 0; sim < n; ++sim) {
      if (!isStable(currentGenerationBySteps[sim], nextGenerationBySteps[sim])) {
        allStable = false;
        break;
      }
    }

    if (allStable) {
      std::cout << "All fields have stabilized. Exiting the program." << '\n';
      break;
    }

    if (i > 300) {
      std::cout << "Too many generations. Exiting the program." << '\n';
      break;
    }

    for (int sim = 0; sim < n; ++sim) {
      currentGenerationBySteps[sim] = nextGenerationBySteps[sim];
    }
  }
  auto end_time_steps = std::chrono::high_resolution_clock::now();
  execution_time = end_time_steps - start_time_steps;
  std::cout << "Steps time: " << execution_time.count() / 1e9 << " seconds" << '\n';

  std::cout << "Thread IDs used in the program:" << '\n';
  for (const std::thread::id& id : threads_id) {
    std::cout << id << '\n';
  }

  return 0;
}
