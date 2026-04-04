// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <dlfcn.h>

#include <string>
#include <optional>
#include <unordered_map>

#include "PluginInterface.h"

typedef std::shared_ptr<PluginInterface>(*plugin_factory_t)();

struct PluginMeta {
    std::string plugin_name;
    std::string plugin_author;
    uint32_t plugin_version;

    std::shared_ptr<PluginInterface> plugin_iface;
    void* lib_handle;
};

class PluginLoader {
public:
    PluginLoader();
    ~PluginLoader();

    std::optional<PluginMeta> load_plugin(const std::string& plugin_path);
    void release_plugins();

private:
    template<typename T>
    std::optional<T> get_lib_object(void* lib_handle, const std::string& obj_name) {
        if (lib_handle == nullptr) {
            return {};
        }

        void* obj = dlsym(lib_handle, obj_name.c_str());
        if (obj == nullptr) {
            return {};
        }

        return reinterpret_cast<T>(obj);
    }

    std::unordered_map<std::string, PluginMeta> m_loaded_plugin_map;
};



#endif //PLUGINLOADER_H
