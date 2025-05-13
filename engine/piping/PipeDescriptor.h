#ifndef PIPEDESCRIPTOR_H
#define PIPEDESCRIPTOR_H

#include <vector>
#include <functional>
#include <memory>

#include "OpenAudioNetwork/common/AudioPipe.h"

enum class PipeType {
    PORTAL_RX,
    PORTAL_TX,
    FILTER_LPF,
    FILTER_HPF,
    FILTER_EQ,
    COMPRESSOR,
    SFX_BUILTIN,
    SFX_EXTERNAL
};

struct ElemBlueprint {
    PipeType element_type;
    std::function<std::unique_ptr<AudioPipe>()> factory;
};

struct PipeBlueprint {
    std::vector<ElemBlueprint> pipe_elements;
};

#endif //PIPEDESCRIPTOR_H
