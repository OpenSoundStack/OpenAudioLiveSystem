#ifndef OSST_OANINSPECT_FILTER_H
#define OSST_OANINSPECT_FILTER_H

#include <cstdint>
#include <string>
#include <unordered_set>

// Filter expression — comma-separated key=value[,value...] conditions.
// Keys are ANDed; values within a key are ORed. Examples:
//   ethertype=audio,disco
//   ethertype=control,src=42
//   peer=100
//
// Keys: ethertype, src, dst, peer, uid (uid is alias for peer).
struct Filter {
    // 5-bit mask of EtypeIdx::COUNT entries. 0 means "no ethertype clause"
    // = pass everything; set bits mean "only these allowed".
    uint8_t  ethertype_mask{0};
    std::unordered_set<uint16_t> src_uids;   // empty = no clause
    std::unordered_set<uint16_t> dst_uids;
    std::unordered_set<uint16_t> peer_uids;  // matches src OR dst

    bool match(uint8_t etype_idx, uint16_t src_uid, uint16_t dst_uid) const;

    // Cheap "could this ethertype ever match, ignoring src/dst clauses?"
    // Used by the ingest fast-path to drop audio before allocating a ring
    // entry — we want to know "is this filter audio-permissive?" without
    // requiring the caller to know what src_uid to ask about.
    bool accepts_ethertype(uint8_t etype_idx) const {
        if (ethertype_mask == 0) return true;
        return (ethertype_mask & (1u << etype_idx)) != 0;
    }

    // Parse "k=v[,v2...][,k2=v3[,v4]]". Returns true on success; appends a
    // human-readable error to `err` on failure.
    bool parse(const std::string& expr, std::string& err);

    // True when no conditions are active.
    bool empty() const {
        return ethertype_mask == 0 && src_uids.empty()
            && dst_uids.empty() && peer_uids.empty();
    }
};

#endif
