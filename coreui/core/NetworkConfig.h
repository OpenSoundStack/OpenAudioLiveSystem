// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include <string>
#include <cstdint>

#include <QJsonObject>

struct NetworkConfig {
    std::string eth_interface;
    // hint_uid: 0 = no hint, static-range value = pin (autoconfig skipped),
    // dynamic-range value = ignored with a warning (per design §2.5).
    uint16_t hint_uid = 0;
    // persisted_uid: the autoconfigurator's last-committed value, fed back
    // into the configurator as a "try this first" hint. Optional.
    uint16_t persisted_uid = 0;

    // After init_console runs, this is the committed UID (autoconfigured
    // or static-pinned).
    uint16_t uid = 0;

    QJsonObject serialize();
};

#endif //NETWORKCONFIG_H
