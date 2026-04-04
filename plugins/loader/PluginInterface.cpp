// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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

