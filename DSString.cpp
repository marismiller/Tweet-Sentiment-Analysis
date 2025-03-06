#include "DSString.h"
#include <cctype>  // allowed for manual char transformations

// Default constructor -> empty string
DSString::DSString() : data(nullptr), len(0) {
    data = new char[1];
    data[0] = '\0';
}

// Construct from C-string
DSString::DSString(const char* str) : data(nullptr), len(0) {
    if (str) {
        // Manual length
        size_t count = 0;
        while (str[count] != '\0') {
            count++;
        }
        len = count;
        data = new char[len + 1];
        for (size_t i = 0; i < len; i++) {
            data[i] = str[i];
        }
        data[len] = '\0';
    }
    else {
        // treat null as empty
        data = new char[1];
        data[0] = '\0';
        len = 0;
    }
}

// Copy constructor
DSString::DSString(const DSString& other) : data(nullptr), len(other.len) {
    data = new char[len + 1];
    for (size_t i = 0; i < len; i++) {
        data[i] = other.data[i];
    }
    data[len] = '\0';
}

// Assignment operator
DSString& DSString::operator=(const DSString& other) {
    if (this == &other) {
        return *this; // self-assign
    }
    delete[] data; // free old

    len = other.len;
    data = new char[len + 1];
    for (size_t i = 0; i < len; i++) {
        data[i] = other.data[i];
    }
    data[len] = '\0';

    return *this;
}

// Destructor
DSString::~DSString() {
    delete[] data;
    data = nullptr;
    len = 0;
}

// Return length
size_t DSString::length() const {
    return len;
}

// Return C-string
const char* DSString::c_str() const {
    return data;
}

// Non-const indexing
char& DSString::operator[](size_t index) {
    return data[index]; // no bounds check
}

// Const indexing
char DSString::operator[](size_t index) const {
    return data[index]; 
}

// Concatenation
DSString DSString::operator+(const DSString& rhs) const {
    DSString result;
    // remove the default constructor’s data
    delete[] result.data;

    size_t newLen = len + rhs.len;
    result.data = new char[newLen + 1];
    result.len = newLen;

    // copy
    for (size_t i = 0; i < len; i++) {
        result.data[i] = data[i];
    }
    // then append rhs
    for (size_t i = 0; i < rhs.len; i++) {
        result.data[len + i] = rhs.data[i];
    }
    result.data[newLen] = '\0';
    return result;
}

// Equality
bool DSString::operator==(const DSString& rhs) const {
    if (len != rhs.len) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (data[i] != rhs.data[i]) {
            return false;
        }
    }
    return true;
}

// Less-than (lexicographical)
bool DSString::operator<(const DSString& rhs) const {
    // compare char by char
    size_t i = 0;
    while (i < len && i < rhs.len) {
        if (data[i] < rhs.data[i]) return true;
        if (data[i] > rhs.data[i]) return false;
        i++;
    }
    // if all same so far, shorter is less
    return (len < rhs.len);
}

// substring
DSString DSString::substring(size_t start, size_t numChars) const {
    if (start >= len) {
        return DSString(); // empty
    }
    if (start + numChars > len) {
        numChars = len - start;
    }
    DSString sub;
    delete[] sub.data;

    sub.len = numChars;
    sub.data = new char[numChars + 1];
    for (size_t i = 0; i < numChars; i++) {
        sub.data[i] = data[start + i];
    }
    sub.data[numChars] = '\0';
    return sub;
}

// toLower
DSString DSString::toLower() const {
    DSString lower;
    delete[] lower.data;

    lower.len = len;
    lower.data = new char[len + 1];
    for (size_t i = 0; i < len; i++) {
        char c = data[i];
        // manual tolower
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c + ('a' - 'A'));
        }
        lower.data[i] = c;
    }
    lower.data[len] = '\0';
    return lower;
}

// Operator<<
std::ostream& operator<<(std::ostream& out, const DSString& str) {
    out << str.c_str();
    return out;
}
