#pragma once

#include <QString>
#include <vector>

#include "core/core.hpp"

class CSVReader {
public:
    CSVReader(const QString &path);
    static std::vector<QChar> listDelimiters();
    QChar getDelimiter() const;
    void setDelimiter(QChar del);
    size_t rowCount() const;
    size_t columnCount() const;
    QString get(size_t row, size_t column) const;
    bool isOpen() const;

protected:
    void parse();
    QChar guessDelimiter(const QString &line) const;

private:
    size_t mColumnCount;
    std::vector<QString> mLines;
    std::vector<std::vector<QString>> mFields;
    QChar mDelimiter;
    bool mOpen;
};

