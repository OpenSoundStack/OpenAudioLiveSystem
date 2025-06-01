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

#ifndef ELEMCONTROLDATA_H
#define ELEMCONTROLDATA_H

#include "OpenAudioNetwork/common/packet_structs.h"

class ElemControlData {
public:
    ElemControlData();
    virtual ~ElemControlData() = default;

    virtual void fill_packet(ControlPacket& packet) = 0;

    void reset_changed_flag();
    bool has_changed();

protected:
    bool m_changed_flag;
};

template<class T>
class GenericElemControlData : public ElemControlData {
public:
    GenericElemControlData(T init_value) {
        static_assert(sizeof(T) <= 4 * sizeof(uint32_t));
        m_data = init_value;
    }

    ~GenericElemControlData() override = default;

    void fill_packet(ControlPacket &packet) override {
        memcpy(packet.packet_data.data, &m_data, sizeof(T));
    }

    void set_data(T data) {
        m_data = data;
        m_changed_flag = true;
    }

    T& get_data() {
        return m_data;
    }

private:
    T m_data;
};

#endif //ELEMCONTROLDATA_H
