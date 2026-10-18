// Just some useful functions... Just to avoid a mess like in upasnes

#pragma once
#include <type_traits>
#include <cstdint>
#include <limits>
#include <string>

// CONSTANTS
constexpr bool TESTING_CPU = false;
constexpr bool END_UPON_EXCEPTION = false;
constexpr bool SIDELOAD_EXE = true;

template<typename T>
requires std::is_unsigned_v<T>
constexpr T extract_bits(T n, unsigned lo, unsigned hi) {
	if (lo > hi) { // Quick swap if accidentally provided in incorrect order
		unsigned temp = lo;
		lo = hi;
		hi = temp;
	}

	const unsigned width = hi - lo + 1;

	if (width == sizeof(T) * 8) {
		return n;
	}

	return (n >> lo) & ((T{1} << width) - 1);
}

template<typename T>
requires std::is_unsigned_v<T>
constexpr bool get_bit(T n, unsigned bit) {
	return (n >> bit) & 1;
}

// Useful pre-computed values for powers of two
static const uint32_t powers_of_two[32] = {
    1, 2, 4, 8, 16, 32, 64, 128,
    256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
    65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608,
    16777216, 33554432, 67108864, 134217728, 268435456, 536870912,
    1073741824, 2147483648
};

bool add_overflows(uint32_t a, uint32_t b);
bool sub_overflows(uint32_t a, uint32_t b);

// For the test handler
enum BusAction : int {
	NOACTION = 0,
	READ = 1,
	WRITE = 2,
	FETCH = 4
};

struct State {
	uint32_t r[32];
	uint32_t hi, lo;
	uint32_t epc, tar, cause;
	uint32_t pc;

	bool delay_branch_slot;
	bool delay_branch_take;
	uint32_t delay_branch_target;

	int64_t delay_load_target;
	uint32_t delay_load_val;
};

struct Cycle {
	int actions;
	int sz;
	uint32_t addr;
	uint32_t val;
};

// Terminal colours sourced from https://gist.github.com/Kielx/2917687bc30f567d45e15a4577772b02
constexpr std::string COLOUR_RESET       = "\033[0m";
constexpr std::string COLOUR_BLACK       = "\033[30m";
constexpr std::string COLOUR_RED         = "\033[31m";
constexpr std::string COLOUR_GREEN       = "\033[32m";
constexpr std::string COLOUR_YELLOW      = "\033[33m";
constexpr std::string COLOUR_BLUE        = "\033[34m";
constexpr std::string COLOUR_MAGENTA     = "\033[35m";
constexpr std::string COLOUR_CYAN        = "\033[36m";
constexpr std::string COLOUR_WHITE       = "\033[37m";
constexpr std::string COLOUR_BOLDBLACK   = "\033[1m\033[30m";
constexpr std::string COLOUR_BOLDRED     = "\033[1m\033[31m";
constexpr std::string COLOUR_BOLDGREEN   = "\033[1m\033[32m";
constexpr std::string COLOUR_BOLDYELLOW  = "\033[1m\033[33m";
constexpr std::string COLOUR_BOLDBLUE    = "\033[1m\033[34m";
constexpr std::string COLOUR_BOLDMAGENTA = "\033[1m\033[35m";
constexpr std::string COLOUR_BOLDCYAN    = "\033[1m\033[36m";
constexpr std::string COLOUR_BOLDWHITE   = "\033[1m\033[37m";