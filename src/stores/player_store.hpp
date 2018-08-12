#pragma once

#include <QObject>
#include <QString>

class PlayerStore : public QObject {
    Q_OBJECT
public:
    PlayerStore();
public slots:
    // void setValue(int value);
signals:
    // void valueChanged(int value);
private:
    QString m_firstName;
    QString m_lastName;
    int m_age;
    int id;
};

