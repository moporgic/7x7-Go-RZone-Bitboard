#include "Zone7x7Bitboard.h"
#include <string>
#include <sstream>
#include <functional>

using u64 = Zone7x7Bitboard::u64;

struct number {
	u64 x;
	number(u64 x = 0) : x(x) {}
	operator u64&() { return x; }

	friend std::istream& operator >>(std::istream& in, number&& n) {
		std::string num;
		in >> num;
		if (num.find("0b") == std::string::npos) {
			try {
				n.x = std::stoull(num, nullptr, 0);
			} catch (std::exception&) {
				in.setstate(std::ios::failbit);
			}
		} else {
			num.erase(0, 2);
			n.x = 0;
			for (char digit : num) {
				n.x = (n.x << 1) | (digit - '0');
			}
		}
		return in;
	}
};

int main(int argc, const char* argv[]) {
	number zone = 0, black = 0, white = 0;
	while (std::cin >> zone >> black >> white) {
		Zone7x7Bitboard s(zone, black, white), z = s;
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
