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

#include "PluginInterface.h"

typedef std::optional<std::shared_ptr<PluginInterface>>(*plugin_factory_t)();

class PluginLoader {
public:
    PluginLoader();
    ~PluginLoader();

    std::optional<std::shared_ptr<PluginInterface>> load_plugin(const std::string& plugin_path);
    void release_plugin();

private:
    template<typename T>
    std::optional<T> get_lib_object(const std::string& obj_name) {
        if (m_lib_handle == nullptr) {
            return {};
        }

        void* obj = dlsym(m_lib_handle, obj_name.c_str());
        if (obj == nullptr) {
            return {};
        }

        return reinterpret_cast<T>(obj);
    }

    void* m_lib_handle;
};



#endif //PLUGINLOADER_H
