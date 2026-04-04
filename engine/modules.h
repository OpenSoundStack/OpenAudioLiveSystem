// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
