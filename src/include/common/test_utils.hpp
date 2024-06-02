#include <random>
#include <string>
namespace db {
[[nodiscard]] inline std::string GenerateRandomString(int a, int b) {
	// Seed with a real random value, if available
	std::random_device rd;

	// Choose a random length between a and b (inclusive)
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine
	std::uniform_int_distribution<> dist(a, b);
	int stringLength = dist(gen);

	// Define a character set to use in the string
	const char charset[] = "0123456789"
	                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	                       "abcdefghijklmnopqrstuvwxyz";
	int charsetSize = sizeof(charset) - 1; // Subtract 1 to avoid the null terminator

	// Create a random string
	std::string randomString;
	randomString.reserve(stringLength);

	std::uniform_int_distribution<> charDist(0, charsetSize - 1);
	for (int i = 0; i < stringLength; ++i) {
		randomString += charset[charDist(gen)];
	}

	return randomString;
}
} // namespace db
