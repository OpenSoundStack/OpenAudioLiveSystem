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
    uint16_t uid;

    QJsonObject serialize();
};

#endif //NETWORKCONFIG_H
