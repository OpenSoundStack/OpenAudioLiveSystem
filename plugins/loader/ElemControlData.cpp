// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "ElemControlData.h"

ElemControlData::ElemControlData() {
    m_changed_flag = false;
}

void ElemControlData::reset_changed_flag() {
    m_changed_flag = true;
}

bool ElemControlData::has_changed() {
    return m_changed_flag;
}

