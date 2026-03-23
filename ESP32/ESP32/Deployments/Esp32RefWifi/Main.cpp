#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <Fw/Time/TimeInterval.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Console.hpp>
#include <Os/Os.hpp>
#include <ESP32/Deployments/Esp32RefWifi/Top/Esp32RefWifiTopology.hpp>
#include <FreeRTOS/Os/FreeRTOSSupport.hpp>

#if defined(TGT_OS_TYPE_ESP32)
extern "C" {
#include <esp_err.h>
#include <esp_event.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <nvs.h>
#include <nvs_flash.h>
}
#endif

namespace {

#if defined(TGT_OS_TYPE_ESP32)

static constexpr U32 STARTUP_TASK_STACK_BYTES = 8U * 1024U;
static constexpr TickType_t STARTUP_QUIESCE_TICKS = pdMS_TO_TICKS(1000);
static const Fw::TimeInterval RATE_GROUP_INTERVAL(0, 200000);
static const Fw::TimeInterval GROUND_CONNECT_POLL_INTERVAL(0, 100000);
static const Fw::TimeInterval GROUND_CONNECT_TIMEOUT(15, 0);
static constexpr const char* WIFI_CONFIG_PARTITION = "fprimecfg";
static constexpr const char* WIFI_CONFIG_NAMESPACE = "wifi";

static constexpr EventBits_t WIFI_CONNECTED_BIT = BIT0;
static constexpr EventBits_t WIFI_FAILURE_BIT = BIT1;

static esp_event_handler_instance_t s_wifi_event_instance = nullptr;
static esp_event_handler_instance_t s_ip_event_instance = nullptr;
static EventGroupHandle_t s_wifi_event_group = nullptr;
static bool s_default_nvs_initialized = false;
static bool s_config_nvs_initialized = false;

struct WifiRuntimeConfig {
    bool hasMode = false;
    Esp32RefWifi::WifiMode wifiMode = Esp32RefWifi::WifiMode::SOFT_AP;
    std::array<char, 33> apSsid = {};
    std::array<char, 65> apPassword = {};
    std::array<char, 16> apRemoteIp = {};
    U16 apRemotePort = 0;
    U16 apLocalPort = 0;
    U8 apChannel = 0;
    U8 apMaxConnections = 0;

    std::array<char, 33> staSsid = {};
    std::array<char, 65> staPassword = {};
    std::array<char, 16> staRemoteIp = {};
    U16 staRemotePort = 0;
    U16 staLocalPort = 0;

