/// \file Semaphore.h
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 09.06.17

#ifndef MTFS_SEMAPHORE_H
#define MTFS_SEMAPHORE_H


#include <mutex>
#include <condition_variable>

class Semaphore {
public:
	Semaphore(int val = 0);
	~Semaphore();
	void wait();
	void notify();
private:
	int v;
	std::mutex m;
	std::condition_variable cv;
};


#endif //MTFS_SEMAPHORE_H
