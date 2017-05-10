/**
 * @author David Wittwer
 * @date 09.05.17.
**/

#ifndef TRAVAIL_BACHELOR_QUEUE_H
#define TRAVAIL_BACHELOR_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadQueue {

public:
	ThreadQueue() {}

	~ThreadQueue() {
		deque.clear();
	}

	void push(T value) {
		std::unique_lock<std::mutex> lock(mutex);
		deque.push_back(value);
		lock.unlock();
		condV.notify_one();
	}

	bool hasData() {
		std::lock_guard<std::mutex> guard(mutex);
		return !deque.empty();
	}

	T front() {
		T d;

		std::unique_lock<std::mutex> lock(mutex);

		while (deque.empty())
			condV.wait(lock);

		d = deque.front();
		lock.unlock();

		return d;
	}

	void pop() {

		std::unique_lock<std::mutex> lock(mutex);
		if (!deque.empty())
			deque.pop_front();
	}

private:
	std::deque<T> deque;
	std::mutex mutex;
	std::condition_variable condV;


};


#endif //TRAVAIL_BACHELOR_QUEUE_H
