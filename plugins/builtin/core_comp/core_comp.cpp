// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
    return std::make_shared<CoreCompPipe>(router, nmapper);
}

PipeElemDesc *CoreCompPlugin::construct_pipe_elem_desc(AudioRouter *router) {
    return new CoreCompElem(router);
}
