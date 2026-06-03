#include "Filter.h"

#include <cstdlib>

namespace {

// EtypeIdx must mirror Switch::EtypeIdx so the wire-side mask agrees with
// the decoder. Duplicated here so oaninspect doesn't need to pull in
// sim_switch's Switch.h.
constexpr uint8_t IDX_AUDIO   = 0;
constexpr uint8_t IDX_DISCO   = 1;
constexpr uint8_t IDX_CONTROL = 2;
constexpr uint8_t IDX_SYNC    = 3;
constexpr uint8_t IDX_OTHER   = 4;

int ethertype_name_to_idx(const std::string& n) {
    if (n == "audio")   return IDX_AUDIO;
    if (n == "disco")   return IDX_DISCO;
    if (n == "control") return IDX_CONTROL;
    if (n == "sync")    return IDX_SYNC;
    if (n == "other")   return IDX_OTHER;
    return -1;
}

bool parse_uid(const std::string& s, uint16_t& out, std::string& err) {
    if (s.empty()) { err = "empty uid value"; return false; }
    char* endp = nullptr;
    long v = std::strtol(s.c_str(), &endp, 0);
    if (!endp || *endp != '\0' || v < 0 || v > 0xFFFF) {
        err = "bad uid '" + s + "'";
        return false;
    }
    out = static_cast<uint16_t>(v);
    return true;
}

} // namespace

bool Filter::match(uint8_t etype_idx, uint16_t src_uid, uint16_t dst_uid) const {
    if (ethertype_mask != 0 && (ethertype_mask & (1u << etype_idx)) == 0) {
        return false;
    }
    if (!src_uids.empty() && !src_uids.count(src_uid)) return false;
    if (!dst_uids.empty() && !dst_uids.count(dst_uid)) return false;
    if (!peer_uids.empty()
        && !peer_uids.count(src_uid)
        && !peer_uids.count(dst_uid)) return false;
    return true;
}

bool Filter::parse(const std::string& expr, std::string& err) {
    *this = {};
    if (expr.empty()) return true;

    // Split into top-level clauses. The grammar mixes commas at two levels:
    // commas inside a value list ARE separators, but they only mean "next
    // alternative for the same key". We treat the whole input as a stream
    // of key=value tokens where the key sticks until a new key appears
    // (i.e. a "k=" prefix). That lets `ethertype=audio,disco,src=42`
    // parse cleanly without quoting.
    size_t i = 0;
    std::string cur_key;
    while (i < expr.size()) {
        size_t comma = expr.find(',', i);
        std::string tok = expr.substr(i, comma == std::string::npos ? std::string::npos : comma - i);
        if (tok.empty()) { i = comma + 1; continue; }

        auto eq = tok.find('=');
        std::string key, val;
        if (eq != std::string::npos) {
            key = tok.substr(0, eq);
            val = tok.substr(eq + 1);
            cur_key = key;
        } else {
            if (cur_key.empty()) {
                err = "value '" + tok + "' with no key";
                return false;
            }
            val = tok;
        }

        if (cur_key == "ethertype") {
            int idx = ethertype_name_to_idx(val);
            if (idx < 0) {
                err = "unknown ethertype '" + val + "' (expected audio/disco/control/sync/other)";
                return false;
            }
            ethertype_mask |= (1u << idx);
        } else if (cur_key == "src") {
            uint16_t u; if (!parse_uid(val, u, err)) return false;
            src_uids.insert(u);
        } else if (cur_key == "dst") {
            uint16_t u; if (!parse_uid(val, u, err)) return false;
            dst_uids.insert(u);
        } else if (cur_key == "peer" || cur_key == "uid") {
            uint16_t u; if (!parse_uid(val, u, err)) return false;
            peer_uids.insert(u);
        } else {
            err = "unknown filter key '" + cur_key + "'";
            return false;
        }

        if (comma == std::string::npos) break;
        i = comma + 1;
    }
    return true;
}
