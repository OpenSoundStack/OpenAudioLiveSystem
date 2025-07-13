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

#ifndef MODULES_H
#define MODULES_H

#include <iostream>
#include <string>
#include <filesystem>

#include "piping/AudioPlumber.h"
#include "piping/io/AudioInPipe.h"
#include "piping/feedback/LevelMeasurePipe.h"
#include "piping/filtering/FiltHPFPipe.h"
#include "piping/filtering/FiltLPFPipe.h"
#include "piping/io/AudioSendMtx.h"
#include "piping/io/AudioInMtx.h"
#include "piping/io/AudioDirectOut.h"

#include "plugins/loader/PluginLoader.h"

void register_pipes(AudioPlumber* plumber, AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper);

void load_plugins(
    const std::shared_ptr<PluginLoader>& ploader,
    AudioPlumber* plumber,
    AudioRouter* router,
    std::shared_ptr<NetworkMapper> nmapper
);

#endif //MODULES_H
