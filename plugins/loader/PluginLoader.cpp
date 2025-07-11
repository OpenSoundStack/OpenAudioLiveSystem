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

std::optional<std::shared_ptr<PluginInterface>> PluginLoader::load_plugin(const std::string& plugin_path) {
    m_lib_handle = dlopen(plugin_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (m_lib_handle == nullptr) {
        std::cerr << LOG_PREFIX << "Failed to load plugin " << plugin_path << std::endl;
        std::cerr << LOG_PREFIX << "Error is " << dlerror() << std::endl;
    }

    auto plugin_iface_factory = get_lib_object<plugin_factory_t>("plugin_factory");
    if (!plugin_iface_factory.has_value()) {
        std::cerr << LOG_PREFIX << "Plugin Loader : No entry point found." << std::endl;
    }

    auto pname = get_lib_object<std::string*>("plugname");
    auto pvers = get_lib_object<uint32_t*>("plugver");
    auto pauth = get_lib_object<std::string*>("plugauth");

    std::cout << "Loading new plugin... ";
    if (pname.has_value()) {
        std::cout << *(pname.value()) << " ";
    }

    if (pauth.has_value()) {
        std::cout << "written by " << *(pauth.value()) << " ";
    }

    if (pvers.has_value()) {
        uint32_t ver = *(pvers.value());
        std::cout << "version " << (ver >> 16) << "." << ((ver >> 8) & 0xFF) << "." << (ver & 0xFF);
    }

    std::cout << std::endl;

    return { (*plugin_iface_factory)() };
}

void PluginLoader::release_plugin() {
    if (m_lib_handle) {
        dlclose(m_lib_handle);
    }
}
