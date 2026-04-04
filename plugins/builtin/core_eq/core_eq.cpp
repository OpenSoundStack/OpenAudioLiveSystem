// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "core_eq.h"

CoreEqPlugin::CoreEqPlugin() : PluginInterface() {

}

CoreEqPlugin::~CoreEqPlugin() {

}

bool CoreEqPlugin::plugin_init() {
    return true;
}

std::shared_ptr<AudioPipe> CoreEqPlugin::construct_pipe(AudioRouter *router, std::shared_ptr<NetworkMapper> nmapper) {
    return std::make_shared<CoreEqPipe>();
}

PipeElemDesc *CoreEqPlugin::construct_pipe_elem_desc(AudioRouter *router) {
    return new CoreEqElem(router);
}
