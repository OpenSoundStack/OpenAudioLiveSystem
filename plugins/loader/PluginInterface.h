// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <memory>
#include <string>

#include "AudioPipe.h"
#include "PipeDesc.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/common/AudioRouter.h"

#include "engine/log.h"

#define PLUGIN_VERSION(x, y, z) extern "C" uint32_t plugver = ((x << 16) + (y << 8) + z)
#define PLUGIN_NAME(name) extern "C" const char* plugname = name
#define PLUGIN_AUTHOR(author) extern "C" const char* plugauth = author

#define PLUGIN_INTERFACE(plugclass) extern "C" std::shared_ptr<PluginInterface> plugin_factory() { return std::reinterpret_pointer_cast<PluginInterface>(std::make_shared<plugclass>()); }

class PluginInterface {
public:
    PluginInterface();
    virtual ~PluginInterface() = default;

    virtual bool plugin_init();
    virtual std::shared_ptr<AudioPipe> construct_pipe(AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper);
    virtual PipeElemDesc *construct_pipe_elem_desc(AudioRouter *router);
};

#endif //PLUGININTERFACE_H
