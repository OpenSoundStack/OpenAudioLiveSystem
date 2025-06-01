// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2025 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

#include <qapplication.h>
#include <qscreen.h>
#include <qmessagebox.h>

#include "ui/SignalWindow.h"
#include "ui/SetupWindow.h"

#include "core/ShowManager.h"

int main(int argc, char* argv[]) {
    QApplication qapp {argc, argv};

    auto* sm = new ShowManager{};

    // Software initialization
    // Load stored console config

    std::cout << "Loading console config..." << std::endl;
    // Must be loaded before console init as it contains system critical settings
    sm->load_console_config();
    std::cout << "Config loaded !" << std::endl;

    // UI Initialization
    // As ShowManager needs SignalWidow I have to put its initialization here
    SignalWindow signal_win{};

    if (!sm->init_console(&signal_win)) {
        QMessageBox::critical(nullptr, "ERROR", "Failed to initialize ShowManager. Unable to launch console software.");
        std::cerr << "Failed to initialize ShowManager. Unable to launch console software." << std::endl;

        qapp.exit(-2);
        return -2;
    }

    // Must be loaded after console init
    std::cout << "Loading pipe config..." << std::endl;
    sm->load_pipe_config();
    std::cout << "Config loaded !" << std::endl;

    // SetupWindow needs ShowManager to be initialized
    SetupWindow setup_win{sm, &signal_win};

    // Screen meta
    auto screens = QGuiApplication::screens();

    // For debug purposes, emulate console behviour if enough screens available
    if (screens.size() >= 2) {
        setup_win.move(screens[1]->geometry().x(), screens[1]->geometry().y());
        setup_win.showFullScreen();

        signal_win.move(screens[0]->geometry().x(), screens[0]->geometry().y());
        signal_win.showFullScreen();
    } else {
        setup_win.show();
        signal_win.show();
    }

    return qapp.exec();
}