    U32 wifiRxBufferSize = 1024;
};

static WifiRuntimeConfig s_runtime_wifi_config = {};

const char* wifiDisconnectReasonToString(const wifi_err_reason_t reason) {
    switch (reason) {
        case WIFI_REASON_AUTH_EXPIRE:
            return "AUTH_EXPIRE";
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            return "4WAY_HANDSHAKE_TIMEOUT";
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            return "HANDSHAKE_TIMEOUT";
        case WIFI_REASON_AUTH_FAIL:
            return "AUTH_FAIL";
        case WIFI_REASON_ASSOC_FAIL:
            return "ASSOC_FAIL";
        case WIFI_REASON_CONNECTION_FAIL:
            return "CONNECTION_FAIL";
        case WIFI_REASON_NO_AP_FOUND:
            return "NO_AP_FOUND";
        case WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY:
            return "NO_AP_FOUND_W_COMPATIBLE_SECURITY";
        case WIFI_REASON_NO_AP_FOUND_IN_AUTHMODE_THRESHOLD:
            return "NO_AP_FOUND_IN_AUTHMODE_THRESHOLD";
        case WIFI_REASON_NO_AP_FOUND_IN_RSSI_THRESHOLD:
            return "NO_AP_FOUND_IN_RSSI_THRESHOLD";
        case WIFI_REASON_BEACON_TIMEOUT:
            return "BEACON_TIMEOUT";
        case WIFI_REASON_ASSOC_LEAVE:
            return "ASSOC_LEAVE";
        case WIFI_REASON_ASSOC_NOT_AUTHED:
            return "ASSOC_NOT_AUTHED";
        case WIFI_REASON_TIMEOUT:
            return "TIMEOUT";
        default:
            return "UNKNOWN";
    }
}

void boardConsoleWrite(const CHAR* message, FwSizeType size) {
    if ((message == nullptr) || (size == 0)) {
        return;
    }
    static_cast<void>(std::fwrite(message, 1, static_cast<size_t>(size), stdout));
    std::fflush(stdout);
}

void consoleLog(const char* format, ...) {
    char buffer[256] = {};
    va_list args;
    va_start(args, format);
    const int count = std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (count <= 0) {
        return;
    }

    const FwSizeType length =
        static_cast<FwSizeType>((count < static_cast<int>(sizeof(buffer))) ? count : (sizeof(buffer) - 1U));
    Os::Console::write(buffer, length);
    Os::Console::write("\n", 1);
}

void registerConsoleWriter() {
    Os::FreeRTOSSupport::registerConsoleWriter(&boardConsoleWrite);
}

bool initializeDefaultNvs() {
    if (s_default_nvs_initialized) {
        return true;
    }

    esp_err_t status = nvs_flash_init();
    if ((status == ESP_ERR_NVS_NO_FREE_PAGES) || (status == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        status = nvs_flash_erase();
        if (status != ESP_OK) {
            return false;
        }
        status = nvs_flash_init();
    }
    if (status == ESP_OK) {
        s_default_nvs_initialized = true;
    }
    return status == ESP_OK;
}

bool initializeConfigNvsPartition() {
    if (s_config_nvs_initialized) {
        return true;
    }

    esp_err_t status = nvs_flash_init_partition(WIFI_CONFIG_PARTITION);
    if ((status == ESP_ERR_NVS_NO_FREE_PAGES) || (status == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        status = nvs_flash_erase_partition(WIFI_CONFIG_PARTITION);
        if (status != ESP_OK) {
            return false;
        }
        status = nvs_flash_init_partition(WIFI_CONFIG_PARTITION);
    }

    if (status == ESP_OK) {
        s_config_nvs_initialized = true;
    }
    return status == ESP_OK;
}

template <size_t SIZE>
bool loadNvsString(nvs_handle_t handle, const char* key, std::array<char, SIZE>& destination) {
    size_t required_size = 0;
    esp_err_t status = nvs_get_str(handle, key, nullptr, &required_size);
    if (status == ESP_ERR_NVS_NOT_FOUND) {
        return false;
    }
    if (status != ESP_OK) {
        consoleLog("Wi-Fi config read failed for key '%s' (%d)", key, static_cast<int>(status));
        return false;
    }
    if (required_size > destination.size()) {
        consoleLog("Wi-Fi config key '%s' exceeds supported length", key);
        return false;
    }

    status = nvs_get_str(handle, key, destination.data(), &required_size);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi config read failed for key '%s' (%d)", key, static_cast<int>(status));
        return false;
    }
    return true;
}

bool loadNvsU8(nvs_handle_t handle, const char* key, U8& value) {
    uint8_t raw = 0;
    const esp_err_t status = nvs_get_u8(handle, key, &raw);
    if (status == ESP_ERR_NVS_NOT_FOUND) {
        return false;
    }
    if (status != ESP_OK) {
        consoleLog("Wi-Fi config read failed for key '%s' (%d)", key, static_cast<int>(status));
        return false;
    }
    value = raw;
    return true;
}

bool loadNvsU16(nvs_handle_t handle, const char* key, U16& value) {
    uint16_t raw = 0;
    const esp_err_t status = nvs_get_u16(handle, key, &raw);
    if (status == ESP_ERR_NVS_NOT_FOUND) {
        return false;
    }
    if (status != ESP_OK) {
        consoleLog("Wi-Fi config read failed for key '%s' (%d)", key, static_cast<int>(status));
        return false;
    }
    value = raw;
    return true;
}

bool loadProvisionedWifiConfig(WifiRuntimeConfig& config) {
    if (!initializeConfigNvsPartition()) {
        consoleLog("Wi-Fi config partition init failed; provision '%s' first", WIFI_CONFIG_PARTITION);
        return false;
    }

    nvs_handle_t handle = 0;
    const esp_err_t open_status =
        nvs_open_from_partition(WIFI_CONFIG_PARTITION, WIFI_CONFIG_NAMESPACE, NVS_READONLY, &handle);
    if (open_status == ESP_ERR_NVS_NOT_FOUND) {
        consoleLog("Wi-Fi config is missing; provision '%s' first", WIFI_CONFIG_PARTITION);
        return false;
    }
    if (open_status != ESP_OK) {
        consoleLog("Wi-Fi config open failed (%d)", static_cast<int>(open_status));
        return false;
    }

    bool loaded_any_value = false;
    U8 mode = 0;
    if (loadNvsU8(handle, "mode", mode)) {
        if (mode <= static_cast<U8>(Esp32RefWifi::WifiMode::STATION)) {
            config.wifiMode = static_cast<Esp32RefWifi::WifiMode>(mode);
            config.hasMode = true;
            loaded_any_value = true;
        } else {
            consoleLog("Wi-Fi config mode value %u is invalid", static_cast<unsigned>(mode));
        }
    }

    loaded_any_value = loadNvsString(handle, "ap_ssid", config.apSsid) || loaded_any_value;
    loaded_any_value = loadNvsString(handle, "ap_pass", config.apPassword) || loaded_any_value;
    loaded_any_value = loadNvsString(handle, "ap_remote_ip", config.apRemoteIp) || loaded_any_value;
    loaded_any_value = loadNvsString(handle, "sta_ssid", config.staSsid) || loaded_any_value;
    loaded_any_value = loadNvsString(handle, "sta_pass", config.staPassword) || loaded_any_value;
    loaded_any_value = loadNvsString(handle, "sta_remote_ip", config.staRemoteIp) || loaded_any_value;
    loaded_any_value = loadNvsU16(handle, "ap_remote_port", config.apRemotePort) || loaded_any_value;
    loaded_any_value = loadNvsU16(handle, "ap_local_port", config.apLocalPort) || loaded_any_value;
    loaded_any_value = loadNvsU16(handle, "sta_remote_port", config.staRemotePort) || loaded_any_value;
    loaded_any_value = loadNvsU16(handle, "sta_local_port", config.staLocalPort) || loaded_any_value;
    loaded_any_value = loadNvsU8(handle, "ap_channel", config.apChannel) || loaded_any_value;
    loaded_any_value = loadNvsU8(handle, "ap_max_conn", config.apMaxConnections) || loaded_any_value;

    nvs_close(handle);

    if (loaded_any_value) {
        consoleLog("Wi-Fi config loaded from NVS partition '%s'", WIFI_CONFIG_PARTITION);
    } else {
        consoleLog("Wi-Fi config partition '%s' is empty", WIFI_CONFIG_PARTITION);
    }
    return loaded_any_value;
}

bool validateProvisionedWifiConfig(const WifiRuntimeConfig& config) {
    if (!config.hasMode) {
        consoleLog("Wi-Fi config is missing required key 'mode'");
        return false;
    }

    switch (config.wifiMode) {
        case Esp32RefWifi::WifiMode::SOFT_AP:
            if (config.apSsid[0] == '\0') {
                consoleLog("Wi-Fi AP config is missing required key 'ap_ssid'");
                return false;
            }
            if (config.apRemoteIp[0] == '\0') {
                consoleLog("Wi-Fi AP config is missing required key 'ap_remote_ip'");
                return false;
            }
            if (config.apRemotePort == 0U) {
                consoleLog("Wi-Fi AP config is missing required key 'ap_remote_port'");
                return false;
            }
            if (config.apLocalPort == 0U) {
                consoleLog("Wi-Fi AP config is missing required key 'ap_local_port'");
                return false;
            }
            if (config.apChannel == 0U) {
                consoleLog("Wi-Fi AP config is missing required key 'ap_channel'");
                return false;
            }
            if (config.apMaxConnections == 0U) {
                consoleLog("Wi-Fi AP config is missing required key 'ap_max_conn'");
                return false;
            }
            return true;
        case Esp32RefWifi::WifiMode::STATION:
            if (config.staSsid[0] == '\0') {
                consoleLog("Wi-Fi STA config is missing required key 'sta_ssid'");
                return false;
            }
            if (config.staRemoteIp[0] == '\0') {
                consoleLog("Wi-Fi STA config is missing required key 'sta_remote_ip'");
                return false;
            }
            if (config.staRemotePort == 0U) {
                consoleLog("Wi-Fi STA config is missing required key 'sta_remote_port'");
                return false;
            }
            if (config.staLocalPort == 0U) {
                consoleLog("Wi-Fi STA config is missing required key 'sta_local_port'");
                return false;
            }
            return true;
        default:
            consoleLog("Wi-Fi config has unsupported mode value");
            return false;
    }
}

bool initializeWifiCommon() {
    if (!initializeDefaultNvs()) {
        consoleLog("Wi-Fi: failed to initialize NVS");
        return false;
    }

    esp_err_t status = esp_netif_init();
    if ((status != ESP_OK) && (status != ESP_ERR_INVALID_STATE)) {
        consoleLog("Wi-Fi: esp_netif_init failed (%d)", static_cast<int>(status));
        return false;
    }

    status = esp_event_loop_create_default();
    if ((status != ESP_OK) && (status != ESP_ERR_INVALID_STATE)) {
        consoleLog("Wi-Fi: esp_event_loop_create_default failed (%d)", static_cast<int>(status));
        return false;
    }

    const wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    status = esp_wifi_init(&init_config);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi: esp_wifi_init failed (%d)", static_cast<int>(status));
        return false;
    }

    status = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi: esp_wifi_set_storage failed (%d)", static_cast<int>(status));
        return false;
    }

