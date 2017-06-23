#include "taskmanager.h"
#include "task.h"
#include "log.h"
#include <thread>
#include <chrono>
#include <iostream>
#include "settings.h"
#include <ctime>

TaskManager::TaskManager()
{

}

void TaskManager::addTask(Task* added) {
    tasks.push_back(added);
}

void TaskManager::work() {
    clock_t begin = clock();

    int threads = Settings::get().cores;
    Task* done[threads];
    std::thread* thread[threads];
    for (int i = 0; i < threads; i++)
        done[i] = nullptr;
    bool somethingDone = true;
    bool possibleDeadlock = false;
    while ((!tasks.empty() && !possibleDeadlock) || somethingDone) {
        somethingDone = false;
        possibleDeadlock = false;
        auto work = [] (Task* task) -> void {
            task->perform();
        };
        auto addTask = [&] (int i) -> void {
            unsigned int skips = 0;
            while (!tasks.front()->canPerformNow()) {
                tasks.push_back(tasks.front());
                tasks.pop_front();
                skips++;
				//std::cerr << "Looking for a doable task\n";
                if (skips >= tasks.size()) {
                    possibleDeadlock = true;
                    done[i] = nullptr;
                    return;
                }
            }
            done[i] = tasks.front();
			std::cerr << "Added task " << done[i] << std::endl;
            thread[i] = new std::thread(work, done[i]);
            somethingDone = true;
            tasks.pop_front();
        };
		for (int i = 0; i < threads; i++) {
            if (done[i]) {
                int isDone = done[i]->isDone();
                if (isDone != 0) {
                    if (isDone == -1) {
                        std::cerr << "Task failed: " <<
                           done[i]->whatHappened() << std::endl;
                    }
					if (!thread[i]->joinable())
						throw(std::runtime_error("Thread not joinable"));
					thread[i]->join();
					delete thread[i];
                    thread[i] = nullptr;
                    delete done[i];
                    if (!tasks.empty()) {
                        addTask(i);
					} else done[i] = nullptr;
				} else {
					somethingDone = true;
				}
			} else {
                if (!tasks.empty()) {
                    addTask(i);
				};
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (!tasks.empty()) {
        std::cerr << "Deadlock detected, " << tasks.size()
                 << " tasks can't be finished" << std::endl;
        for (auto it = tasks.begin(); it != tasks.end(); ++it)
            delete *it;
        tasks.clear();
    }

    clock_t end = clock();
    double elapsed = double(end - begin) / CLOCKS_PER_SEC;
	Log::write("Tasks took " + to_string(elapsed) + " seconds.\n");
}

