#include "applications/qutejudo_application.hpp"

#include <QStyleFactory>
#include <QCommandLineParser>
#include "widgets/qutejudo_window.hpp"

QutejudoApplication::QutejudoApplication(int &argc, char *argv[]) : QApplication(argc, argv) {
    setApplicationName("Qutejudo");
    setApplicationVersion(QString::fromStdString(getApplicationVersion()));

    QCommandLineParser parser;
    parser.setApplicationDescription("Qutejudo");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("tournament", "Tournament file to open");

    parser.process(*this);

    mArgs = parser.positionalArguments();

    // setStyle(QStyleFactory::create("fusion"));

    // m_palette.setColor(QPalette::Window, QColor(53,53,53));
    // m_palette.setColor(QPalette::WindowText, Qt::white);
    // m_palette.setColor(QPalette::Base, QColor(15,15,15));
    // m_palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    // m_palette.setColor(QPalette::ToolTipBase, Qt::white);
    // m_palette.setColor(QPalette::ToolTipText, Qt::white);
    // m_palette.setColor(QPalette::Text, Qt::white);
    // m_palette.setColor(QPalette::Button, QColor(53,53,53));
    // m_palette.setColor(QPalette::ButtonText, Qt::white);
    // m_palette.setColor(QPalette::BrightText, Qt::red);
    // m_palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    // m_palette.setColor(QPalette::HighlightedText, Qt::black);

    // setPalette(m_palette);
}

int QutejudoApplication::exec() {
    QutejudoWindow window;

    if (mArgs.size() > 0) {
        window.readTournament(mArgs.at(0));
    }

    window.show();

    return QApplication::exec();
}
