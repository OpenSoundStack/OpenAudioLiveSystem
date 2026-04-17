// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef COMPPARAMS_H
#define COMPPARAMS_H

struct CompStaticParams {
    float threshold;
    float ratio;
    float gain;
};

struct CompDynamicsParams {
    int attack_ms;
    int release_ms;
    int hold_ms;
};

#endif //COMPPARAMS_H
