#pragma once
#include <iostream>
#include <algorithm>

class Zone7x7Bitboard {
public:
	using u64 = unsigned long long int;
	using u32 = unsigned int;

	u64 zone;
	u64 black;
	u64 white;

public:
	inline constexpr Zone7x7Bitboard(u64 zone = 0, u64 black = 0, u64 white = 0) : zone(zone), black(black), white(white) {}
	inline constexpr Zone7x7Bitboard(const Zone7x7Bitboard&) = default;
	inline constexpr Zone7x7Bitboard& operator =(const Zone7x7Bitboard&) = default;

	inline constexpr Zone7x7Bitboard& operator &=(const Zone7x7Bitboard& z) { return operator =(operator &(z)); }
	inline constexpr Zone7x7Bitboard& operator |=(const Zone7x7Bitboard& z) { return operator =(operator |(z)); }
	inline constexpr Zone7x7Bitboard& operator ^=(const Zone7x7Bitboard& z) { return operator =(operator ^(z)); }
	inline constexpr Zone7x7Bitboard operator &(const Zone7x7Bitboard& z) const { return {zone & z.zone, black & z.black, white & z.white}; }
	inline constexpr Zone7x7Bitboard operator |(const Zone7x7Bitboard& z) const { return {zone | z.zone, black | z.black, white | z.white}; }
	inline constexpr Zone7x7Bitboard operator ^(const Zone7x7Bitboard& z) const { return {zone ^ z.zone, black ^ z.black, white ^ z.white}; }
	inline constexpr Zone7x7Bitboard operator ~() const { return {~zone & (-1ull >> 15), ~black & (-1ull >> 15), ~white & (-1ull >> 15)}; }

	inline constexpr Zone7x7Bitboard& operator &=(u64 x) { return operator =(operator &(x)); }
	inline constexpr Zone7x7Bitboard& operator |=(u64 x) { return operator =(operator |(x)); }
	inline constexpr Zone7x7Bitboard& operator ^=(u64 x) { return operator =(operator ^(x)); }
	inline constexpr Zone7x7Bitboard& operator <<=(u32 i) { return operator =(operator <<(i)); }
	inline constexpr Zone7x7Bitboard& operator >>=(u32 i) { return operator =(operator >>(i)); }
	inline constexpr Zone7x7Bitboard operator &(u64 x) const { return {zone & x, black & x, white & x}; }
	inline constexpr Zone7x7Bitboard operator |(u64 x) const { return {zone | x, black | x, white | x}; }
	inline constexpr Zone7x7Bitboard operator ^(u64 x) const { return {zone ^ x, black ^ x, white ^ x}; }
	inline constexpr Zone7x7Bitboard operator <<(u32 i) const { return {zone << i, black << i, white << i}; }
	inline constexpr Zone7x7Bitboard operator >>(u32 i) const { return {zone >> i, black >> i, white >> i}; }

