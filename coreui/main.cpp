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

    sm->load_pipe_config();
    sm->load_console_config();

    std::cout << "Config loaded !" << std::endl;

    // UI Initialization
    // As ShowManager needs SignalWidow I have to put their initialization here
    SetupWindow setup_win{sm};
    SignalWindow signal_win{};

    if (!sm->init_console(&signal_win)) {
        QMessageBox::critical(nullptr, "ERROR", "Failed to initialize ShowManager. Unable to launch console software.");
        std::cerr << "Failed to initialize ShowManager. Unable to launch console software." << std::endl;

        qapp.exit(-2);
        return -2;
    }

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
