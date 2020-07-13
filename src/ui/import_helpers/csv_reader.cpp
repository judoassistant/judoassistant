#include <fstream>
#include <algorithm>
#include "ui/import_helpers/csv_reader.hpp"

CSVReader::CSVReader(const QString &path) {
    try {
        std::ifstream file(path.toStdString(), std::ios::in);

        if (!file.is_open()) {
            mOpen = false;
            return;
        }

        for (std::string line; std::getline(file, line); )
            mLines.push_back(QString::fromStdString(line));

        mDelimiter = (!mLines.empty() ? guessDelimiter(mLines.front()) : QChar(','));
    }
    catch (const std::exception &e) {
        mOpen = false;
        return;
    }

    mOpen = true;
    parse();
}

QChar CSVReader::guessDelimiter(const QString &line) const {
    QChar del(',');
    size_t delCount = 0;

    for (QChar a : listDelimiters()) {
        size_t count = std::count(line.begin(), line.end(), a);
        if (count >= delCount) {
            del = a;
            delCount = count;
        }
    }

    return del;
}

QChar CSVReader::getDelimiter() const {
    return mDelimiter;
}

void CSVReader::setDelimiter(QChar del) {
    mDelimiter = del;

    parse();
}

size_t CSVReader::rowCount() const {
    return mFields.size();
}

size_t CSVReader::columnCount() const {
    return mColumnCount;
}

void CSVReader::parse() {
    mFields.clear();
    mColumnCount = 0;

    for (const QString &line : mLines) {
        mFields.emplace_back();
        QString curField;

        bool inQuotes = false;
        for (QChar c : line) {
            if (c == '"' && mDelimiter != '"') {
                inQuotes = !inQuotes;
                continue;
            }

            if (c == mDelimiter && !inQuotes) {
                mFields.back().push_back(curField.trimmed());
                curField.clear();
                continue;
            }

            curField.push_back(c);
        }

        mFields.back().push_back(curField.trimmed());
        mColumnCount = std::max(mColumnCount, mFields.back().size());
    }
}

QString CSVReader::get(size_t row, size_t column) const {
    assert(row < rowCount());
    assert(column < columnCount());

    const auto & vec = mFields[row];

    if (column < vec.size())
        return vec[column];
    return QString("");
}

std::vector<QChar> CSVReader::listDelimiters() {
    return {',', ';', '\t'};
}

bool CSVReader::isOpen() const {
    return mOpen;
}

