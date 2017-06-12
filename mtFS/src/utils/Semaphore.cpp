
#include "utils/Semaphore.h"

using namespace std;

Semaphore::Semaphore(int val) : v(val) {}

Semaphore::~Semaphore() {
	this->cv.notify_all();
}

void Semaphore::wait() {
	unique_lock<std::mutex> lk(this->m);
	while (this->v <= 0)
		this->cv.wait(lk);
	this->v--;
	lk.unlock();
}

void Semaphore::notify() {
	unique_lock<mutex> lk(this->m);
	this->v++;
	lk.unlock();
	this->cv.notify_one();
}
/// \file Semaphore.cpp
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 09.06.17

#include "utils/Semaphore.h"
