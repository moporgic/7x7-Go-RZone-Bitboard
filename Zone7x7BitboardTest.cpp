#include "Zone7x7Bitboard.h"
#include <sstream>

int main(int argc, const char* argv[]) {
	using board = Zone7x7Bitboard;
	using u64 = board::u64;

	board s;
	auto bits = [](u64 x) -> std::string {
		std::stringstream ss;
		int n = 0;
		do {
			ss << ((x & 1) ? '1' : '0');
			if (++n % 7 == 0) ss << ' ';
		} while ((x >>= 1) != 0);
		return ss.str();
	};
	u64 where, black, white;
	while (std::cin >> where >> black >> white) {
		std::cout << bits(where) << std::endl;
		std::cout << bits(black) << std::endl;
		std::cout << bits(white) << std::endl;
		s = {where, black, white};
		board z = s;
		z.minimize();
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
