#include <fstream>
#include <algorithm>
#include "widgets/import_helpers/csv_reader.hpp"

CSVReader::CSVReader(const QString &path) {
    try {
        std::ifstream file(path.toStdString(), std::ios::in);

        if (!file.is_open()) {
            mOpen = false;
            return;
        }

        for (std::string line; std::getline(file, line); )
            mLines.push_back(line);

        mDelimiter = (!mLines.empty() ? guessDelimiter(mLines.front()) : ',');
    }
    catch (const std::exception &e) {
        mOpen = false;
        return;
    }

    mOpen = true;
    parse();
}

char CSVReader::guessDelimiter(const std::string &line) const {
    char del;
    size_t delCount = 0;

    for (char a : listDelimiters()) {
        size_t count = std::count(line.begin(), line.end(), a);
        if (count > delCount) {
            del = a;
            delCount = count;
        }
    }

    return del;
}

char CSVReader::getDelimiter() const {
    return mDelimiter;
}

void CSVReader::setDelimiter(char del) {
    mDelimiter = del;

    parse();
}

size_t CSVReader::rowCount() const {
    return mFields.size();
}

size_t CSVReader::columnCount() const {
    return mColumnCount;
}

std::string trim(const std::string &str) {
    size_t begin;
    for (begin = 0; begin < str.size(); ++begin) {
        if (str[begin] != ' ')
            break;
    }

    size_t end;
    for (end = str.size(); end > 0; --end) {
        if (str[end-1] != ' ')
            break;
    }

    return str.substr(begin, end-begin);
}

void CSVReader::parse() {
    mFields.clear();
    mColumnCount = 0;

    for (std::string line : mLines) {
        mFields.emplace_back();
        std::string curField;

        bool inQuotes = false;
        for (char c : line) {
            if (c == '"' && mDelimiter != '"') {
                inQuotes = !inQuotes;
                continue;
            }

            if (c == mDelimiter && !inQuotes) {
                mFields.back().push_back(trim(curField));
                curField.clear();
                continue;
            }

            curField.push_back(c);
        }

        mFields.back().push_back(trim(curField));
        mColumnCount = std::max(mColumnCount, mFields.back().size());
    }
}

std::string CSVReader::get(size_t row, size_t column) const {
    assert(row < rowCount());
    assert(column < columnCount());

    const auto & vec = mFields[row];

    if (column < vec.size())
        return vec[column];
    return "";
}

std::vector<char> CSVReader::listDelimiters() {
    return {',', ';', '\t'};
}

bool CSVReader::isOpen() const {
    return mOpen;
}

