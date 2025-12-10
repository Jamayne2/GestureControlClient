
Md Shad <mss464@cornell.edu>
10:57 AM (4 hours ago)
to me

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"
#include "example_http_client_util.h"
#include "mpu6050.h"
#include "imu_filter.h"
#include "gesture.h"

#undef WIFI_SSID
#undef WIFI_PASSWORD
#define WIFI_SSID     "jamayne's wifi"
#define WIFI_PASSWORD "password"
#define HOST          "192.168.4.1"   // AP Pico IP

// ---------- Wi-Fi init ----------
static int init_wifi(void) {
    stdio_init_all();
    sleep_ms(2000);
    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43\n");
        return -1;
    }
    printf("WiFi stack up\n");
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID, WIFI_PASSWORD,
            CYW43_AUTH_WPA2_AES_PSK,
            30000))
    {
        printf("failed to connect to AP\n");
        return -1;
    }
    printf("Connected to AP '%s'\n", WIFI_SSID);
    return 0;
}

// ---------- IMU init ----------
static void init_imu(void) {
    // I2C setup (from mpu6050.h: I2C_CHAN, SDA_PIN, SCL_PIN, I2C_BAUD_RATE)
    i2c_init(I2C_CHAN, I2C_BAUD_RATE);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    // Reset/configure MPU6050
    mpu6050_reset();
    // Initialize filter state (this does an initial read)
    imu_filter_init();
}

// ---------- Send one gesture over Wi-Fi ----------
static void send_gesture_over_wifi(gesture_t g) {
    const char *name = gesture_name(g);   // "FORWARD", "LEFT", etc.
    char url[128];
    snprintf(url, sizeof(url), "/msg?msg=%s", name);
    EXAMPLE_HTTP_REQUEST_T req = (EXAMPLE_HTTP_REQUEST_T){0};
    req.hostname   = HOST;
    req.url        = url;
    req.headers_fn = http_client_header_print_fn;
    req.recv_fn    = http_client_receive_print_fn;
    printf("Sending gesture '%s' as GET http://%s%s\n", name, HOST, url);
    int rc = http_client_request_sync(cyw43_arch_async_context(), &req);
    printf("rc = %d, httpc_result = %d\n", rc, req.result);
    // 0 = OK, 4 = server closed after reply (fine for us)
    if (req.result == 0 || req.result == 4) {
        printf("Gesture '%s' sent OK.\n", name);
    } else {
        printf("Error sending gesture '%s'\n", name);
    }
}

// added this so it matches the website controls it was missing a stop feature
// ---------- Send STOP command ----------
static void send_stop_command(void) {
    char url[128];
    snprintf(url, sizeof(url), "/msg?msg=STOP");
    EXAMPLE_HTTP_REQUEST_T req = {0};
    req.hostname = HOST;
    req.url = url;
    req.headers_fn = http_client_header_print_fn;
    req.recv_fn = http_client_receive_print_fn;
    printf("Sending STOP command\n");
    int rc = http_client_request_sync(cyw43_arch_async_context(), &req);
    if (req.result == 0 || req.result == 4) {
        printf("STOP sent OK.\n");
    } else {
        printf("Error sending STOP\n");
    }
}

// ---------- Main loop ----------
int main(void)
{
    if (init_wifi() != 0) {
        return 1;
    }
    init_imu();
    
    gesture_t last_sent = GESTURE_NONE;
    int none_counter = 0;
    const int SEND_STOP_AFTER = 50;  // 50 loops @ 10ms = 500ms
    
    while (true) {
        // Update IMU + complementary filter
        imu_update_and_filter();  // also refreshes 'acceleration' array
        
        // Convert fixed-point accel to floats for gesture.c
        float ax = fix2float15(acceleration[0]);
        float ay = fix2float15(acceleration[1]);
        float az = fix2float15(acceleration[2]);
        
        // Use your gesture library
        gesture_t g = detect_gesture(ax, ay, az);
        
        // Only send FORWARD and BACKWARD gestures (ignore LEFT/RIGHT)
        if ((g == GESTURE_FORWARD || g == GESTURE_BACKWARD) && g != last_sent) {
            printf("Detected gesture: %s\n", gesture_name(g));
            send_gesture_over_wifi(g);
            last_sent = g;
            none_counter = 0;  // Reset idle counter
        }
        // Send STOP when idle for too long
        else if (g == GESTURE_NONE) {
            none_counter++;
            if (none_counter >= SEND_STOP_AFTER && last_sent != GESTURE_NONE) {
                printf("Idle detected, sending STOP\n");
                send_stop_command();
                last_sent = GESTURE_NONE;
                none_counter = 0;
            }
        }
        // Ignore LEFT/RIGHT gestures silently
        
        // Tune this delay to your IMU update rate
        sleep_ms(10);
    }
    
    cyw43_arch_deinit();
    return 0;
}
