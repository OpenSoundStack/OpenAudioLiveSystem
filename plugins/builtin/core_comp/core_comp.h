// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_CORE_COMP_H
#define OALIVESYSTEM_CORE_COMP_H

#include "plugins/loader/PluginInterface.h"

#include "CoreCompPipe.h"
#include "CoreCompElem.h"

extern "C" class CoreCompPlugin : public PluginInterface {
public:
    CoreCompPlugin();
    ~CoreCompPlugin() override;

    bool plugin_init() override;
    std::shared_ptr<AudioPipe> construct_pipe(AudioRouter *router, std::shared_ptr<NetworkMapper> nmapper) override;

    PipeElemDesc *construct_pipe_elem_desc(AudioRouter *router) override;
};

PLUGIN_INTERFACE(CoreCompPlugin);

#endif //OALIVESYSTEM_CORE_COMP_H
