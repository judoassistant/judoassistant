#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QString>
#include <QVBoxLayout>

class QutejudoWidget : public QMainWindow {
    Q_OBJECT

public:
    QutejudoWidget();
    ~QutejudoWidget();
private:
    QTabWidget * m_tabWidget;
};

