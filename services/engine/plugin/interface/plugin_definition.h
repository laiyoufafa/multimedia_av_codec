/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGIN_INTF_PLUGIN_DEFINITION_H
#define PLUGIN_INTF_PLUGIN_DEFINITION_H

#include <string>
#include <memory>
#include <cstdint> // NOLINT: using int32_t in this file


namespace OHOS {
namespace Media {
namespace Plugin {
/**
 * @brief Macro definition, creating the version information.
 *
 * @details The versioning is the process of assigning a unique version number to a unique state
 * of plugin interface. Within a given version number category (major, minor), these numbers are
 * usually assigned in ascending order and correspond to new developments in the plugin.
 *
 * Given a version number MAJOR.MINOR:
 *  - MAJOR: When you make incompatible API changes.
 *  - MINOR: When you add features in a backwards-compatible manner or do backwards-compatible bug fixes.
 */
#define MAKE_VERSION(MAJOR, MINOR) ((((MAJOR)&0xFFFF) << 16) | ((MINOR)&0xFFFF))

/// Plugin interface major number
#define PLUGIN_INTERFACE_VERSION_MAJOR (1)

/// Plugin interface minor number
#define PLUGIN_INTERFACE_VERSION_MINOR (0)

/// Plugin interface version
#define PLUGIN_INTERFACE_VERSION MAKE_VERSION(PLUGIN_INTERFACE_VERSION_MAJOR, PLUGIN_INTERFACE_VERSION_MINOR)

/**
 * @enum License Type.
 * an official permission or permit.
 *
 * @since 10
 * @version 1.0
 */
enum struct LicenseType : uint8_t {
    APACHE_V2, ///< The Apache License 2.0
    LGPL,      ///< The GNU Lesser General Public License
    GPL,       ///< The GNU General Public License
    CC0,       ///< The Creative Commons Zero v1.0 Universal
    UNKNOWN,   ///< Unknown License
};

/**
 * @brief Definition of plugin packaging information.
 *
 * @since 10
 * @version 1.0
 */
struct PackageDef {
    uint32_t pkgVersion;    ///< Package information version, which indicates the latest plug-in interface version
                            ///< used by the plugin in the package. The default value is PLUGIN_INTERFACE_VERSION.

    std::string name;   ///< Package name. The plugin framework registers the plugin using this name.
                        ///< If the plugins are packaged as a dynamic library, the name of library
                        ///< must be in the format of "libplugin_<name>.so".

    LicenseType
        licenseType;    ///< The License information of the plugin in the package.
                        ///< The different plugins must be the same.
                        ///< The plugin framework processing in the plugin running state based on different license.
};

/**
 * @enum Plugin Return Status.
 *
 * @since 10
 * @version 1.0
 */
enum struct Status : int32_t {
    END_OF_STREAM = 1,                ///< Read source when end of stream
    OK = 0,                           ///< The execution result is correct.
    NO_ERROR = OK,                    ///< Same as Status::OK
    ERROR_UNKNOWN = -1,               ///< An unknown error occurred.
    ERROR_PLUGIN_ALREADY_EXISTS = -2, ///< The plugin already exists, usually occurs when in plugin registered.
    ERROR_INCOMPATIBLE_VERSION = -3,  ///< Incompatible version,
                                      ///< may occur during plugin registration or function calling.
    ERROR_NO_MEMORY = -4,             ///< The system memory is insufficient.
    ERROR_WRONG_STATE = -5,           ///< The function is called in an invalid state.
    ERROR_UNIMPLEMENTED = -6,         ///< This method or interface is not implemented.
    ERROR_INVALID_PARAMETER = -7,     ///< The plugin does not support this parameter.
    ERROR_INVALID_DATA = -8,          ///< The value is not in the valid range.
    ERROR_MISMATCHED_TYPE = -9,       ///< Mismatched data type
    ERROR_TIMED_OUT = -10,            ///< Operation timeout.
    ERROR_UNSUPPORTED_FORMAT = -11,   ///< The plugin not support this format/name.
    ERROR_NOT_ENOUGH_DATA = -12,      ///< Not enough data when read from source.
    ERROR_NOT_EXISTED = -13,          ///< Source is not existed.
    ERROR_AGAIN = -14,                ///< Operation is not available right now, should try again later.
    ERROR_PERMISSION_DENIED = -15,    ///< Permission denied.
    ERROR_NULL_POINTER = -16,         ///< Null pointer.
    ERROR_INVALID_OPERATION = -17,    ///< Invalid operation.
    ERROR_CLIENT = -18,               ///< Http client error
    ERROR_SERVER = -19,               ///< Http server error
    ERROR_DELAY_READY = -20,          ///< Delay ready event
};

/**
 * @enum Plugin Type.
 *
 * @since 10
 * @version 1.0
 */
enum struct PluginType : int32_t {
    INVALID_TYPE = -1, ///< Invalid plugin
    MUXER = 1,         ///< reference MuxerPlugin
    DEMUXER,           ///< reference DemuxerPlugin
};

/**
 * @brief Describes the basic information about the plugin.
 *
 * @since 10
 * @version 1.0
 */
struct PluginDefBase {
    uint32_t apiVersion; ///< Versions of different plugins. Different types of plugin have their own versions.

