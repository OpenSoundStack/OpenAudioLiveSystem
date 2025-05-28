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

