#pragma once

#include <QString>
#include <vector>

#include "core/core.hpp"

class CSVReader {
public:
    CSVReader(const QString &path);
    static std::vector<char> listDelimiters();
    char getDelimiter() const;
    void setDelimiter(char del);
    size_t rowCount() const;
    size_t columnCount() const;
    std::string get(size_t row, size_t column) const;
    bool isOpen() const;

protected:
    void parse();
    char guessDelimiter(const std::string &line) const;

private:
    size_t mColumnCount;
    std::vector<std::string> mLines;
    std::vector<std::vector<std::string>> mFields;
    char mDelimiter;
    bool mOpen;
};

