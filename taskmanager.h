#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <list>

class Task;

class TaskManager
{
public:
    static TaskManager& getInstance()
    {
        static TaskManager instance;
        return instance;
    }

    void addTask(Task* added);
    void work();

private:
    TaskManager();

    std::list<Task*> tasks;

    TaskManager(TaskManager const&) = delete;
    void operator=(TaskManager const&) = delete;
};

#endif // TASKMANAGER_H
