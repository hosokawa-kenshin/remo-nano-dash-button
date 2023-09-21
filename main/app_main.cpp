#include "stdio.h"
#include "pairing_code.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_commissioner.h>
#include <esp_matter_console.h>
#include <esp_matter_controller_console.h>
#include <esp_matter_controller_cluster_command.h>
#include <esp_matter_ota.h>
#include <esp_route_hook.h>

#include <device.h>

#include <app_reset.h>

#include "sdkconfig.h"
#include "led_strip.h"

static constexpr uint32_t TARGET_NODE_ID = 1;
static constexpr uint16_t TARGET_ENDPOINT_ID = 2;

static const char *TAG = "app_main";

/*Lチカはじめ*/
#define BLINK_GPIO 2

static uint8_t s_led_state = 0;

static led_strip_t *pStrip_a;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        pStrip_a->set_pixel(pStrip_a, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        pStrip_a->refresh(pStrip_a, 100);
    } else {
        /* Set all LED off to clear all pixels */
        pStrip_a->clear(pStrip_a, 50);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    pStrip_a = led_strip_init(CONFIG_BLINK_LED_RMT_CHANNEL, BLINK_GPIO, 1);
    /* Set all LED off to clear all pixels */
    pStrip_a->clear(pStrip_a, 50);
}

/*Lチカおわり*/

typedef void *app_driver_handle_t;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::PublicEventTypes::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    default:
        break;
    }
}

static void app_driver_button_toggle_cb(void*, void*)
{
    using namespace chip::app::Clusters;
    ESP_LOGI(TAG, "Toggle button pressed");
    chip::DeviceLayer::StackLock lock;
    controller::command_data_t *command_data = (controller::command_data_t *)esp_matter_mem_calloc(1, sizeof(controller::command_data_t));
    command_data->cluster_id = OnOff::Id;
    command_data->command_id = OnOff::Commands::Toggle::Id;
    command_data->command_data_count = 0;
    controller::cluster_command *cmd = chip::Platform::New<controller::cluster_command>(1, 2, command_data);
    cmd->send_command();
}

static app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_config_t config = button_driver_get_config();
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}

extern "C" void app_main()
{
    /*-----------------------------------LチカOK-------------------------------*/
    esp_err_t err = ESP_OK;
    /* Initialize the ESP NVS layer */
    /*不揮発性メモリ*/
    nvs_flash_init();
    app_driver_handle_t button_handle = app_driver_button_init();
    app_reset_button_register(button_handle);
    /*-----------------------------------LチカOK-------------------------------*/

    /* Matter start */
    err = esp_matter::start(app_event_cb);
    // esp-matter/components/esp_matter/esp_matter_core.cpp
    // 

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Matter start failed: %d", err);
    }
    /*-----------------------------------LチカOK-------------------------------*/

    esp_matter::console::diagnostics_register_commands();
    // esp-matter/components/esp_matter_console/esp_matter_console_diagnotices.cpp
    // matter esp diagnosticesコマンドを登録している．
    /*-----------------------------------LチカOK-------------------------------*/

    esp_matter::console::wifi_register_commands();
    // esp-matter/components/esp_matter_console/esp_matter_console_wifi.cpp
    // SSIDやPasswordを入力し，Wifiに繋げるコマンドを登録している．

    /*-----------------------------------LチカOK-------------------------------*/
    
    esp_matter::console::init();
    // esp-matter/componets/esp_matter_console/esp_mater_console.cpp
    // consoleの初期化をする．

    /*-----------------------------------Lチカ1回だけOK消灯-------------------------------*/

    esp_matter::lock::chip_stack_lock(portMAX_DELAY);

    /*-----------------------------------Lチカ1回だけOKずっと点灯-------------------------------*/
    
    esp_matter::commissioner::init(5580);
    // esp-matter/componets/esp_matter_controller/esp_matter_commissioner.cpp
    // コミッショナーの初期化を行う
    // 引数はcommissioner_port
    // 
    /*--------------esp_event_loop_create_default();---------------------LチカNO-------------------------------*/
    
    esp_matter::lock::chip_stack_unlock();
    //chip_
    /*-----------------------------------LチカNO-------------------------------*/

    esp_matter::console::controller_register_commands();
    // esp-matter/componets/esp_matter_controller/esp_matter_controller_console.cpp
    // pairing
    // .handler = controller_pairing_handler
    //   

    esp_matter::console::pairing_code_register_commands();
    //git/remo-nano-dash-bottan/main/pairing_code.cpp
    

    /*
    configure_led();
    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        // Toggle the LED state 
        s_led_state = !s_led_state;
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
    */
}