    if (s_wifi_event_group == nullptr) {
        s_wifi_event_group = xEventGroupCreate();
        if (s_wifi_event_group == nullptr) {
            consoleLog("Wi-Fi: failed to create event group");
            return false;
        }
    }
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAILURE_BIT);
    return true;
}

void wifiEventHandler(void* argument,
                      esp_event_base_t event_base,
                      int32_t event_id,
                      void* event_data) {
    const auto mode = (argument != nullptr) ? *static_cast<const Esp32RefWifi::WifiMode*>(argument)
                                            : Esp32RefWifi::WifiMode::SOFT_AP;

    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_START:
                consoleLog("Wi-Fi AP started");
                break;
            case WIFI_EVENT_AP_STACONNECTED: {
                const auto* event = static_cast<wifi_event_ap_staconnected_t*>(event_data);
                consoleLog("Wi-Fi AP client connected: " MACSTR " aid=%d",
                           MAC2STR(event->mac),
                           static_cast<int>(event->aid));
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                const auto* event = static_cast<wifi_event_ap_stadisconnected_t*>(event_data);
                consoleLog("Wi-Fi AP client disconnected: " MACSTR " aid=%d",
                           MAC2STR(event->mac),
                           static_cast<int>(event->aid));
                break;
            }
            case WIFI_EVENT_STA_START:
                consoleLog("Wi-Fi STA started");
                static_cast<void>(esp_wifi_connect());
                break;
            case WIFI_EVENT_STA_CONNECTED:
                consoleLog("Wi-Fi STA connected to AP");
                break;
            case WIFI_EVENT_STA_DISCONNECTED: {
                const auto* event = static_cast<wifi_event_sta_disconnected_t*>(event_data);
                consoleLog("Wi-Fi STA disconnected from AP: reason=%u (%s)",
                           static_cast<unsigned>(event->reason),
                           wifiDisconnectReasonToString(static_cast<wifi_err_reason_t>(event->reason)));
                if (s_wifi_event_group != nullptr) {
                    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                    xEventGroupSetBits(s_wifi_event_group, WIFI_FAILURE_BIT);
                }
                if (mode == Esp32RefWifi::WifiMode::STATION) {
                    static_cast<void>(esp_wifi_connect());
                }
                break;
            }
            default:
                break;
        }
        return;
    }

    if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP)) {
        const auto* event = static_cast<ip_event_got_ip_t*>(event_data);
        consoleLog("Wi-Fi STA got IP: " IPSTR " gateway=" IPSTR, IP2STR(&event->ip_info.ip), IP2STR(&event->ip_info.gw));
        if (s_wifi_event_group != nullptr) {
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            xEventGroupClearBits(s_wifi_event_group, WIFI_FAILURE_BIT);
        }
    }
}

