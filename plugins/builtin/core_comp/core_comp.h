// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

#ifndef OALIVESYSTEM_CORE_COMP_H
#define OALIVESYSTEM_CORE_COMP_H

#include "plugins/loader/PluginInterface.h"

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
