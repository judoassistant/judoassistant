#include "ui/misc/dark_palette.hpp"

// TODO: Subclass QPalette with special colors
DarkPalette::DarkPalette() {
    // Active and inactive
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

    // Disabled
    setColor(QPalette::Disabled, QPalette::AlternateBase, color(QPalette::AlternateBase).darker()); // Used for alternating rows and similar
    setColor(QPalette::Disabled, QPalette::Base, color(QPalette::Base).darker()); // Background of text areas
    setColor(QPalette::Disabled, QPalette::BrightText, color(QPalette::BrightText).darker());
    setColor(QPalette::Disabled, QPalette::Button, color(QPalette::Button).darker());
    setColor(QPalette::Disabled, QPalette::ButtonText, color(QPalette::ButtonText).darker());
    setColor(QPalette::Disabled, QPalette::Highlight, color(QPalette::Highlight));
    setColor(QPalette::Disabled, QPalette::HighlightedText, color(QPalette::HighlightedText).darker());
    setColor(QPalette::Disabled, QPalette::Text, color(QPalette::Text).darker());
    // setColor(QPalette::ToolTipBase, Qt::white);
    // setColor(QPalette::ToolTipText, Qt::white);
    setColor(QPalette::Disabled, QPalette::Window, color(QPalette::Window).darker()); // Background of border area
    setColor(QPalette::Disabled, QPalette::WindowText, color(QPalette::WindowText).darker());

    setColor(QPalette::Light, color(QPalette::Button).lighter(150)); // Lighter than Button color.
    setColor(QPalette::Midlight, color(QPalette::Button).lighter(100)); // Between Button and Light.
    setColor(QPalette::Dark, color(QPalette::Button).darker(150)); // Darker than Button.
    setColor(QPalette::Mid, color(QPalette::Button).darker(100)); // Between Button and Dark.
    setColor(QPalette::Shadow, QColor("#000000"));
}

