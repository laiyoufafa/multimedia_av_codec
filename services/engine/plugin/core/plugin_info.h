/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_CORE_PLUGIN_INFO_H
#define PLUGIN_CORE_PLUGIN_INFO_H

#include "plugin_definition.h"
#include "format.h"

namespace OHOS {
namespace Media {
namespace Plugin {

/**
 * PluginInfo, which describes static information for a plugin, including basic plugin information,
 * such as the type, name, rank.
 *
 * Different types of plugins have their own extra information,
 * which is described in the "extra" field in the form of key-value.
 *
 * Note that the type, name, rating, and extra information describes the plugin as a whole;
 *
 */
struct PluginInfo {
    uint32_t apiVersion;
    std::string name;
    PluginType pluginType;
    std::string description;
    uint32_t rank;
    Format extra;
};

/**
 * Extra information about the plugin.
 * Describes the protocol types supported by the Source plugin for playback.
 */
#define PLUGIN_INFO_EXTRA_PROTOCOL          "protocol"  // NOLINT: macro constant

/**
 * Extra information about the plugin.
 * Describes the input source types supported by the Source plugin for record.
 */
#define PLUGIN_INFO_EXTRA_INPUT_TYPE        "inputType"  // NOLINT: macro constant

/**
 * Extra information about the plugin.
 * Describes the output types supported by the OutputSink plugin.
 */
#define PLUGIN_INFO_EXTRA_OUTPUT_TYPE        "outputType"  // NOLINT: macro constant

/**
 * Extra information about the plugin.
 * Describes the extensions supported by the Demuxer plugin.
 */
#define PLUGIN_INFO_EXTRA_EXTENSIONS        "extensions" // NOLINT: macro constant

/**
 * Extra information about the plugin.
 * Describes the CodecMode supported by the Codec plugin.
 *
 * ValueType: enum Plugin::CodecMode
 */
#define PLUGIN_INFO_EXTRA_CODEC_MODE        "codec_mode" // NOLINT: macro constant
} // namespace Plugin
} // namespace Media
} // namespace OHOS
#endif // PLUGIN_CORE_PLUGIN_INFO_H