	inline constexpr bool operator ==(const Zone7x7Bitboard& z) const { return (zone == z.zone) & (black == z.black) & (white == z.white); }
	inline constexpr bool operator < (const Zone7x7Bitboard& z) const {
		return (zone != z.zone) ? (zone < z.zone) : ((black != z.black) ? (black < z.black) : (white < z.white));
	}
	inline constexpr bool operator !=(const Zone7x7Bitboard& z) const { return !(*this == z); }
	inline constexpr bool operator > (const Zone7x7Bitboard& z) const { return  (z < *this); }
	inline constexpr bool operator <=(const Zone7x7Bitboard& z) const { return !(z < *this); }
	inline constexpr bool operator >=(const Zone7x7Bitboard& z) const { return !(*this < z); }

public:
	enum PieceType {
		ZONE_EMPTY   = 0b000u,
		ZONE_BLACK   = 0b001u,
		ZONE_WHITE   = 0b010u,
		ZONE_UNKNOWN = 0b011u,
		IRRELEVANT   = 0b100u,
	};
	inline constexpr PieceType get(u32 x, u32 y) const {
		u64 mask = 1ull << (y * 7 + x);
		return static_cast<PieceType>((black & mask ? 1 : 0) | (white & mask ? 2 : 0) | (zone & mask ? 0 : 4));
	}
	inline constexpr void set(u32 x, u32 y, u32 type) {
		u64 mask = 1ull << (y * 7 + x);
		black = (type & PieceType::ZONE_BLACK) ? (black | mask) : (black & ~mask);
		white = (type & PieceType::ZONE_WHITE) ? (white | mask) : (white & ~mask);
		zone  = (type & PieceType::IRRELEVANT) ? (zone & ~mask) : (zone | mask);
	}

public:
	inline constexpr void transpose() {
		zone = transpose(zone);
		black = transpose(black);
		white = transpose(white);
	}
	inline constexpr void flip() {
		zone = flip(zone);
		black = flip(black);
		white = flip(white);
	}
	inline constexpr void mirror() {
		zone = mirror(zone);
		black = mirror(black);
		white = mirror(white);
	}
	inline constexpr void transform(u32 i) {
		zone = Isomorphisms::transform(zone, i);
		black = Isomorphisms::transform(black, i);
		white = Isomorphisms::transform(white, i);
	}

public:
	inline constexpr void minimize() {
		u32 offset = 0;
		if ((zone & 0b1111111000000000000000000000000000000000000000000ull) == 0) {
			offset += ((__builtin_ctzll(zone | (1ull << 49)) / 7 ?: 1) - 1) * 7;
		}
		if ((zone & 0b1000000100000010000001000000100000010000001000000ull) == 0) {
			u64 squz = zone;
			squz = squz | (squz >> 21);
			squz = squz | (squz >> 14);
			squz = squz | (squz >> 7);
			offset += ((__builtin_ctzll(squz | (0b10000000)) ?: 1) - 1);
		}
		*this >>= offset;
	}
	inline constexpr void normalize() {
		Isomorphisms isoz = zone, isob = black, isow = white;
		minimize();
		for (u32 i = 1; i < 8; i++) {
			Zone7x7Bitboard iso(isoz[i], isob[i], isow[i]);
			iso.minimize();
			if (iso < *this) *this = iso;
		}
	}

protected:
	static inline constexpr u64 transpose(u64 x) {

//		(1)                             (2)                             (3)                             (4)                             (5)
//		q r s t u v w   0 1 0 0 0 1 0   q l s t u p w   1 0 0 0 1 0 0   e l s t i p w   0 0 0 1 0 0 0   e l s b i p w   1 1 1 0 0 0 0   G N U b i p w
//		j k l m n o p   1 0 * 0 1 0 *   d k r m h o v   0 0 0 1 0 0 0   d k r a h o v   0 0 0 0 0 0 0   d k r a h o v   1 1 1 0 0 0 0   F M T a h o v
//		c d e f g h i   0 * 0 1 0 * 0   c j e Z g n i   0 0 * 0 0 0 *   c j q Z g n u   0 0 0 0 0 0 0   c j q Z g n u   1 1 1 0 0 0 0   E L S Z g n u
//		V W X Y Z a b   0 0 1 0 * 0 0   V W R Y f a b   0 1 0 0 0 * 0   V K R Y f m b   1 0 0 0 0 0 *   D K R Y f m t   0 0 0 0 0 0 0   D K R Y f m t
//		O P Q R S T U   0 1 0 * 0 1 0   O J Q X S N U   1 0 0 0 1 0 0   C J Q X G N U   0 0 0 0 0 0 0   C J Q X G N U   0 0 0 0 * * *   C J Q X e l s
//		H I J K L M N   1 0 * 0 1 0 *   B I P K F M T   0 0 0 * 0 0 0   B I P W F M T   0 0 0 0 0 0 0   B I P W F M T   0 0 0 0 * * *   B I P W d k r
//		A B C D E F G   0 * 0 0 0 * 0   A H C D E L G   0 0 * 0 0 0 *   A H O D E L S   0 0 0 * 0 0 0   A H O V E L S   0 0 0 0 * * *   A H O V c j q

		u64 z = x; // (1)
		z = z ^ (z << 6);
		z = z & 0b0100010001000100010000000100010001000100010000000ull;
		x = z = x ^ (z | (z >> 6)); // (2)
		z = z ^ (z << 12);
		z = z & 0b0010001000100000000000000010001000100000000000000ull;
		x = z = x ^ (z | (z >> 12)); // (3)
		z = z ^ (z << 18);
		z = z & 0b0001000000000000000000000001000000000000000000000ull;
		x = z = x ^ (z | (z >> 18)); // (4)
		z = z ^ (z << 24);
		z = z & 0b0000111000011100001110000000000000000000000000000ull;
		x = x ^ (z | (z >> 24)); // (5)
		return x;
	}
	static inline constexpr u64 flip(u64 x) {

//      (1)               (2)                                                   (3)               (4)
//		0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   0 1 0 0 0 0 0 0   B A A A A A A A   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0
//		G 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   A A B B B B B B   A 0 0 0 0 0 0 0   A 0 0 0 0 0 0 0
//		F F G G G G G G   0 0 0 0 0 0 0 0   0 0 0 1 0 0 0 0   0 0 0 A A A A A   B B A A A A A A   B B A A A A A A
//		E E E F F F F F   E E E 0 0 0 0 0   0 0 0 0 0 0 0 0   E E E E 0 0 0 0   A A A B B B B B   0 0 0 B B B B B
//		D D D D E E E E   0 0 0 0 E E E E   0 0 0 0 0 0 0 0   0 0 0 0 0 E E E   0 0 0 0 A A A A   0 0 0 0 0 0 0 0
//		C C C C C D D D   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   E E E E E 0 0 0   E E E E E 0 0 0
//		B B B B B B C C   B B B B B B 0 0   0 0 0 0 0 0 0 0   B B B B B B B 0   0 0 0 0 0 0 E E   0 0 0 0 0 0 E E
//		A A A A A A A B   A A A A A A A B   0 1 0 0 0 0 0 0   0 A A A A A A A   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0

		u64 p = x; // (1)
		p = p & 0x00000007f0003fffull; // (2)
		p = p * 0x0200080000000002ull >> 15; // (3)
		p = p & 0x0001fff8001fc000ull; // (4)

//      (5)               (6)               (7)                                                   (8)               (9)
//		0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   0 1 0 0 0 0 0 0   0 L L L L L L L   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0
//		G 0 0 0 0 0 0 0   K 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   K K 0 0 0 0 0 0   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0
//		F F G G G G G G   M M K K K K K K   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   M M M K K K K K   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0
//		0 0 0 F F F F F   0 0 0 M M M M M   K K K 0 0 0 0 0   0 0 0 0 0 0 0 0   K K K K M M M M   L L L 0 0 0 0 0   L L L 0 0 0 0 0
//		0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   M M M M K K K K   0 0 0 0 0 0 0 0   M M M M M K K K   0 0 0 0 L L L L   0 0 0 0 L L L L
//		C C C C C 0 0 0   L L L L L 0 0 0   0 0 0 0 0 M M M   0 0 0 0 0 0 0 0   L L L L L L M M   K K K K K 0 0 0   0 0 0 0 0 0 0 0
//		0 0 0 0 0 0 C C   0 0 0 0 0 0 L L   0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 1   0 0 0 0 0 0 0 L   M M M M M M K K   M M M M M M 0 0
//		0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0   L L L L L L L 0   0 1 0 0 0 0 0 0   0 L L L L L L L   K K K K K K K M   K K K K K K K M

		u64 q = x; // (1)
		q = q & 0x0001fff8001fc000ull; // (5)
		q = q ^ p; // (6)

		p = q >> 14; // (7)
		p = p * 0x0200000000008002ull >> 29; // (8)
		p = p & 0x00000007f0003fffull; // (9)

//      (10)              (11)
//		0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0
//		K 0 0 0 0 0 0 0   A 0 0 0 0 0 0 0
//		M M K K K K K K   B B A A A A A A
//		L L L M M M M M   C C C B B B B B
//		0 0 0 0 L L L L   D D D D C C C C
//		L L L L L 0 0 0   E E E E E D D D
//		M M M M M M L L   F F F F F F E E
//		K K K K K K K M   G G G G G G G F

		u64 z = p | q; // (10)
		return x ^ z; // (11)
	}
	static inline constexpr u64 mirror(u64 x) {
		u64 z = x & 0b0001000000100000010000001000000100000010000001000ull;
		for (u32 i = 0; i < 3; i++) {
			z |= (x & (0b0000001000000100000010000001000000100000010000001ull << i)) << (6 - i * 2);
			z |= (x & (0b1000000100000010000001000000100000010000001000000ull >> i)) >> (6 - i * 2);
		}
		return z;
	}

