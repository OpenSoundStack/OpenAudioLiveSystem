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

#include "modules.h"

void register_pipes(AudioPlumber* plumber, AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper) {
    plumber->register_pipe_element("audioin", []() {
        return std::make_shared<AudioInPipe>();
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
