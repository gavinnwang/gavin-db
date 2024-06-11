#include <array>
#include <random>
#include <string>
namespace db {
[[nodiscard]] inline std::string GenerateRandomString(int min_size, int max_size) {
	// Seed with a real random value, if available
	std::random_device rand_device;

	// Choose a random length between a and b (inclusive)
	std::mt19937 gen(rand_device()); // Standard mersenne_twister_engine
	std::uniform_int_distribution<> dist(min_size, max_size);
	int string_length = dist(gen);

	// Define a character set to use in the string
	std::array<char, 62> charset = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	                                'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
	                                'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
	                                'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

	int charset_size = sizeof(charset) - 1; // Subtract 1 to avoid the null terminator

	// Create a random string
	std::string random_string;
	random_string.reserve(string_length);

	std::uniform_int_distribution<> char_dist(0, charset_size - 1);
	for (int i = 0; i < string_length; ++i) {
		random_string += charset[char_dist(gen)];
	}

	return random_string;
}
} // namespace db
