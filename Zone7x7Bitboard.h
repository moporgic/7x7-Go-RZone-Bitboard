#pragma once
#include <iostream>

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
	inline constexpr Zone7x7Bitboard operator ~() const { return {~zone, ~black, ~white}; }

	inline constexpr bool operator ==(const Zone7x7Bitboard& z) const { return (zone == z.zone) & (black == z.black) & (white == z.white); }
	inline constexpr bool operator < (const Zone7x7Bitboard& z) const { return (zone <  z.zone) | (black <  z.black) | (white <  z.white); }
	inline constexpr bool operator !=(const Zone7x7Bitboard& z) const { return !(*this == z); }
	inline constexpr bool operator > (const Zone7x7Bitboard& z) const { return  (z < *this); }
	inline constexpr bool operator <=(const Zone7x7Bitboard& z) const { return !(z < *this); }
	inline constexpr bool operator >=(const Zone7x7Bitboard& z) const { return !(*this < z); }

public:
	enum PieceType {
		ZONE_EMPTY = 0u,
		ZONE_BLACK = 1u,
		ZONE_WHITE = 2u,
		ZONE_UNKNOWN = 3u,
		IRRELEVANT = -1u,
	};
	inline constexpr PieceType get(u32 x, u32 y) const {
		u64 mask = 1ull << (y * 7 + x);
		if ((zone & mask) == 0) return PieceType::IRRELEVANT;
		return static_cast<PieceType>(
				(black & mask ? PieceType::ZONE_BLACK : PieceType::ZONE_EMPTY) |
				(white & mask ? PieceType::ZONE_WHITE : PieceType::ZONE_EMPTY));
	}
	inline constexpr void set(u32 x, u32 y, u32 type) {
		u64 mask = 1ull << (y * 7 + x);
		switch (type) {
		case PieceType::ZONE_EMPTY:
			zone |= mask;
			black &= ~mask;
			white &= ~mask;
			break;
		case PieceType::ZONE_BLACK:
			zone |= mask;
			black |= mask;
			white &= ~mask;
			break;
		case PieceType::ZONE_WHITE:
			zone |= mask;
			black &= ~mask;
			white |= mask;
			break;
		default:
		case PieceType::IRRELEVANT:
			zone &= ~mask;
			black &= ~mask;
			white &= ~mask;
			break;
		}
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
	inline constexpr void isomorphize(u32 i) {
		zone = isomorphize(zone, i);
		black = isomorphize(black, i);
		white = isomorphize(white, i);
	}

public:
	inline constexpr void minimize() {
		u64 ziso[8] = {};
		store_isomorphisms(zone, ziso);
		u32 min = 0;
		u32 offset = calculate_offset(zone);
		zone >>= offset;
		for (u32 i = 1; i < 8; i++) {
			u32 ofiso = calculate_offset(ziso[i]);
			ziso[i] >>= ofiso;
			if (ziso[i] < zone) {
				min = i;
				offset = ofiso;
				zone = ziso[i];
			}
		}
		black = isomorphize(black, min) >> offset;
		white = isomorphize(white, min) >> offset;
	}

protected:
	static inline constexpr u64 transpose(u64 x) {

//		(1)             (2)             (3)             (4)             (5)
//		q r s t u v w   q<l>s t u<p>w  <e>l s t<i>p w   e l s<b>i p w  <G N U>b i p w
//		j k l m n o p  <d>k<r>m<h>o<v>  d k r<a>h o v   d k r a h o v  <F M T>a h o v
//		c d e f g h i   c<j>e<Z>g<n>i   c j<q>Z g n<u>  c j q Z g n u  <E L S>Z g n u
//		V W X Y Z a b   V W<R>Y<f>a b   V<K>R Y f<m>b  <D>K R Y f m<t>  D K R Y f m t
//		O P Q R S T U   O<J>Q<X>S<N>U  <C>J Q X<G>N U   C J Q X G N U   C J Q X<e l s>
//		H I J K L M N  <B>I<P>K<F>M<T>  B I P<W>F M T   B I P W F M T   B I P W<d k r>
//		A B C D E F G   A<H>C D E<L>G   A H<O>D E L<S>  A H O<V>E L S   A H O V<c j q>

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

//      (1)      (2)                        (3)      (4)
//		00000000 00000000 01000000 BAAAAAAA 00000000 00000000
//		G0000000 00000000 00000000 AABBBBBB A0000000 A0000000
//		FFGGGGGG 00000000 00010000 000AAAAA BBAAAAAA BBAAAAAA
//		EEEFFFFF EEE00000 00000000 EEEE0000 AAABBBBB 000BBBBB
//		DDDDEEEE 0000EEEE 00000000 00000EEE 0000AAAA 00000000
//		CCCCCDDD 00000000 00000000 00000000 EEEEE000 EEEEE000
//		BBBBBBCC BBBBBB00 00000000 BBBBBBB0 000000EE 000000EE
//		AAAAAAAB AAAAAAAB 01000000 0AAAAAAA 00000000 00000000

		u64 p = x; // (1)
		p = p & 0x00000007f0003fffull; // (2)
		p = p * 0x0200080000000002ull >> 15; // (3)
		p = p & 0x0001fff8001fc000ull; // (4)

//      (5)      (6)      (7)                        (8)      (9)
//		00000000 00000000 00000000 01000000 0LLLLLLL 00000000 00000000
//		G0000000 K0000000 00000000 00000000 KK000000 00000000 00000000
//		FFGGGGGG MMKKKKKK 00000000 00000000 MMMKKKKK 00000000 00000000
//		000FFFFF 000MMMMM KKK00000 00000000 KKKKMMMM LLL00000 LLL00000
//		00000000 00000000 MMMMKKKK 00000000 MMMMMKKK 0000LLLL 0000LLLL
//		CCCCC000 LLLLL000 00000MMM 00000000 LLLLLLMM KKKKK000 00000000
//		000000CC 000000LL 00000000 00000001 0000000L MMMMMMKK MMMMMM00
//		00000000 00000000 LLLLLLL0 01000000 0LLLLLLL KKKKKKKM KKKKKKKM

		u64 q = x; // (1)
		q = q & 0x0001fff8001fc000ull; // (5)
		q = q ^ p; // (6)

		p = q >> 14; // (7)
		p = p * 0x0200000000008002ull >> 29; // (8)
		p = p & 0x00000007f0003fffull; // (9)

//      (10)     (11)
//		00000000 00000000
//		K0000000 A0000000
//		MMKKKKKK BBAAAAAA
//		LLLMMMMM CCCBBBBB
//		0000LLLL DDDDCCCC
//		LLLLL000 EEEEEDDD
//		MMMMMMLL FFFFFFEE
//		KKKKKKKM GGGGGGGF

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
	static inline constexpr u64 isomorphize(u64 x, u32 i) {
		switch (i % 8) {
		default:
		case 0:
			return x;
		case 6: x = transpose(x);
			// no break
		case 1: x = flip(x);
			return x;
		case 4: x = flip(x);
			// no break
		case 5: x = mirror(x);
			return x;
		case 3: x = mirror(x);
			// no break
		case 2: x = flip(x);
			// no break
		case 7: x = transpose(x);
			return x;
		// for XMTMTMTM series
//		case 3: z = transpose(z);
//		case 4: z = flip(z);
//		case 1: z = mirror(z);
//			return z;
//		case 2: z = transpose(z);
//		case 5: z = flip(z);
//			return z;
//		case 6: z = flip(z);
//		case 7: z = transpose(z);
//			return z;
		}
	}
	static inline constexpr void store_isomorphisms(u64 x, u64 iso[]) {
		iso[0] = x;
		iso[1] = flip(iso[0]);
		iso[2] = transpose(iso[1]);
		iso[3] = flip(iso[2]);
		iso[4] = transpose(iso[3]);
		iso[5] = flip(iso[4]);
		iso[6] = transpose(iso[5]);
		iso[7] = flip(iso[6]);
	}
	static inline constexpr u32 calculate_offset(u64 x) {
		u32 offset = 0;
		if (x >> 42 == 0) offset += (std::max<u32>(tzcnt49(x) / 7, 1) - 1) * 7;
		u64 z = transpose(x);
		if (z >> 42 == 0) offset += (std::max<u32>(tzcnt49(z) / 7, 1) - 1);
		return offset;
	}
	static inline constexpr u32 tzcnt49(u64 x) {
		return __builtin_ctzll(x | (1ull << 49));
	}

public:
	friend std::ostream& operator <<(std::ostream& out, const Zone7x7Bitboard& z) {
		bool show_label = false;
		bool extra_space = true;
		const char* print[] = {"\u00B7", "\u25CF", "\u25CB", "\u00A0"};
		const char* border = extra_space ? "+-------------+" : "+-------+";
		const char* border_left = show_label ? " " : "";
		out << border_left << border << std::endl;
		for (int y = 6; y >= 0; y--) {
			if (show_label) out << char('1' + y);
			out << '|';
			for (int x = 0; x < 7; x++) {
				out << print[z.get(x, y) & 3];
				if (x != 6 && extra_space) out << ' ';
			}
			out << '|' << std::endl;
		}
		out << border_left << border << std::endl;
		if (show_label) out << "  A B C D E F G " << std::endl;
		return out;
	}
};
