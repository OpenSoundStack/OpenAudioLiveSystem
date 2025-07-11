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

#include "core_eq.h"

CoreEqPlugin::CoreEqPlugin() : PluginInterface() {
    std::cout << "CORE EQ CTOR" << std::endl;
}

CoreEqPlugin::~CoreEqPlugin() {

}

bool CoreEqPlugin::plugin_init() {
    return true;
}

std::shared_ptr<AudioPipe> CoreEqPlugin::construct_pipe(AudioRouter *router, std::shared_ptr<NetworkMapper> nmapper) {
    return {};
}

PipeDesc *CoreEqPlugin::construct_pipe_desc(AudioRouter *router) {
    return nullptr;
}
