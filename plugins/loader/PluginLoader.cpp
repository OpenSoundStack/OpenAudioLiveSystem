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
    m_loaded_plugin_map = std::unordered_map<std::string, PluginMeta>{};
}

PluginLoader::~PluginLoader() {
    release_plugins();
}

std::optional<PluginMeta> PluginLoader::load_plugin(const std::string& plugin_path) {
    void* lib_handle = dlopen(plugin_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (lib_handle == nullptr) {
        std::cerr << LOG_PREFIX << "Failed to load plugin " << plugin_path << std::endl;
        std::cerr << LOG_PREFIX << "Error is " << dlerror() << std::endl;
    }

    auto plugin_iface_factory = get_lib_object<plugin_factory_t>(lib_handle, "plugin_factory");
    if (!plugin_iface_factory.has_value()) {
        std::cerr << LOG_PREFIX << "Plugin Loader : No entry point found." << std::endl;
        return {};
    }

    auto pname = get_lib_object<std::string*>(lib_handle, "plugname");
    auto pvers = get_lib_object<uint32_t*>(lib_handle, "plugver");
    auto pauth = get_lib_object<std::string*>(lib_handle, "plugauth");

    PluginMeta meta{};
    meta.plugin_iface = (*plugin_iface_factory)();
    meta.lib_handle = lib_handle;

    std::cout << "Loading new plugin... ";
    if (pname.has_value()) {
        meta.plugin_name = std::string(*(pname.value()));
        std::cout << *(pname.value()) << " ";
    } else {
        meta.plugin_name = plugin_path;
    }

    if (pauth.has_value()) {
        meta.plugin_author = std::string(*(pauth.value()));
        std::cout << "written by " << *(pauth.value()) << " ";
    }

    if (pvers.has_value()) {
        meta.plugin_version = *(pvers.value());
        uint32_t ver = *(pvers.value());
        std::cout << "version " << (ver >> 16) << "." << ((ver >> 8) & 0xFF) << "." << (ver & 0xFF);
    } else {
        meta.plugin_version = 0;
    }

    std::cout << std::endl;

    m_loaded_plugin_map[plugin_path] = std::move(meta);
    return m_loaded_plugin_map[plugin_path];
}

void PluginLoader::release_plugins() {
    for (auto& p : m_loaded_plugin_map) {
        dlclose(p.second.lib_handle);
    }
}
