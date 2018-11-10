#include <QStyleFactory>
#include <QCommandLineParser>

#include "applications/qutejudo_application.hpp"
#include "version.hpp"
#include "widgets/qutejudo_window.hpp"

QutejudoApplication::QutejudoApplication(int &argc, char *argv[]) : QApplication(argc, argv) {
    setApplicationName("Qutejudo");
    setApplicationVersion(QString::fromStdString(ApplicationVersion::current().toString()));

    QCommandLineParser parser;
    parser.setApplicationDescription("Qutejudo");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("tournament", "Tournament file to open");

    parser.process(*this);

    mArgs = parser.positionalArguments();

    // setStyle(QStyleFactory::create("fusion"));

    // mPalette.setColor(QPalette::Window, QColor(53,53,53));
    // mPalette.setColor(QPalette::WindowText, Qt::white);
    // mPalette.setColor(QPalette::Base, QColor(15,15,15));
    // mPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    // mPalette.setColor(QPalette::ToolTipBase, Qt::white);
    // mPalette.setColor(QPalette::ToolTipText, Qt::white);
    // mPalette.setColor(QPalette::Text, Qt::white);
    // mPalette.setColor(QPalette::Button, QColor(53,53,53));
    // mPalette.setColor(QPalette::ButtonText, Qt::white);
    // mPalette.setColor(QPalette::BrightText, Qt::red);
    // mPalette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    // mPalette.setColor(QPalette::HighlightedText, Qt::black);

    // setPalette(mPalette);
}

int QutejudoApplication::exec() {
    QutejudoWindow window;

    if (mArgs.size() > 0) {
        window.readTournament(mArgs.at(0));
    }

    window.show();

    return QApplication::exec();
}