bool initializeWifiAccessPoint(const Esp32RefWifi::TopologyState& state) {
    if ((state.wifiSsid == nullptr) || (state.wifiSsid[0] == '\0')) {
        consoleLog("Wi-Fi AP mode requires an SSID");
        return false;
    }

    if (!initializeWifiCommon()) {
        return false;
    }

    if (esp_netif_create_default_wifi_ap() == nullptr) {
        consoleLog("Wi-Fi AP netif creation failed");
        return false;
    }

    esp_err_t status = esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, const_cast<Esp32RefWifi::WifiMode*>(&state.wifiMode), &s_wifi_event_instance);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi AP event registration failed (%d)", static_cast<int>(status));
        return false;
    }

    wifi_config_t wifi_config = {};
    std::snprintf(reinterpret_cast<char*>(wifi_config.ap.ssid),
                  sizeof(wifi_config.ap.ssid),
                  "%s",
                  state.wifiSsid);
    std::snprintf(reinterpret_cast<char*>(wifi_config.ap.password),
                  sizeof(wifi_config.ap.password),
                  "%s",
                  (state.wifiPassword != nullptr) ? state.wifiPassword : "");
    wifi_config.ap.ssid_len = std::strlen(state.wifiSsid);
    wifi_config.ap.channel = state.wifiChannel;
    wifi_config.ap.max_connection = state.wifiMaxConnections;
    wifi_config.ap.authmode = (wifi_config.ap.password[0] == '\0') ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    status = esp_wifi_set_mode(WIFI_MODE_AP);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi AP set mode failed (%d)", static_cast<int>(status));
        return false;
    }

    status = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi AP set config failed (%d)", static_cast<int>(status));
        return false;
    }

    status = esp_wifi_start();
    if (status != ESP_OK) {
        consoleLog("Wi-Fi AP start failed (%d)", static_cast<int>(status));
        return false;
    }

    consoleLog("Wi-Fi AP ready: ssid=%s channel=%u max_conn=%u target_gds=%s:%u",
               state.wifiSsid,
               static_cast<unsigned>(state.wifiChannel),
               static_cast<unsigned>(state.wifiMaxConnections),
               state.remoteIp,
               static_cast<unsigned>(state.remotePort));
    return true;
}

