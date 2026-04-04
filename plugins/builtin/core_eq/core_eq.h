// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0
#ifndef CORE_EQ_H
#define CORE_EQ_H

#include "plugins/loader/PluginInterface.h"

#include "CoreEqElem.h"
#include "CoreEqPipe.h"

PLUGIN_VERSION(1, 0, 0);
PLUGIN_NAME("Core Eq");
PLUGIN_AUTHOR("Mathis.D");

extern "C" class CoreEqPlugin : public PluginInterface {
public:
    CoreEqPlugin();
    ~CoreEqPlugin() override;

    bool plugin_init() override;
    std::shared_ptr<AudioPipe> construct_pipe(AudioRouter *router, std::shared_ptr<NetworkMapper> nmapper) override;

    PipeElemDesc *construct_pipe_elem_desc(AudioRouter *router) override;
};

PLUGIN_INTERFACE(CoreEqPlugin);

#endif //CORE_EQ_H
