#ifndef OSST_OANINSPECT_DECODER_H
#define OSST_OANINSPECT_DECODER_H

#include <cstdint>
#include <cstddef>
#include <string>

// Options controlling decoder output. Owned by the caller; the decoder
// re-reads each call so toggling at runtime (e.g. 'h' for hex) takes
// effect immediately.
struct DecodeOpts {
    bool hex{false};
    bool color{true};
};

// Returns a single rendered line for the frame. Caller appends a newline.
// payload is the bytes the switch fanned out (ethhdr + LowLatHeader + OAN
// payload). Length checks are bounded; truncated frames render as best-effort.
std::string decode_frame_line(uint16_t ethertype,
                              uint16_t src_uid,
                              uint16_t dst_uid,
                              const uint8_t* payload,
                              size_t payload_len,
                              uint64_t now_ms,
                              const DecodeOpts& opts);

// Index used by the inspector for filter/colour selection. Mirrors
// sim_switch's Switch::EtypeIdx.
uint8_t etype_index_of(uint16_t ethertype);

#endif
