// // includes for library features (any comment with // // is by Sione, just // is by claudeAI)
#include <iostream>
#include <string>

// ---------- Helpers ----------

static bool isRelevant(char c) { // // Bool function to check for valid characters
    return (c >= '0' && c <= '9') || c == '.' || c == ':';
}

static bool isDigitChar(char c) { // // Bool function to check for digit characters
    return c >= '0' && c <= '9';
}

// Reads a maximal run of digit characters starting at pos (within [start,end)).
// Sets digitsEnd to one-past-the-last digit. Returns false if there are no digits at pos.
static bool readDigits(const std::string& s, size_t pos, size_t end, size_t& digitsEnd) {
    size_t p = pos;
    while (p < end && isDigitChar(s[p])) {
        ++p;
    }
    if (p == pos) return false; // empty digit run
    digitsEnd = p;
    return true;
}

// Validates the digit substring [start,end) as a numeric field:
//  - length must be 1..maxLen
//  - no leading zero unless the field is the single character '0'
//  - numeric value must be <= maxValue
// On success, sets value.
static bool validateField(const std::string& s, size_t start, size_t end,
                           size_t maxLen, long maxValue, long& value) {
    size_t len = end - start;
    if (len == 0 || len > maxLen) return false;
    if (len > 1 && s[start] == '0') return false; // disallowed leading zero

    long val = 0;
    for (size_t i = start; i < end; ++i) { // // for loop to itreate through the string and convert the value
        val = val * 10 + (s[i] - '0');
    }
    if (val > maxValue) return false;

    value = val;
    return true;
}

// Attempts to parse the ENTIRE run [start,end) as: octet.octet.octet.octet(:port)?
// Every character in the run must be consumed by the grammar, or it fails.
static bool parseAddressToken(const std::string& s, size_t start, size_t end,
                               unsigned long& addr, int& port) {
    size_t pos = start;
    long octets[4];

    for (int i = 0; i < 4; ++i) { // // for loop to itreate throug hte string and pare the octets
        size_t digitsEnd;
        if (!readDigits(s, pos, end, digitsEnd)) return false;

        long val;
        if (!validateField(s, pos, digitsEnd, 3, 255, val)) return false;

        octets[i] = val;
        pos = digitsEnd;

        if (i < 3) {
            if (pos >= end || s[pos] != '.') return false; // must be separated by '.'
            ++pos; // consume '.'
        }
    }

    // After the 4th octet: either end of run (no port), or ':' + valid port, and
    // nothing may be left over afterward.
    if (pos == end) {
        port = -1;
    } else if (s[pos] == ':') { // // if statement to catch if there is a port number
        ++pos;
        size_t digitsEnd;
        if (!readDigits(s, pos, end, digitsEnd)) return false;

        long val;
        if (!validateField(s, pos, digitsEnd, 5, 65535, val)) return false;

        port = static_cast<int>(val);
        pos = digitsEnd;
        if (pos != end) return false; // trailing junk after the port
    } else {
        return false; // stray '.' or other leftover right after the 4th octet
    }

    addr = (static_cast<unsigned long>(octets[0]) << 24) | // // bitwise operations to convert the octets into a 32-bit value
           (static_cast<unsigned long>(octets[1]) << 16) |
           (static_cast<unsigned long>(octets[2]) << 8)  |
           (static_cast<unsigned long>(octets[3]));
    return true;
}

// ---------- Public API ----------

// Returns true if a valid address was found, false otherwise.
// On success: outAddress holds the 32-bit value,
// and outPort holds the port number, or -1 if no port was present.
// On failure: outAddress is set to 0 and outPort is set to -1.
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    size_t n = str.size();
    size_t i = 0;

    while (i < n) { // // while loop to scan through the string for relevant characters
        if (!isRelevant(str[i])) {
            ++i;
            continue;
        }

        size_t runStart = i;
        while (i < n && isRelevant(str[i])) {
            ++i;
        }
        size_t runEnd = i; // run is [runStart, runEnd)

        unsigned long addr;
        int port;
        if (parseAddressToken(str, runStart, runEnd, addr, port)) { // // parse if statement to check if the address token is valid
            outAddress = addr;
            outPort = port;
            return true;
        }
        // otherwise keep scanning for the next run
    }

    outAddress = 0; // // setting gthe out address and outport to specified values for C++ as stated in assignment doc
    outPort = -1;
    return false;
}

// ---------- Driver ----------

int main() { // // main function to run program
    std::string line;

    while (true) { // // infinite loop for user input until user END
        std::cout << "Enter a string (or 'END' to quit): "; // // prompt user for input
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line == "END") { // // user end statement check
            std::cout << "Program terminated." << std::endl;
            break;
        }

        // // values to hold the extracted address and port
        unsigned long addr;
        int port;

        // // extracting the IPv4 address and port form user input
        if (extractIPv4(line, addr, port)) {
            unsigned int a = (addr >> 24) & 0xFF;
            unsigned int b = (addr >> 16) & 0xFF;
            unsigned int c = (addr >> 8) & 0xFF;
            unsigned int d = addr & 0xFF;

            // // ouput for valid IPv4 address and port
            std::cout << "Extracted IPv4 address: "
                       << a << "." << b << "." << c << "." << d
                       << " (decimal value: " << addr << ", port: ";
            if (port == -1) {
                std::cout << "none";
            } else {
                std::cout << port;
            }
            std::cout << ")" << std::endl;
        } else { // // else statment if invalid input is given.
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl;
        }
    }

    return 0; // // standard final return
}