bool initializeWifiStation(const Esp32RefWifi::TopologyState& state) {
    if ((state.wifiSsid == nullptr) || (state.wifiSsid[0] == '\0')) {
        consoleLog("Wi-Fi station mode requires an SSID");
        return false;
    }

    if (!initializeWifiCommon()) {
        return false;
    }

    if (esp_netif_create_default_wifi_sta() == nullptr) {
        consoleLog("Wi-Fi STA netif creation failed");
        return false;
    }

    esp_err_t status = esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, const_cast<Esp32RefWifi::WifiMode*>(&state.wifiMode), &s_wifi_event_instance);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi STA event registration failed (%d)", static_cast<int>(status));
        return false;
    }

    status = esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, const_cast<Esp32RefWifi::WifiMode*>(&state.wifiMode), &s_ip_event_instance);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi STA IP event registration failed (%d)", static_cast<int>(status));
        return false;
    }

    wifi_config_t wifi_config = {};
    std::snprintf(reinterpret_cast<char*>(wifi_config.sta.ssid),
                  sizeof(wifi_config.sta.ssid),
                  "%s",
                  state.wifiSsid);
    std::snprintf(reinterpret_cast<char*>(wifi_config.sta.password),
                  sizeof(wifi_config.sta.password),
                  "%s",
                  (state.wifiPassword != nullptr) ? state.wifiPassword : "");
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    status = esp_wifi_set_mode(WIFI_MODE_STA);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi STA set mode failed (%d)", static_cast<int>(status));
        return false;
    }

    status = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi STA set config failed (%d)", static_cast<int>(status));
        return false;
    }

    status = esp_wifi_start();
    if (status != ESP_OK) {
        consoleLog("Wi-Fi STA start failed (%d)", static_cast<int>(status));
        return false;
    }

    status = esp_wifi_set_ps(WIFI_PS_NONE);
    if (status != ESP_OK) {
        consoleLog("Wi-Fi STA disable power save failed (%d)", static_cast<int>(status));
        return false;
    }

    const EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAILURE_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(15000));
    if ((bits & WIFI_CONNECTED_BIT) == 0U) {
        consoleLog("Wi-Fi STA did not become ready: event_bits=0x%02x connected=%u failure=%u",
                   static_cast<unsigned>(bits),
                   ((bits & WIFI_CONNECTED_BIT) != 0U) ? 1U : 0U,
                   ((bits & WIFI_FAILURE_BIT) != 0U) ? 1U : 0U);
        return false;
    }

    consoleLog("Wi-Fi STA ready: ssid=%s target_gds=%s:%u",
               state.wifiSsid,
               state.remoteIp,
               static_cast<unsigned>(state.remotePort));
    return true;
}

