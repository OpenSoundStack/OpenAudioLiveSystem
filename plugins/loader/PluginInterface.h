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
#define PLUGIN_NAME(name) extern "C" std::string plugname = name
#define PLUGIN_AUTHOR(author) extern "C" std::string plugauth = author

#define PLUGIN_INTERFACE(plugclass) extern "C" std::shared_ptr<PluginInterface> plugin_factory() { return std::reinterpret_pointer_cast<PluginInterface>(std::make_shared<plugclass>()); }

class PluginInterface {
public:
    PluginInterface();
    virtual ~PluginInterface() = default;

    virtual bool plugin_init();
    virtual std::shared_ptr<AudioPipe> construct_pipe(AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper);
    virtual PipeDesc* construct_pipe_desc(AudioRouter* router);
};

#endif //PLUGININTERFACE_H
