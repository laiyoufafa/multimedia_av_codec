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

#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <map>
#include <vector>

namespace OHOS {
namespace MediaAVCodec {
enum FormatDataType : uint32_t {
    /* None */
    FORMAT_TYPE_NONE,
    /* Int32 */
    FORMAT_TYPE_INT32,
    /* Int64 */
    FORMAT_TYPE_INT64,
    /* Float */
    FORMAT_TYPE_FLOAT,
    /* Double */
    FORMAT_TYPE_DOUBLE,
    /* String */
    FORMAT_TYPE_STRING,
    /* Addr */
    FORMAT_TYPE_ADDR,
};

struct FormatData {
    FormatDataType type = FORMAT_TYPE_NONE;
    union Val {
        int32_t int32Val;
        int64_t int64Val;
        float floatVal;
        double doubleVal;
    } val = {0};
    std::string stringVal = "";
    uint8_t *addr = nullptr;
    size_t size = 0;
};

class __attribute__((visibility("default"))) Format {
public:
    Format() = default;
    ~Format();

    Format(const Format &rhs);
    Format(Format &&rhs) noexcept;
    Format &operator=(const Format &rhs);
    Format &operator=(Format &&rhs) noexcept;

    /**
     * @brief Sets metadata of the integer type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value, which is a 32-bit integer.
     * @return Returns <b>true</b> if the setting is successful; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool PutIntValue(const std::string_view &key, int32_t value);

    /**
     * @brief Sets metadata of the long integer type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value, which is a 64-bit integer.
     * @return Returns <b>true</b> if the setting is successful; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool PutLongValue(const std::string_view &key, int64_t value);

    /**
     * @brief Sets metadata of the single-precision floating-point type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value, which is a single-precision floating-point number.
     * @return Returns <b>true</b> if the metadata is successfully set; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool PutFloatValue(const std::string_view &key, float value);

    /**
     * @brief Sets metadata of the double-precision floating-point type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value, which is a double-precision floating-point number.
     * @return Returns <b>true</b> if the setting is successful; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool PutDoubleValue(const std::string_view &key, double value);

    /**
     * @brief Sets metadata of the string type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value, which is a string.
     * @return Returns <b>true</b> if the metadata is successfully set; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool PutStringValue(const std::string_view &key, const std::string_view &value);

    /**
     * @brief Sets metadata of the string type.
     *
     * @param key Indicates the metadata key.
     * @param addr Indicates the metadata addr, which is a uint8_t *.
     * @param size Indicates the metadata addr size, which is a size_t.
     * @return Returns <b>true</b> if the metadata is successfully set; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool PutBuffer(const std::string_view &key, const uint8_t *addr, size_t size);

    /**
     * @brief Obtains the metadata value of the integer type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value to obtain, which is a 32-bit integer.
     * @return Returns <b>true</b> if the integer is successfully obtained; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool GetIntValue(const std::string_view &key, int32_t &value) const;

    /**
     * @brief Obtains the metadata value of the long integer type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value to obtain, which is a 64-bit long integer.
     * @return Returns <b>true</b> if the integer is successfully obtained; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool GetLongValue(const std::string_view &key, int64_t &value) const;

    /**
     * @brief Obtains the metadata value of the single-precision floating-point type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value to obtain, which is a single-precision floating-point number.
     * @return Returns <b>true</b> if the single-precision number is successfully obtained; returns
     * <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool GetFloatValue(const std::string_view &key, float &value) const;

    /**
     * @brief Obtains the metadata value of the double-precision floating-point type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value to obtain, which is a double-precision floating-point number.
     * @return Returns <b>true</b> if the double-precision number is successfully obtained; returns
     * <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool GetDoubleValue(const std::string_view &key, double &value) const;

    /**
     * @brief Obtains the metadata value of the string type.
     *
     * @param key Indicates the metadata key.
     * @param value Indicates the metadata value to obtain, which is a string.
     * @return Returns <b>true</b> if the string is successfully obtained; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool GetStringValue(const std::string_view &key, std::string &value) const;

    /**
     * @brief Obtains the metadata value of the string type.
     *
     * @param key Indicates the metadata key.
     * @param addr Indicates the metadata addr to obtain, which is a uint8_t **.
     * @param size Indicates the metadata addr size to obtain, which is a size_t.
     * @return Returns <b>true</b> if the string is successfully obtained; returns <b>false</b> otherwise.
     * @since 10
     * @version 1.0
     */
    bool GetBuffer(const std::string_view &key, uint8_t **addr, size_t &size) const;

    /**
     * @brief Query whether the key exists in this Format.
     *
     * @param key Indicates the metadata key.
     * @return true
     * @return false
     */
    bool ContainKey(const std::string_view &key) const;

    /**
     * @brief Get the value type for the key if the key exists in this Format.
     *
     * @param key Indicates the metadata key.
     * @return FormatDataType. If the key does not exists, return FORMAT_TYPE_NONE.
     */
    FormatDataType GetValueType(const std::string_view &key) const;

    /**
     * @brief Remove the key from the Format
     *
     * @param keys the key will be removed.
     */
    void RemoveKey(const std::string_view &key);

    /**
     * @brief A trick to enable the comparision between the std::string and std::string_view for
     * std::map, the trick called Transparent Comparator.
     *
     */
    using FormatDataMap = std::map<std::string, FormatData, std::less<>>;

    /**
     * @brief Obtains the metadata map.
     *
     * @return Returns the map object.
     * @since 10
     * @version 1.0
     */
    const FormatDataMap &GetFormatMap() const;

    /**
     * @brief Convert the metadata map to string.
     *
     * @return Returns a converted string.
     * @since 10
     * @version 1.0
     */
    std::string Stringify() const;

private:
    FormatDataMap formatMap_;
};
} // namespace MediaAVCodec
} // namespace OHOS
#endif // FORMAT_H