bool initializeWifi(const Esp32RefWifi::TopologyState& state) {
    switch (state.wifiMode) {
        case Esp32RefWifi::WifiMode::SOFT_AP:
            return initializeWifiAccessPoint(state);
        case Esp32RefWifi::WifiMode::STATION:
            return initializeWifiStation(state);
        default:
            consoleLog("Wi-Fi mode is invalid");
            return false;
    }
}

void applyWifiConfigToState(const WifiRuntimeConfig& config, Esp32RefWifi::TopologyState& state) {
    state.wifiMode = config.wifiMode;
    state.wifiChannel = config.apChannel;
    state.wifiMaxConnections = config.apMaxConnections;
    state.wifiRxBufferSize = config.wifiRxBufferSize;

    if (config.wifiMode == Esp32RefWifi::WifiMode::SOFT_AP) {
        state.wifiSsid = config.apSsid.data();
        state.wifiPassword = config.apPassword.data();
        state.remoteIp = config.apRemoteIp.data();
        state.remotePort = config.apRemotePort;
        state.localPort = config.apLocalPort;
    } else {
        state.wifiSsid = config.staSsid.data();
        state.wifiPassword = config.staPassword.data();
        state.remoteIp = config.staRemoteIp.data();
        state.remotePort = config.staRemotePort;
        state.localPort = config.staLocalPort;
    }
}

