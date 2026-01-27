#include "SpiProtocolHelper.hpp"

SpiProtocolHelper::SpiProtocolHelper() {
    lastSeenRequestCounter = 0;
    nextResponseSequenceCounter = 100;
    canPrepareNextResponse = true;
    nextResponsePrepared = false;
}

bool SpiProtocolHelper::validateRequestPacket(p4_spi_request_header *header, p4_spi_request2 *response) {
    uint16_t req_crc = calcPayloadCrc((uint8_t*)response, header->payload_length);
    return req_crc == header->payload_crc;
}

bool SpiProtocolHelper::shouldPrepareNextResponse() {
    return canPrepareNextResponse;
}

void SpiProtocolHelper::markNextResponsePrepared(p4_spi_response_header *header, p4_spi_response2 *response) {
    header->payload_length = sizeof(p4_spi_response2);
    header->payload_crc = calcPayloadCrc((uint8_t*) response, header->payload_length);
    canPrepareNextResponse = false;
    nextResponsePrepared = true;
}

void SpiProtocolHelper::updateResponseBeforeSending(p4_spi_response_header *header, p4_spi_response2 *response) {
    header->magic = 0xCAFE;
    header->response_sequence_counter = nextResponseSequenceCounter;
}

uint16_t SpiProtocolHelper::calcPayloadCrc(uint8_t *data, uint16_t length) {
    // stupid crc
    uint16_t sum = 42;
    for(uint16_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum;
}

bool SpiProtocolHelper::shouldSendPreparedResponse() {
    return nextResponsePrepared;
}

void SpiProtocolHelper::queuedPreparedResponse() {
    nextResponsePrepared = false;
    nextResponseSequenceCounter = getNextSequence(nextResponseSequenceCounter);
}

uint8_t SpiProtocolHelper::getNextSequence(uint8_t currentNumber) {
    return 100 + ((currentNumber + 1) % 100);
}

void SpiProtocolHelper::markRequestSeen(uint8_t seq) {
    lastSeenRequestCounter = seq;
    canPrepareNextResponse = true;
}