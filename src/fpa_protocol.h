#pragma once

#pragma once

// -------------------------------
// Compile-time message selection
// -------------------------------

#define FPA_MSG_LIGHTS 0
#define FPA_MSG_TIME 1
#define FPA_MSG_SCORE 1
#define FPA_MSG_STATUS 0
#define FPA_MSG_NAMES 0
#define FPA_MSG_COMPETITION 0
#define FPA_MSG_U2F 0
#define FPA_MSG_CONTROL 0

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stddef.h>
#include <stdint.h>

namespace FPA {
// Protocol constants
static constexpr uint8_t SOH = 0x01;
static constexpr uint8_t EOT = 0x04;
static constexpr uint8_t DC3 = 0x13;
static constexpr uint8_t DC4 = 0x14;
static constexpr uint8_t STX = 0x02;

static constexpr size_t MAX_PACKET_SIZE = 64;

struct LightsState {
  char red[2];
  char green[2];
  char whiteRight[2];
  char whiteLeft[2];
};

struct TimeState {
  char status[2]; // 'R','N','J','B'
  char value[9];  // up to 8 chars + null
};

struct ScoreState {
  char scoreRight[3];
  char scoreLeft[3];

  char yellowRight[3];
  char redRight[3];
  char blackRight[2];

  char yellowLeft[3];
  char redLeft[3];
  char blackLeft[2];

  char priority[2];
  char period[8];

  char videoRight[2];
  char videoLeft[2];
};

struct CompetitorInfo {
  char bib[9];
  char name[21];
  char nation[4];
};

struct CompetitionInfo {
  char competition[9];
  char phase[3];
  char poule[9];
  char match[4];
};

struct U2FState {
  char timer[6];
  char pRight[2];
  char pLeft[2];
};

struct MatchControl {
  char value[16];
};

struct GlobalState {
  LightsState lights;
  TimeState time;
  ScoreState score;

  char matchStatus[2];
  char weapon[2];
  char serviceCall[2];
  char technicianCall[2];

  CompetitorInfo left;
  CompetitorInfo right;

  CompetitionInfo competition;
  U2FState u2f;
  MatchControl control;
};

// API
void init();
bool parsePacket(const uint8_t *data, size_t len);

void lockState();
void unlockState();

void registerDisplayTask(TaskHandle_t handle);

const GlobalState &getState();
} // namespace FPA