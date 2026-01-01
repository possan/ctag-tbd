#pragma once

#include <stdint.h>

#define P4_SPI_REQUEST_MIDI_DATA_SIZE 320

// request sent from pico to p4, 1022 bytes long (-2 for fingerprint)
struct p4_spi_request {
    // offset 0
    uint32_t magic;
    // offset 4
    uint32_t synth_midi_length;
    // offset 8
    uint8_t synth_midi[320]; // midi data to p4 synth rack
    // offset 328
    uint32_t sequencer_tempo; // bpm * 100
    // offset 332
    uint8_t reserved[690];
    // offset 1022
};

#define P4_SPI_RESPONSE_USB_MIDI_DATA_SIZE 320

// response sent from p4 to pico, 1022 bytes long (-2 for fingerprint)
struct p4_spi_response {
    // offset 0
    uint32_t magic;
    // offset 4
    uint32_t usb_device_midi_length;
    // offset 8
    uint8_t usb_device_midi[320]; // usb midi data from p4 connected usb device(s)
    // offset 328
    uint8_t input_waveform[128];
    // offset 456
    uint8_t output_waveform[128];
    // offset 584
    uint8_t link_data[64];
    // offset 648
    uint32_t led_color;
    // offset 652
    uint32_t frame_counter;
    // offset 656
    uint32_t magic2;
    // offset 660
    uint8_t reserved[298];
    // offset 1022
};
