#include "Zone7x7Bitboard.h"
#include <string>
#include <sstream>
#include <functional>

using board = Zone7x7Bitboard;
using u64 = board::u64;

int main(int argc, const char* argv[]) {
	u64 where, black, white;
	while (std::cin >> where >> black >> white) {
		// for (u64 x : {where, black, white}) {
		// 	int n = 0;
		// 	do {
		// 		std::cout << ((x & 1) ? '1' : '0');
		// 		if (++n % 7 == 0) std::cout << ' ';
		// 	} while ((x >>= 1) != 0);
		// 	std::cout << std::endl;
		// }
		board s = {where, black, white}, z = s;
		z.normalize();
		std::stringstream ss, sz;
		ss << s;
		sz << z;
		std::string sline, zline;
		for (int i = 0; std::getline(ss, sline) && std::getline(sz, zline); i++) {
			const char* padding = i != 4 ? "     " : " >>> ";
			std::cout << sline << padding << zline << std::endl;
		}
	}
	return 0;
}
