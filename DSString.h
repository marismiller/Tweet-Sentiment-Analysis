#ifndef DSSTRING_H
#define DSSTRING_H

#include <iostream>
#include <cstddef> // for size_t

class DSString {
private:
    char* data;
    size_t len;

public:
    // Constructors & destructor
    DSString();
    DSString(const char*);            // from C-style string
    DSString(const DSString&);        // copy constructor
    DSString& operator=(const DSString&); // assignment
    ~DSString();

    // Accessors
    size_t length() const;
    const char* c_str() const; 

    // Operators
    char& operator[](size_t index);         // non-const indexing
    char operator[](size_t index) const;    // const indexing

    DSString operator+(const DSString&) const;
    bool operator==(const DSString&) const;
    bool operator<(const DSString&) const;

    // Substring + toLower
    DSString substring(size_t start, size_t numChars) const;
    DSString toLower() const;

    // Stream insertion
    friend std::ostream& operator<<(std::ostream&, const DSString&);
};

#endif
