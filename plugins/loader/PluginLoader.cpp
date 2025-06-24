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

#include "PluginLoader.h"

PluginLoader::PluginLoader() {
    m_lib_handle = nullptr;
}

PluginLoader::~PluginLoader() {
    release_plugin();
}

bool PluginLoader::load_plugin(const std::string& plugin_path) {
    m_lib_handle = dlopen(plugin_path.c_str(), RTLD_NOW);

    return m_lib_handle != nullptr;
}

void PluginLoader::release_plugin() {
    if (m_lib_handle) {
        dlclose(m_lib_handle);
    }
}