	struct Isomorphisms {
		u64 iso[8];
		inline constexpr Isomorphisms(u64 x) : iso{} {
			iso[0] = x;
			iso[1] = flip(iso[0]);
			iso[2] = transpose(iso[1]);
			iso[3] = flip(iso[2]);
			iso[4] = transpose(iso[3]);
			iso[5] = flip(iso[4]);
			iso[6] = transpose(iso[5]);
			iso[7] = flip(iso[6]);
		}
		static inline constexpr u64 transform(u64 x, u32 i) {
			switch (i % 8) {
			default:
			case 0: return x;
			case 6: x = transpose(x);
			case 1: x = flip(x);
			        return x;
			case 4: x = flip(x);
			case 5: x = mirror(x);
			        return x;
			case 3: x = mirror(x);
			case 2: x = flip(x);
			case 7: x = transpose(x);
			        return x;
			}
		}
		inline constexpr u64& operator[] (u32 i) { return iso[i]; }
		inline constexpr const u64& operator[] (u32 i) const { return iso[i]; }
		inline constexpr u64* begin() { return iso; }
		inline constexpr const u64* begin() const { return iso; }
		inline constexpr u64* end() { return iso + 8; }
		inline constexpr const u64* end() const { return iso + 8; }
	};

public:
	friend std::ostream& operator <<(std::ostream& out, const Zone7x7Bitboard& z) {
		const bool print_axis_label = false;
		const bool print_extra_space = true;
		const char* symbol[] = {"\u00B7", "\u25CF", "\u25CB", "\u00A0"}; // empty, black, white, irrelevant
		const char* border = print_extra_space ? "+-------------+" : "+-------+";
		const char* padding = print_axis_label ? " " : "";
		out << padding << border << std::endl;
		for (int y = 6; y >= 0; y--) {
			if (print_axis_label) out << char('1' + y);
			out << '|';
			for (int x = 0; x < 7; x++) {
				out << symbol[std::min<u32>(z.get(x, y), 3)];
				if (x != 6 && print_extra_space) out << ' ';
			}
			out << '|' << std::endl;
		}
		out << padding << border << std::endl;
		if (print_axis_label) out << "  A B C D E F G " << std::endl;
		return out;
	}
};
