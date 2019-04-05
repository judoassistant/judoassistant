#include "ui/misc/dark_palette.hpp"

DarkPalette::DarkPalette() {
    setColor(QPalette::AlternateBase, QColor("#3d4248").darker()); // Used for alternating rows and similar
    setColor(QPalette::Base, QColor("#3d4248")); // Background of text areas
    setColor(QPalette::BrightText, Qt::red);
    setColor(QPalette::Button, QColor("#3d4248"));
    setColor(QPalette::ButtonText, Qt::white);
    setColor(QPalette::Highlight, QColor("#3689e6"));
    setColor(QPalette::HighlightedText, Qt::white);
    setColor(QPalette::Text, Qt::white);
    // setColor(QPalette::ToolTipBase, Qt::white);
    // setColor(QPalette::ToolTipText, Qt::white);
    setColor(QPalette::Window, QColor("#36393e")); // Background of border area
    setColor(QPalette::WindowText, Qt::white);
}

