#include "task.h"
#include "parser.h"
#include "structs.h"
#include "derivator.h"
#include "integrator.h"
#include "fitter.h"
#include "settings.h"

Task::Task(graph *data) :
	data_(data),
	done_(0)
{

}

Task::~Task() {

}

int Task::isDone() {
	int result = 0;
	result = done_;
	// Therefore, result 0 may mean that it's being manipulated
	return result;
}

std::string Task::whatHappened() {
	std::string result = "working...";
	result = error_;
	result += " at " + data_->name;
	// It should not be accessed without being done or while
	// reading something else
	return result;
}

graph* Task::getData() {
	if (isDone()) return data_;
	else return nullptr;
}

void Task::setDone(int value) {
	done_ = value;
}


TaskLoad::TaskLoad(graph* data) :
	Task(data)
{

}

TaskLoad::~TaskLoad() {

}

void TaskLoad::perform() {
	error_ = parseFile(*data_);
	if (error_.empty()) setDone(1);
	else setDone(-1);
}

bool TaskLoad::canPerformNow() {
	return true; // No prerequisites
}

TaskDerivative::TaskDerivative(graph* data) :
	Task(data)
{

}

TaskDerivative::~TaskDerivative() {

}

void TaskDerivative::perform() {
	error_ = differentiate(data_->base[UP], data_->derivative[UP], data_);
	if (error_.empty())
		error_ = differentiate(data_->base[DOWN], data_->derivative[DOWN], data_);
	if (error_.empty()) setDone(1);
	else setDone(-1);
}

bool TaskDerivative::canPerformNow() {
	if (!data_->base[UP].points || !data_->base[DOWN].points)
		return false; // Later
	return true;
}

TaskIntegral::TaskIntegral(graph* data) :
	Task(data)
{

}

TaskIntegral::~TaskIntegral() {

}

void TaskIntegral::perform() {
	error_ = integrate(Settings::get().integrateSmoothed ?
				data_->raw[UP] : data_->base[UP], data_->integral[UP], data_);
	if (error_.empty())
		error_ = integrate(Settings::get().integrateSmoothed ?
				data_->raw[DOWN] : data_->base[DOWN], data_->integral[DOWN], data_);
	if (error_.empty()) setDone(1);
	else setDone(-1);
}

bool TaskIntegral::canPerformNow() {
	if (!data_->base[UP].points || !data_->base[DOWN].points)
		return false; // Later
	return true;
}

TaskFit::TaskFit(graph* data) :
	Task(data)
{
}

TaskFit::~TaskFit() {

}

void TaskFit::perform() {
	error_ = fit(data_->base[DOWN], data_->fitted[DOWN], data_, DOWN);
	error_ = fit(data_->base[UP], data_->fitted[UP], data_, UP);
	if (error_.empty()) setDone(1);
	else setDone(-1);
}

bool TaskFit::canPerformNow() {
	if (!data_->base[UP].points || !data_->base[DOWN].points)
		return false; // Later
	return true;
}
