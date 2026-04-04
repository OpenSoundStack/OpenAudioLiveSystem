// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include <QApplication>

#include "DebuggerWindow.h"

int main(int argc, char* argv[]) {
    QApplication app{argc, argv};

    auto* dbg_win = new DebuggerWindow{};
    dbg_win->show();

    return app.exec();
}