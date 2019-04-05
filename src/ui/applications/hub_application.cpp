#include <QStyleFactory>
#include <QCommandLineParser>

#include "ui/applications/hub_application.hpp"
#include "core/core.hpp"
#include "core/version.hpp"
#include "ui/widgets/hub_window.hpp"

HubApplication::HubApplication(int &argc, char *argv[]) : QApplication(argc, argv) {
    setApplicationName("JudoAssistant");
    setApplicationVersion(QString::fromStdString(ApplicationVersion::current().toString()));

    QCommandLineParser parser;
    parser.setApplicationDescription("JudoAssistant");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("tournament", "Tournament file to open");

    parser.process(*this);

    mArgs = parser.positionalArguments();

    setStyle(QStyleFactory::create("fusion"));

    QPalette palette;
    palette.setColor(QPalette::AlternateBase, QColor("#9bdb4d"));
    palette.setColor(QPalette::Base, QColor("#3d4248")); // Background of text areas
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Button, QColor("#3d4248"));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::Highlight, QColor("#3689e6"));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    // palette.setColor(QPalette::ToolTipBase, Qt::white);
    // palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Window, QColor("#36393e")); // Background of border area
    palette.setColor(QPalette::WindowText, Qt::white);
    setPalette(palette);
}

int HubApplication::exec() {
    HubWindow window;

    if (mArgs.size() > 0) {
        window.readTournament(mArgs.at(0));
    }

    window.show();
    window.startServer();

    return QApplication::exec();
}

int main(int argc, char *argv[]) {
    HubApplication app(argc, argv);
    return app.exec();
}

