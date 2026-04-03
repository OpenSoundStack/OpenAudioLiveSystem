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

#include "core_comp.h"

PLUGIN_VERSION(1, 0, 0);
PLUGIN_NAME("Core Comp");
PLUGIN_AUTHOR("Mathis.D");

CoreCompPlugin::CoreCompPlugin() : PluginInterface() {

}

CoreCompPlugin::~CoreCompPlugin() {

}

bool CoreCompPlugin::plugin_init() {
    return true;
}

std::shared_ptr<AudioPipe> CoreCompPlugin::construct_pipe(AudioRouter *router, std::shared_ptr<NetworkMapper> nmapper) {
    return {};
}

PipeElemDesc *CoreCompPlugin::construct_pipe_elem_desc(AudioRouter *router) {
    return nullptr;
}
