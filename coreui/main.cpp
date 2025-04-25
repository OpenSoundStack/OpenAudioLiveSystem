#include <qapplication.h>
#include <qscreen.h>

#include "core/PipeElemLPF.h"
#include "ui/SignalWindow.h"
#include "ui/SetupWindow.h"

#include "ui/PipeVisualizer.h"

int main(int argc, char* argv[]) {
    QApplication qapp {argc, argv};

    SignalWindow signal_win{};
    SetupWindow setup_win{};

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

    QList<PipeVisualizer*> pvs;
    for (int i = 0; i < 8; i++) {
        pvs.append(new PipeVisualizer{});
    }

    auto* pdesc = new PipeDesc;
    pdesc->type = PET_FILTER;
    pdesc->desc_content = new PipeElemLPF{100.0f};

    pvs[0]->set_pipe_content(pdesc);

    signal_win.set_page_content(pvs);

    return qapp.exec();
}
