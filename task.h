#ifndef TASK_H
#define TASK_H

#include <mutex>
#include <condition_variable>
#include <atomic>

struct graph;
struct curve;

class Task {
public:
    virtual ~Task();
    virtual void perform() = 0;
    virtual bool canPerformNow() = 0;

    int isDone();
    std::string whatHappened();
    graph* getData();

protected:
    Task(graph* data);
    void setDone(int value);

    graph* data_;
    std::atomic_short done_;
    std::string error_;
    std::mutex mutex;
    std::condition_variable condvar;
};

class TaskLoad : public Task {
public:
    TaskLoad(graph* data);
    virtual ~TaskLoad();
    virtual void perform();
    virtual bool canPerformNow();
};

class TaskDerivative : public Task {
public:
    TaskDerivative(graph* data);
    virtual ~TaskDerivative();
    virtual void perform();
    virtual bool canPerformNow();
};

class TaskIntegral : public Task {
public:
	TaskIntegral(graph* data);
	virtual ~TaskIntegral();
	virtual void perform();
	virtual bool canPerformNow();
};

class TaskFit : public Task {
public:
	TaskFit(graph* data);
	virtual ~TaskFit();
	virtual void perform();
	virtual bool canPerformNow();
};


#endif // TASK_H
