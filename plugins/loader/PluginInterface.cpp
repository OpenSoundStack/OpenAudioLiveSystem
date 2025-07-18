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

#include "PluginInterface.h"

PluginInterface::PluginInterface() {

}

bool PluginInterface::plugin_init() {
    return false;
}

PipeElemDesc *PluginInterface::construct_pipe_elem_desc(AudioRouter *router) {
    return nullptr;
}

std::shared_ptr<AudioPipe> PluginInterface::construct_pipe(AudioRouter *router, std::shared_ptr<NetworkMapper> nmapper) {
    return {};
}