    PluginType pluginType = PluginType::INVALID_TYPE; ///< Describe the plugin type, e.g. 'source', 'codec'.

    std::string name;   ///< Indicates the name of a plugin. The name of the same type plugins must be unique.
                        ///< Plugins with the same name may fail to be registered.

    std::string description; ///< Detailed description of the plugin.

    uint32_t rank;  ///< Plugin score. The plugin with a high score may be preferred. You can evaluate the
                    ///< plugin score in terms of performance, version support, and license. Range: 0 to 100.
};

/**
 * @brief The plugin registration interface.
 * The plugin framework will provide the implementation.
 * Developers only need to invoke the API to register the plugin.
 *
 * @since 10
 * @version 1.0
 */
struct Register {
    virtual ~Register() = default;
    /**
     * @brief Register the plugin.
     *
     * @param def   Basic information about the plugin
     * @return  Registration status return
     *  @retval OK: The plugin is registered succeed.
     *  @retval ERROR_PLUGIN_ALREADY_EXISTS: The plugin already exists in plugin registered.
     *  @retval ERROR_INCOMPATIBLE_VERSION: Incompatible version during plugin registration.
     */
    virtual Status AddPlugin(const PluginDefBase& def) = 0;
};

/**
 * @brief The package registration interface.
 * The plugin framework will provide the implementation and auto invoke the API to
 * finish the package registration when plugin framework first time be initialized.
 *
 * @since 10
 * @version 1.0
 */
struct PackageRegister : Register {
    ~PackageRegister() override = default;

    /**
     * @brief Register the package.
     * During package registration, all plugins in the package are automatically registered.
     *
     * @param def   plugin packaging information.
     * @return  Registration status return
     *  @retval OK: The package is registered succeed without any errors.
     *  @retval ERROR_PLUGIN_ALREADY_EXISTS: The package or plugins already exists.
     *  @retval ERROR_INCOMPATIBLE_VERSION: Incompatible plugin interface version or api version.
     */
    virtual Status AddPackage(const PackageDef& def) = 0;
};

/// Plugin registration function, all plugins must be implemented.
using RegisterFunc = Status (*)(std::shared_ptr<Register> reg);

/// Plugin deregister function, all plugins must be implemented.
using UnregisterFunc = void (*)();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#if defined(__GNUC__) || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
#define PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define PLUGIN_EXPORT
#endif
#endif

/// Macro definition, string concatenation
#define PLUGIN_PASTE_ARGS(str1, str2) str1##str2

/// Macro definition, string concatenation
#define PLUGIN_PASTE(str1, str2) PLUGIN_PASTE_ARGS(str1, str2)

/// Macro definition, stringify
#define PLUGIN_STRINGIFY_ARG(str) #str

/// Macro definition, stringify
#define PLUGIN_STRINGIFY(str) PLUGIN_STRINGIFY_ARG(str)

/**
 * @brief Macro definition, Defines basic plugin information.
 * Which is invoked during plugin package registration. All plugin packages must be implemented.
 *
 * @param name              Package name. For details, @see PackageDef::name
 * @param license           Package License, For details, @see PackageDef::licenseType
 * @param registerFunc      Plugin registration function, MUST NOT be NULL.
 * @param unregisterFunc    Plugin deregister function，MUST NOT be NULL.
 */
#define PLUGIN_DEFINITION(name, license, registerFunc, unregisterFunc)                                                 \
    PLUGIN_EXPORT OHOS::Media::Plugin::Status PLUGIN_PASTE(register_, name)(                                           \
        const std::shared_ptr<OHOS::Media::Plugin::PackageRegister>& pkgReg)                                           \
    {                                                                                                                  \
        pkgReg->AddPackage({ PLUGIN_INTERFACE_VERSION, PLUGIN_STRINGIFY(name), license });                             \
        std::shared_ptr<OHOS::Media::Plugin::Register> pluginReg = pkgReg;                                             \
        return registerFunc(pluginReg);                                                                                \
    }                                                                                                                  \
    PLUGIN_EXPORT void PLUGIN_PASTE(unregister_, name)()                                                               \
    {                                                                                                                  \
        unregisterFunc();                                                                                              \
    }
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // PLUGIN_INTF_PLUGIN_DEFINITION_H
