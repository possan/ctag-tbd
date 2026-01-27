#pragma once

#include "SpiProtocol.h"
#include <stdint.h>

class SpiProtocolHelper {
private:
  bool nextResponsePrepared;
  bool canPrepareNextResponse;

public:
  SpiProtocolHelper();
  bool shouldPrepareNextResponse();
  void markNextResponsePrepared(p4_spi_response_header *header, p4_spi_response2 *response);

  void updateResponseBeforeSending(p4_spi_response_header *header, p4_spi_response2 *response);
  bool shouldSendPreparedResponse();
  void queuedPreparedResponse();

  bool validateRequestPacket(p4_spi_request_header *header, p4_spi_request2 *request);

  uint16_t calcPayloadCrc(uint8_t *data, uint16_t length);
  uint8_t nextResponseSequenceCounter;
  uint8_t lastSeenRequestCounter;

  uint8_t getNextSequence(uint8_t currentNumber);
  void markRequestSeen(uint8_t seq);
};

