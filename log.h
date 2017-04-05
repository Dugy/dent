#ifndef LOG_H
#define LOG_H

#include <string>
#include <sstream>
#include <mutex>
#include <ctime>

struct Log
{

	static Log& get() {
		static Log instance;
		return instance;
	}
	static void write(const std::string& added) {
		time_t start = time(NULL);
		Log& got = get();
		while (!got.mutex.try_lock())
			if (difftime(time(NULL), start) > 2)
				throw(std::runtime_error("Could not lock report mutex"));
		got.contents.append(added);
		got.mutex.unlock();
	}
	static std::string& read() {
		return get().contents;
	}

	void clear() {
		contents.clear();
	}

private:
	Log();
	~Log();

	std::mutex mutex;
	std::string contents;
	Log(Log const&) = delete;
	void operator=(Log const&) = delete;
};

#include <iostream>

class logStream {
public:
	logStream() { }
	~logStream() {
		Log::write(stream.str());
	}
	template <class T>
	std::stringstream& operator<< (T input) {
		stream << input;
		return stream;
	}

private:
	std::stringstream stream;
};

#endif // LOG_H
