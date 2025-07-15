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
