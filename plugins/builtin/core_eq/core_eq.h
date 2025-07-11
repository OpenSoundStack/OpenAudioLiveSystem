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
#ifndef CORE_EQ_H
#define CORE_EQ_H

#include "plugins/loader/PluginInterface.h"

#include <iostream>

PLUGIN_VERSION(1, 0, 0);
PLUGIN_NAME("Core Eq");
PLUGIN_AUTHOR("Mathis.D");

extern "C" class CoreEqPlugin : public PluginInterface {
public:
    CoreEqPlugin();
    ~CoreEqPlugin() override;

    bool plugin_init() override;
    std::shared_ptr<AudioPipe> construct_pipe(AudioRouter *router, std::shared_ptr<NetworkMapper> nmapper) override;
    PipeDesc *construct_pipe_desc(AudioRouter *router) override;
};

PLUGIN_INTERFACE(CoreEqPlugin);

#endif //CORE_EQ_H
