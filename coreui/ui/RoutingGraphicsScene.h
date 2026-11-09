// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef ROUTING_GRAPHICS_SCENE_H
#define ROUTING_GRAPHICS_SCENE_H

#include <QGraphicsScene>

#include "routing_items/RackedIOItem.h"

class RoutingGraphicsScene : public QGraphicsScene {
public:
    RoutingGraphicsScene();
    ~RoutingGraphicsScene() override = default;
};


#endif // ROUTING_GRAPHICS_SCENE_H
