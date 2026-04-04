// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "modules.h"

void register_pipes(AudioPlumber* plumber, AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper) {
    plumber->register_pipe_element("audioin", [router]() {
        return std::make_shared<AudioInPipe>(router);
    });

    plumber->register_pipe_element("dbmeas", [router, nmapper]() {
        return std::make_shared<LevelMeasurePipe>(router, nmapper);
    });

    plumber->register_pipe_element("hpf1", []() {
        return std::make_shared<FiltHPFPipe>();
    });

    plumber->register_pipe_element("lpf1", [](){
        return std::make_shared<FiltLPFPipe>();
    });

    plumber->register_pipe_element("sendmtx", [router]() {
        return std::make_shared<AudioSendMtx>(router);
    });

    plumber->register_pipe_element("inmtx", []() {
        return std::make_shared<AudioInMtx>();
    });

    plumber->register_pipe_element("dirout", [router]() {
        return std::make_shared<AudioDirectOut>(router);
    });
}

void load_plugins(
    const std::shared_ptr<PluginLoader>& ploader,
    AudioPlumber* plumber,
    AudioRouter* router,
    std::shared_ptr<NetworkMapper> nmapper)
{
    std::filesystem::path plugin_root{ENGINE_PLUGIN_SYSLOCATION};

    for (auto& f : std::filesystem::directory_iterator(plugin_root)) {
        auto plugmeta = ploader->load_plugin(f.path());
        if (plugmeta.has_value()) {
            std::cout << "Loaded plugin !" << std::endl;

            PluginMeta& meta = plugmeta.value();
            auto piface = meta.plugin_iface;
            plumber->register_pipe_element(meta.plugin_name, [piface, router, nmapper]() {
                return piface->construct_pipe(router, nmapper);
            });
        }
    }
}
