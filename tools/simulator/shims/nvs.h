#pragma once

// In-memory stand-in for ESP-IDF's NVS, enough for the modes' persisted settings.

#include <cstddef>
#include <cstdint>

using esp_err_t = int;
using nvs_handle_t = uint32_t;
constexpr esp_err_t ESP_OK{0};
constexpr esp_err_t ESP_FAIL{-1};
constexpr esp_err_t ESP_ERR_NVS_NOT_FOUND{0x1102};

enum class nvs_open_mode_t
{
    NVS_READONLY,
    NVS_READWRITE,
};
// Unscoped aliases, as in the SDK.
constexpr nvs_open_mode_t NVS_READONLY{nvs_open_mode_t::NVS_READONLY};
constexpr nvs_open_mode_t NVS_READWRITE{nvs_open_mode_t::NVS_READWRITE};

enum nvs_type_t : int
{
    NVS_TYPE_ANY,
};

esp_err_t nvs_open(const char *name, nvs_open_mode_t mode, nvs_handle_t *handle);
void nvs_close(nvs_handle_t handle);
esp_err_t nvs_commit(nvs_handle_t handle);
esp_err_t nvs_erase_key(nvs_handle_t handle, const char *key);
esp_err_t nvs_find_key(nvs_handle_t handle, const char *key, nvs_type_t *type);

esp_err_t nvs_set_blob(nvs_handle_t handle, const char *key, const void *value, size_t length);
esp_err_t nvs_get_blob(nvs_handle_t handle, const char *key, void *value, size_t *length);
esp_err_t nvs_set_str(nvs_handle_t handle, const char *key, const char *value);
esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *value, size_t *length);

#define NVS_SCALAR(suffix, type)                                                                                       \
    esp_err_t nvs_set_##suffix(nvs_handle_t handle, const char *key, type value);                                      \
    esp_err_t nvs_get_##suffix(nvs_handle_t handle, const char *key, type *value);
NVS_SCALAR(u8, uint8_t)
NVS_SCALAR(u16, uint16_t)
NVS_SCALAR(i16, int16_t)
NVS_SCALAR(i64, int64_t)
#undef NVS_SCALAR
