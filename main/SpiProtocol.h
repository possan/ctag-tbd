#pragma once

#include <stdint.h>

#define P4_SPI_REQUEST_SIZE 512
#define P4_SPI_REQUEST_MIDI_DATA_SIZE 256
#define P4_SPI_RESPONSE_USB_MIDI_DATA_SIZE 256

#define P4_SPI_REQUEST_HEADER_SIZE 16
#define P4_SPI_RESPONSE_HEADER_SIZE 16

struct p4_spi_request_header {
    uint16_t magic;
    uint8_t request_sequence_counter;
    uint8_t reserved1[5];

    uint16_t payload_length;
    uint16_t payload_crc;
    uint32_t reserved2;
};

struct p4_spi_response_header {
    uint16_t magic;
    uint8_t response_sequence_counter;
    uint8_t reserved1[5];

    uint16_t payload_length;
    uint16_t payload_crc;
    uint32_t reserved2;
};

// request sent from pico to p4
struct p4_spi_request2 {
    // offset 0
    uint32_t magic;
    // offset 4
    uint32_t synth_midi_length;
    // offset 8
    uint8_t synth_midi[P4_SPI_REQUEST_MIDI_DATA_SIZE]; // midi data to p4 synth rack
    // offset 264
    uint32_t sequencer_tempo; // bpm * 100
    // offset 268
    uint32_t magic2;
    // offset 272
};

// response sent from p4 to pico
struct p4_spi_response2 {
    // offset 0
    uint32_t magic;
    // offset 4
    uint32_t usb_device_midi_length;
    // offset 8
    uint8_t usb_device_midi[P4_SPI_RESPONSE_USB_MIDI_DATA_SIZE]; // usb midi data from p4 connected usb device(s)
    // offset 264
    uint8_t input_waveform[64];
    // offset 328
    uint8_t output_waveform[64];
    // offset 392
    uint8_t link_data[64];
    // offset 456
    uint32_t led_color;
    // offset 460
    uint32_t magic2;
    // offset 464
};


// // request sent from pico to p4, 510 bytes long (-2 for fingerprint)
// struct p4_spi_request {
//     // offset 0
//     uint32_t magic;
//     // offset 4
//     uint32_t synth_midi_length;
//     // offset 8
//     uint8_t synth_midi[P4_SPI_REQUEST_MIDI_DATA_SIZE]; // midi data to p4 synth rack
//     // offset 264
//     uint32_t sequencer_tempo; // bpm * 100
//     // offset 268
//     uint32_t request_counter;
//     // offset 272
//     uint32_t magic2;
//     // offset 276
//     uint8_t reserved[234];
//     // offset 510
// };

// // response sent from p4 to pico, 510 bytes long (-2 for fingerprint)
// struct p4_spi_response {
//     // offset 0
//     uint32_t magic;
//     // offset 4
//     uint32_t usb_device_midi_length;
//     // offset 8
//     uint8_t usb_device_midi[P4_SPI_RESPONSE_USB_MIDI_DATA_SIZE]; // usb midi data from p4 connected usb device(s)
//     // offset 264
//     uint8_t input_waveform[64];
//     // offset 328
//     uint8_t output_waveform[64];
//     // offset 392
//     uint8_t link_data[64];
//     // offset 456
//     uint32_t led_color;
//     // offset 460
//     uint32_t response_counter;
//     // offset 464
//     uint32_t magic2;
//     // offset 468
//     uint8_t reserved[42];
//     // offset 510
// };
