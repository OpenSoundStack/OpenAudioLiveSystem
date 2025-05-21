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
