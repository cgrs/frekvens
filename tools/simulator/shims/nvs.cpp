#include "nvs.h"

#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace
{
std::map<std::string, std::vector<uint8_t>> store;
std::vector<std::string> namespaces;

std::string slot(nvs_handle_t handle, const char *key) { return namespaces[handle] + "/" + key; }

esp_err_t setRaw(nvs_handle_t handle, const char *key, const void *value, size_t length)
{
    const auto *bytes{static_cast<const uint8_t *>(value)};
    store[slot(handle, key)].assign(bytes, bytes + length);
    return ESP_OK;
}

esp_err_t getRaw(nvs_handle_t handle, const char *key, void *value, size_t *length)
{
    const auto found{store.find(slot(handle, key))};
    if (found == store.end())
    {
        return ESP_ERR_NVS_NOT_FOUND;
    }
    if (value == nullptr)
    {
        *length = found->second.size();
        return ESP_OK;
    }
    if (*length < found->second.size())
    {
        return ESP_FAIL;
    }
    std::memcpy(value, found->second.data(), found->second.size());
    *length = found->second.size();
    return ESP_OK;
}
} // namespace

esp_err_t nvs_open(const char *name, nvs_open_mode_t, nvs_handle_t *handle)
{
    namespaces.emplace_back(name);
    *handle = static_cast<nvs_handle_t>(namespaces.size() - 1U);
    return ESP_OK;
}
void nvs_close(nvs_handle_t) {}
esp_err_t nvs_commit(nvs_handle_t) { return ESP_OK; }
esp_err_t nvs_erase_key(nvs_handle_t handle, const char *key)
{
    return store.erase(slot(handle, key)) != 0U ? ESP_OK : ESP_ERR_NVS_NOT_FOUND;
}
esp_err_t nvs_find_key(nvs_handle_t handle, const char *key, nvs_type_t *)
{
    return store.contains(slot(handle, key)) ? ESP_OK : ESP_ERR_NVS_NOT_FOUND;
}

esp_err_t nvs_set_blob(nvs_handle_t handle, const char *key, const void *value, size_t length)
{
    return setRaw(handle, key, value, length);
}
esp_err_t nvs_get_blob(nvs_handle_t handle, const char *key, void *value, size_t *length)
{
    return getRaw(handle, key, value, length);
}
esp_err_t nvs_set_str(nvs_handle_t handle, const char *key, const char *value)
{
    return setRaw(handle, key, value, std::strlen(value) + 1U);
}
esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *value, size_t *length)
{
    return getRaw(handle, key, value, length);
}

#define NVS_SCALAR(suffix, type)                                                                                       \
    esp_err_t nvs_set_##suffix(nvs_handle_t handle, const char *key, type value)                                       \
    {                                                                                                                  \
        return setRaw(handle, key, &value, sizeof(type));                                                              \
    }                                                                                                                  \
    esp_err_t nvs_get_##suffix(nvs_handle_t handle, const char *key, type *value)                                      \
    {                                                                                                                  \
        size_t length{sizeof(type)};                                                                                   \
        return getRaw(handle, key, value, &length);                                                                    \
    }
NVS_SCALAR(u8, uint8_t)
NVS_SCALAR(u16, uint16_t)
NVS_SCALAR(i16, int16_t)
NVS_SCALAR(i64, int64_t)
