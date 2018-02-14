#include <aos/aos.h>
#include "driver/rmt.h"

#include <hal/soc/pwm.h>

#include <cJSON.h>
#include <types.h>
#include <util.h>

// static const char *RMT_TX_TAG = "RMT Tx";

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define RMT_TX_GPIO 16

#define RMT_RX_CHANNEL RMT_CHANNEL_0 /*!< RMT channel for receiver */
#define RMT_RX_GPIO_NUM 17           /*!< GPIO number for receiver */
#define RMT_CLK_DIV 100
#define RMT_TICK_10_US (80000000 / RMT_CLK_DIV / 100000) /*!< RMT counter value for 10 us.(Source clock is APB clock) */
#define rmt_item32_tIMEOUT_US 9500                       /*!< RMT receiver timeout value(us) */

/*
 * Prepare a raw table with a message in the Morse code
 *
 * The message is "ESP" : . ... .--.
 *
 * The table structure represents the RMT item structure:
 * {duration, level, duration, level}
 *
 */
// rmt_item32_t items[] = {
//     // E : dot
//     {{{32767, 1, 32767, 0}}}, // dot
//     //
//     {{{32767, 0, 32767, 0}}}, // SPACE
//     // S : dot, dot, dot
//     {{{32767, 1, 32767, 0}}}, // dot
//     {{{32767, 1, 32767, 0}}}, // dot
//     {{{32767, 1, 32767, 0}}}, // dot
//     //
//     {{{32767, 0, 32767, 0}}}, // SPACE
//     // P : dot, dash, dash, dot
//     {{{32767, 1, 32767, 0}}}, // dot
//     {{{32767, 1, 32767, 1}}},
//     {{{32767, 1, 32767, 0}}}, // dash
//     {{{32767, 1, 32767, 1}}},
//     {{{32767, 1, 32767, 0}}}, // dash
//     {{{32767, 1, 32767, 0}}}, // dot

//     // RMT end marker
//     {{{0, 1, 0, 0}}}};

// /*
//  * @brief Build register value of waveform for NEC one data bit
//  */
// static inline void nec_fill_item_level(rmt_item32_t* item, int high_us, int low_us)
// {
//     item->level0 = 1;
//     item->duration0 = (high_us) / 10 * RMT_TICK_10_US;
//     item->level1 = 0;
//     item->duration1 = (low_us) / 10 * RMT_TICK_10_US;
// }

void sourceIR(void *arg)
{
    LOG("Start IR source");
    LINK *link = arg;

    rmt_config_t rmt_rx;
    rmt_rx.channel = RMT_RX_CHANNEL;
    rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = true;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
    rmt_config(&rmt_rx);
    rmt_driver_install(rmt_rx.channel, 1000, 0);

    RingbufHandle_t rb = NULL;
    //get RMT RX ringbuffer
    rmt_get_ringbuf_handle(rmt_rx.channel, &rb);
    rmt_rx_start(rmt_rx.channel, 1);
    uint32_t item_count = 0;
    while (rb)
    {
        size_t rx_size = 0;
        //try to receive data from ringbuffer.
        //RMT driver will push all the data it receives to its ringbuffer.
        //We just need to parse the value and return the spaces of ringbuffer.
        rmt_item32_t *item = (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, 1000);
        if (item)
        {
            item_count += rx_size;
            // uint16_t rmt_addr;
            // uint16_t rmt_cmd;
            // int offset = 0;
            // while(1) {
            //     //parse data value from ringbuffer.
            //     int res = nec_parse_items(item + offset, rx_size / 4 - offset, &rmt_addr, &rmt_cmd);
            //     if(res > 0) {
            //         offset += res + 1;
            //         LOG("RMT RCV --- addr: 0x%04x cmd: 0x%04x", rmt_addr, rmt_cmd);
            //     } else {
            //         break;
            //     }
            // }
            //after parsing the data, return spaces to ringbuffer.
            vRingbufferReturnItem(rb, (void *)item);
        }
        else
        {
            break;
        }
    }
    LOG("Received %d ir item", item_count);

    rmt_rx_stop(rmt_rx.channel);

    rmt_driver_uninstall(rmt_rx.channel);

    int32_t interval = jsonInt(link->sourceConfig, "interval");
    if (interval > 0)
    {
        aos_post_delayed_action(interval, sourceIR, arg);
    }

    LOG("End IR source");
}

void targetIR(void *arg)
{
    // LOG("Start targetIR");
    // LINK *link = arg;

    // rmt_config_t config;
    // config.rmt_mode = RMT_MODE_TX;
    // config.channel = RMT_TX_CHANNEL;
    // config.gpio_num = RMT_TX_GPIO;
    // config.mem_block_num = 1;
    // config.tx_config.loop_en = 0;
    // // enable the carrier to be able to hear the Morse sound
    // // if the RMT_TX_GPIO is connected to a speaker
    // config.tx_config.carrier_en = 1;
    // config.tx_config.idle_output_en = 1;
    // config.tx_config.idle_level = 0;
    // config.tx_config.carrier_duty_percent = 50;
    // // set audible career frequency of 611 Hz
    // // actually 611 Hz is the minimum, that can be set
    // // with current implementation of the RMT API
    // config.tx_config.carrier_freq_hz = 38000;
    // config.tx_config.carrier_level = 1;
    // // set the maximum clock divider to be able to output
    // // RMT pulses in range of about one hundred milliseconds
    // config.clk_div = 255;

    // rmt_config(&config);
    // rmt_driver_install(config.channel, 0, 0);

    // int number_of_items = sizeof(items) / sizeof(items[0]);

    // // while (1) {
    //     rmt_write_items(RMT_TX_CHANNEL, items, number_of_items, true);
    //     // ESP_LOGI(RMT_TX_TAG, "Transmission complete");
    //     // vTaskDelay(2000 / portTICK_PERIOD_MS);
    // // }
    // rmt_driver_uninstall(config.channel);

    // IOTLINK_FREE_MESSAGE(&(link->message));
    // LOG("End targetIR");

    LOG("Start targetIR");
    pwm_dev_t pwm;
    pwm_config_t config;
    config.duty_cycle = 0.5;
    config.freq = 38000;
    pwm.port = 16;
    pwm.config = config;

    hal_pwm_init(&pwm);
    hal_pwm_start(&pwm);
    aos_msleep(100);
    hal_pwm_stop(&pwm);
    aos_msleep(100);
    hal_pwm_finalize(&pwm);
    LOG("End targetIR");
}