void logProvisionedWifiConfig(const Esp32RefWifi::TopologyState& state) {
    if (state.wifiMode == Esp32RefWifi::WifiMode::SOFT_AP) {
        consoleLog("Wi-Fi config: mode=softap ssid=%s channel=%u max_conn=%u local_port=%u remote=%s:%u rx_buffer=%u",
                   (state.wifiSsid != nullptr) ? state.wifiSsid : "",
                   static_cast<unsigned>(state.wifiChannel),
                   static_cast<unsigned>(state.wifiMaxConnections),
                   static_cast<unsigned>(state.localPort),
                   (state.remoteIp != nullptr) ? state.remoteIp : "",
                   static_cast<unsigned>(state.remotePort),
                   static_cast<unsigned>(state.wifiRxBufferSize));
        return;
    }

    consoleLog("Wi-Fi config: mode=station ssid=%s local_port=%u remote=%s:%u rx_buffer=%u",
               (state.wifiSsid != nullptr) ? state.wifiSsid : "",
               static_cast<unsigned>(state.localPort),
               (state.remoteIp != nullptr) ? state.remoteIp : "",
               static_cast<unsigned>(state.remotePort),
               static_cast<unsigned>(state.wifiRxBufferSize));
}

void run_esp32_ref_wifi() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    Os::init();
    registerConsoleWriter();

    Esp32RefWifi::TopologyState state;
    const bool config_loaded = loadProvisionedWifiConfig(s_runtime_wifi_config);
    FW_ASSERT(config_loaded);
    const bool config_valid = validateProvisionedWifiConfig(s_runtime_wifi_config);
    FW_ASSERT(config_valid);
    applyWifiConfigToState(s_runtime_wifi_config, state);
    logProvisionedWifiConfig(state);

    const bool wifi_ready = initializeWifi(state);
    FW_ASSERT(wifi_ready);

    Esp32RefWifi::setupTopology(state);
    consoleLog("Ground TCP connection attempt started: %s:%u",
               state.remoteIp,
               static_cast<unsigned>(state.remotePort));
    const bool ground_ready =
        Esp32RefWifi::waitForGroundConnection(GROUND_CONNECT_POLL_INTERVAL, GROUND_CONNECT_TIMEOUT);
    if (!ground_ready) {
        consoleLog("Ground TCP connection failed: %s:%u",
                   state.remoteIp,
                   static_cast<unsigned>(state.remotePort));
    }
    FW_ASSERT(ground_ready);
    consoleLog("Ground TCP connection established: %s:%u",
               state.remoteIp,
               static_cast<unsigned>(state.remotePort));
    vTaskDelay(STARTUP_QUIESCE_TICKS);
    Esp32RefWifi::startRateGroups(RATE_GROUP_INTERVAL);
}

static void run_esp32_ref_wifi_task(void* argument) {
    static_cast<void>(argument);
    run_esp32_ref_wifi();
    vTaskDelete(nullptr);
}

extern "C" void app_main() {
    const BaseType_t status = xTaskCreatePinnedToCore(run_esp32_ref_wifi_task,
                                                      "fprime_wifi_startup",
                                                      STARTUP_TASK_STACK_BYTES,
                                                      nullptr,
                                                      static_cast<UBaseType_t>(tskIDLE_PRIORITY + 4U),
                                                      nullptr,
                                                      tskNO_AFFINITY);
    FW_ASSERT(status == pdPASS, static_cast<FwAssertArgType>(status));
}

#else

void run_esp32_ref_wifi() {
    Os::init();

    Esp32RefWifi::TopologyState state;
    state.wifiMode = Esp32RefWifi::WifiMode::STATION;
    state.remoteIp = "127.0.0.1";
    state.remotePort = 50000;
    state.localPort = 50000;
    state.wifiSsid = "host-build";
    state.wifiPassword = "";

    Esp32RefWifi::setupTopology(state);
    Esp32RefWifi::startRateGroups(RATE_GROUP_INTERVAL);
}

int main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);
    run_esp32_ref_wifi();
    return 0;
}

#endif

}  // namespace
