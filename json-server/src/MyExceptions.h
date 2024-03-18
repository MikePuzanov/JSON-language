#ifndef MYEXCEPTIONS_H
#define MYEXCEPTIONS_H

#include <stdexcept>

class IndexException : public std::runtime_error {
public:
    explicit IndexException(const std::string& message)
        : std::runtime_error(message) {}
};

class NotFoundDataException : public std::runtime_error {
public:
    explicit NotFoundDataException(const std::string& message)
        : std::runtime_error(message) {}
};

class IsNotArrayException : public std::runtime_error {
public:
    explicit IsNotArrayException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidJSONFormatException : public std::runtime_error {
public:
    InvalidJSONFormatException(const std::string& message) : std::runtime_error(message) {}
};

#endif // MYEXCEPTIONS_H
