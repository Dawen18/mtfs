/**
 * \file main.cpp
 * \brief
 * \author David Wittwer
 * \version 0.0.1
 * \copyright GNU Publis License V3
 *
 * This file is part of MTFS.

    MTFS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <boost/filesystem.hpp>
#include <fcntl.h>

#define FIRST_SIZE 512
#define LAST_SIZE 131072
#define SAMPLE 5
#define LOOP 50

#define FILENAME "/tmp/mtfs/test24"

using namespace std;

int main(int argc, char **argv) {
	cout << "Start Mtfs perfs tests" << endl;

	ofstream file("sample.csv");

	creat(FILENAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	file << "octets ";
	for (int i = 1; i <= LOOP; ++i) {
		file << to_string(i) << " ";
	}
	file << endl;

	string readTable, writeTable;

	for (size_t i = FIRST_SIZE; i <= LAST_SIZE; i *= 2) {
		string readRow, writeRow;
		readRow = to_string(i);
		writeRow = to_string(i);

		for (int j = 0; j < LOOP; ++j) {
			char buffer[i + 1];
			char *rbuffer = static_cast<char *>(calloc(sizeof(char), i + 1));
			for (int k = 0; k < i; ++k) {
				buffer[k] = 'a';
			}

			clock_t writeSum = 0;
			clock_t readSum = 0;

			for (int l = 0; l < SAMPLE; ++l) {
				int fd = open(FILENAME, O_SYNC | O_RDWR);
				if (fd <= 0) {
					cerr << "open error " << strerror(errno) << endl;
					break;
				}
				ssize_t nb = 0;
				clock_t start;

				start = clock();
				do {
					nb += write(fd, buffer, i);
				} while (nb < i);
				writeSum += clock() - start;

				if (nb < 0) {
					cerr << "write error " << strerror(errno) << endl;
					break;
				}

				lseek(fd, 0, SEEK_SET);

				start = clock();
				do {
					nb += read(fd, buffer, i);
				} while (nb < i);
				readSum += clock() - start;
				close(fd);
			}
			float wm = (float) writeSum / SAMPLE;
			float rm = (float) readSum / SAMPLE;
//			cout << to_string(m) << " " << to_string((m / CLOCKS_PER_SEC) * 1000) << endl;
			writeRow += " " + to_string(wm);
			readRow += " " + to_string(rm);
//			file << to_string(m) << " ";
		}

		writeTable += writeRow + "\n";
		readTable += readRow + "\n";
		cout << "size " << to_string(i) << endl;

	}

	file << readTable << endl << writeTable << endl;

}