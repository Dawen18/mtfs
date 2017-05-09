/**
 * @author David Wittwer
 * @date 30.04.17.
**/

#include <thread>
#include <iostream>
#include <mtfs/Mtfs.h>

#define _GNU_SOURCE
#define FUSE_USE_VERSION 30

using namespace std;
using namespace mtfs;

void th1(int i) {
	int loc = 0;
	std::cout << "thread " << i << "\n";
	for (int j = 0; j < 10; ++j) {
		std::this_thread::sleep_for(std::chrono::milliseconds(i * 100));
		loc += i;
		std::cout << loc << "\n";
	}
}

int main(int argc, char **argv) {

	Mtfs *mtfs = Mtfs::getInstance();

	mtfs->start();
	mtfs->join();

	cout << "Thread ended." << endl;

	return 0;
}