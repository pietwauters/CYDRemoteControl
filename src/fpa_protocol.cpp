#include "fpa_protocol.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

namespace FPA {
static GlobalState g_state;
static SemaphoreHandle_t g_mutex;
static TaskHandle_t g_displayTaskHandle = nullptr;

// ----------------------------------------------------------
// Message caches
// ----------------------------------------------------------

static uint8_t g_lastMsg1[MAX_PACKET_SIZE];
static size_t g_lastMsg1Len = 0;

static uint8_t g_lastMsg2[MAX_PACKET_SIZE];
static size_t g_lastMsg2Len = 0;

static uint8_t g_lastMsg3[MAX_PACKET_SIZE];
static size_t g_lastMsg3Len = 0;

static uint8_t g_lastMsg4[MAX_PACKET_SIZE];
static size_t g_lastMsg4Len = 0;

static uint8_t g_lastMsg5[MAX_PACKET_SIZE];
static size_t g_lastMsg5Len = 0;

static uint8_t g_lastMsg6[MAX_PACKET_SIZE];
static size_t g_lastMsg6Len = 0;

static uint8_t g_lastMsg7[MAX_PACKET_SIZE];
static size_t g_lastMsg7Len = 0;

static uint8_t g_lastMsg8[MAX_PACKET_SIZE];
static size_t g_lastMsg8Len = 0;

static uint8_t g_lastMsg9[MAX_PACKET_SIZE];
static size_t g_lastMsg9Len = 0;

void registerDisplayTask(TaskHandle_t handle) { g_displayTaskHandle = handle; }

static bool nextField(const uint8_t *data, size_t len, size_t &pos,
                      const uint8_t *&fieldStart, size_t &fieldLen) {
  if (pos >= len || data[pos] != STX)
    return false;

  size_t start = pos + 1;
  size_t i = start;

  while (i < len && data[i] != STX && data[i] != EOT)
    i++;

  fieldStart = &data[start];
  fieldLen = i - start;
  pos = i;

  return true;
}

static bool isSameAsLast(const uint8_t *data, size_t len, uint8_t *cache,
                         size_t &cacheLen) {
  if (len == cacheLen && memcmp(data, cache, len) == 0) {
    return true;
  }

  memcpy(cache, data, len);
  cacheLen = len;

  return false;
}

// ----------------------------------------------------------
// Utility
// ----------------------------------------------------------

static void copyField(char *dst, size_t dstSize, const uint8_t *src,
                      size_t len) {
  if (len >= dstSize)
    len = dstSize - 1;

  memcpy(dst, src, len);
  dst[len] = '\0';
}

static bool validateFrame(const uint8_t *data, size_t len) {
  if (!data || len < 3)
    return false;

  if (data[0] != SOH)
    return false;

  if (data[len - 1] != EOT)
    return false;

  return true;
}

// ----------------------------------------------------------
// Message 1 - Lights (Fixed length = 11)
// ----------------------------------------------------------

static bool parseLights(const uint8_t *data, size_t len, GlobalState &state) {
  if (len != 11)
    return false;

  // Format:
  // SOH DC4 R x G x W x w x EOT

  copyField(state.lights.red, 2, &data[3], 1);
  copyField(state.lights.green, 2, &data[5], 1);
  copyField(state.lights.whiteRight, 2, &data[7], 1);
  copyField(state.lights.whiteLeft, 2, &data[9], 1);

  return true;
}

// ----------------------------------------------------------
// Message 2 - Time
// ----------------------------------------------------------

static bool parseTime(const uint8_t *data, size_t len, GlobalState &state) {
  if (len < 7)
    return false;

  // Store timer state (data[2])
  state.time.status[0] = data[2];
  state.time.status[1] = '\0';

  // Expect STX at position 3
  if (data[3] != STX)
    return false;

  // Time field starts at 4 until EOT
  const uint8_t *start = &data[4];
  size_t timeLen = len - 5; // exclude EOT

  if (timeLen >= sizeof(state.time.value))
    timeLen = sizeof(state.time.value) - 1;

  memcpy(state.time.value, start, timeLen);
  state.time.value[timeLen] = '\0';
  printf("%s\n", state.time.value);
  return true;
}

// ----------------------------------------------------------
// Message 3 - Score / Cards / Priority
// Fixed length = 29 bytes
// ----------------------------------------------------------

static bool parseScore(const uint8_t *data, size_t len, GlobalState &state) {
  if (len != 29)
    return false;

  // Expected STX positions
  if (data[3] != STX)
    return false;
  if (data[9] != STX)
    return false;
  if (data[15] != STX)
    return false;
  if (data[21] != STX)
    return false;
  if (data[23] != STX)
    return false;
  if (data[25] != STX)
    return false;

  // Score field: XX:YY
  // Positions:
  // 4,5 = XX
  // 6 = ':'
  // 7,8 = YY

  if (data[6] != ':')
    return false;

  copyField(state.score.scoreRight, 3, &data[4], 2);
  copyField(state.score.scoreLeft, 3, &data[7], 2);

  // Right cards: ABb
  // 10,11 = yellowRight
  // 12,13 = redRight
  // 14 = blackRight

  copyField(state.score.yellowRight, 3, &data[10], 2);
  copyField(state.score.redRight, 3, &data[12], 2);
  copyField(state.score.blackRight, 2, &data[14], 1);

  // Left cards: CDd
  // 16,17 = yellowLeft
  // 18,19 = redLeft
  // 20 = blackLeft

  copyField(state.score.yellowLeft, 3, &data[16], 2);
  copyField(state.score.redLeft, 3, &data[18], 2);
  copyField(state.score.blackLeft, 2, &data[20], 1);

  // Priority
  // STX at 21
  // 22 = priority

  copyField(state.score.priority, 2, &data[22], 1);

  // Period (variable but in this fixed message max 1 char normally)
  // STX at 23
  // 24 = period
  // Next STX at 25

  if (data[25] != STX)
    return false;

  // period length = 1 byte (fixed in 29-byte layout)
  copyField(state.score.period, sizeof(state.score.period), &data[24], 1);

  // Video field
  // 26 = videoRight
  // 27 = videoLeft

  // If field contains space (0x20), do NOT overwrite (per your rule)
  if (data[26] != 0x20)
    copyField(state.score.videoRight, 2, &data[26], 1);

  if (data[27] != 0x20)
    copyField(state.score.videoLeft, 2, &data[27], 1);
  printf("Score: %s  %s\n", state.score.scoreLeft, state.score.scoreRight);
  return true;
}

static bool parseStatus(const uint8_t *data, size_t len, GlobalState &state) {
  if (len < 10)
    return false;

  size_t pos = 3; // after SOH DC3 'I'

  const uint8_t *field;
  size_t flen;

  // M
  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.matchStatus, 2, field, 1);

  // W
  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.weapon, 2, field, 1);

  // S
  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.serviceCall, 2, field, 1);

  // N
  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.technicianCall, 2, field, 1);

  return true;
}

static bool parseNameLeft(const uint8_t *data, size_t len, GlobalState &state) {
  size_t pos = 4; // after SOH DC3 N L

  const uint8_t *field;
  size_t flen;

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.left.bib, sizeof(state.left.bib), field, flen);

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.left.name, sizeof(state.left.name), field, flen);

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.left.nation, sizeof(state.left.nation), field, flen);

  return true;
}

static bool parseNameRight(const uint8_t *data, size_t len,
                           GlobalState &state) {
  size_t pos = 4;

  const uint8_t *field;
  size_t flen;

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.right.bib, sizeof(state.right.bib), field, flen);

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.right.name, sizeof(state.right.name), field, flen);

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.right.nation, sizeof(state.right.nation), field, flen);

  return true;
}

static bool parseCompetition(const uint8_t *data, size_t len,
                             GlobalState &state) {
  size_t pos = 4;

  const uint8_t *field;
  size_t flen;

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.competition.competition,
              sizeof(state.competition.competition), field, flen);

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.competition.phase, sizeof(state.competition.phase), field,
              flen);

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.competition.poule, sizeof(state.competition.poule), field,
              flen);

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.competition.match, sizeof(state.competition.match), field,
              flen);

  return true;
}

static bool parseU2F(const uint8_t *data, size_t len, GlobalState &state) {
  size_t pos = 4;

  const uint8_t *field;
  size_t flen;

  // Timer
  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.u2f.timer, sizeof(state.u2f.timer), field, flen);

  // P right
  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.u2f.pRight, 2, field, 1);

  // P left
  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.u2f.pLeft, 2, field, 1);

  return true;
}

static bool parseMatchControl(const uint8_t *data, size_t len,
                              GlobalState &state) {
  size_t pos = 4;

  const uint8_t *field;
  size_t flen;

  if (!nextField(data, len, pos, field, flen))
    return false;
  if (flen > 0)
    copyField(state.control.value, sizeof(state.control.value), field, flen);

  return true;
}

// ----------------------------------------------------------
// Public API
// ----------------------------------------------------------

void init() {
  memset(&g_state, 0, sizeof(g_state));
  g_mutex = xSemaphoreCreateMutex();
}

void lockState() { xSemaphoreTake(g_mutex, portMAX_DELAY); }

void unlockState() { xSemaphoreGive(g_mutex); }

const GlobalState &getState() { return g_state; }

static inline bool isMessageEnabled(const uint8_t *data) {
  // Message 1 (Lights)
#if FPA_MSG_LIGHTS
  if (data[1] == DC4)
    return true;
#endif

#if (FPA_MSG_TIME || FPA_MSG_SCORE || FPA_MSG_STATUS || FPA_MSG_NAMES ||       \
     FPA_MSG_COMPETITION || FPA_MSG_U2F || FPA_MSG_CONTROL)

  if (data[1] == DC3) {
    switch (data[2]) {
#if FPA_MSG_TIME
    case 'R':
    case 'N':
    case 'J':
    case 'B':
      return true;
#endif

#if FPA_MSG_SCORE
    case 'D':
      return true;
#endif

#if FPA_MSG_STATUS
    case 'I':
      return true;
#endif

#if FPA_MSG_NAMES
    case 'N':
      return true;
#endif

#if FPA_MSG_COMPETITION
    case 'M':
      return true;
#endif

#if FPA_MSG_U2F
    case 'U':
      return true;
#endif

#if FPA_MSG_CONTROL
    case 'F':
      return true;
#endif
    }
  }
#endif

  return false;
}

bool parsePacket(const uint8_t *data, size_t len) {
  if (!validateFrame(data, len))
    return false;

  if (!isMessageEnabled(data))
    return false;

  GlobalState temp;

  // Copy current state
  xSemaphoreTake(g_mutex, portMAX_DELAY);
  temp = g_state;
  xSemaphoreGive(g_mutex);

  bool ok = false;

#if FPA_MSG_LIGHTS
  if (data[1] == DC4) {
    if (isSameAsLast(data, len, g_lastMsg1, g_lastMsg1Len))
      return false;

    ok = parseLights(data, len, temp);
  } else
#endif

      if (data[1] == DC3) {
    switch (data[2]) {

#if FPA_MSG_TIME
    case 'R':
    case 'N':
    case 'J':
    case 'B':
      if (isSameAsLast(data, len, g_lastMsg2, g_lastMsg2Len))
        return false;
      ok = parseTime(data, len, temp);
      break;
#endif

#if FPA_MSG_SCORE
    case 'D':
      if (isSameAsLast(data, len, g_lastMsg3, g_lastMsg3Len))
        return false;
      ok = parseScore(data, len, temp);
      break;
#endif

#if FPA_MSG_STATUS
    case 'I':
      if (isSameAsLast(data, len, g_lastMsg4, g_lastMsg4Len))
        return false;
      ok = parseStatus(data, len, temp);
      break;
#endif

#if FPA_MSG_NAMES
    case 'N':
      if (data[3] == 'L') {
        if (isSameAsLast(data, len, g_lastMsg5, g_lastMsg5Len))
          return false;
        ok = parseNameLeft(data, len, temp);
      } else if (data[3] == 'R') {
        if (isSameAsLast(data, len, g_lastMsg6, g_lastMsg6Len))
          return false;
        ok = parseNameRight(data, len, temp);
      }
      break;
#endif

#if FPA_MSG_COMPETITION
    case 'M':
      if (data[3] == 'C') {
        if (isSameAsLast(data, len, g_lastMsg7, g_lastMsg7Len))
          return false;
        ok = parseCompetition(data, len, temp);
      }
      break;
#endif

#if FPA_MSG_U2F
    case 'U':
      if (data[3] == 'F') {
        if (isSameAsLast(data, len, g_lastMsg8, g_lastMsg8Len))
          return false;
        ok = parseU2F(data, len, temp);
      }
      break;
#endif

#if FPA_MSG_CONTROL
    case 'F':
      if (data[3] == 'C') {
        if (isSameAsLast(data, len, g_lastMsg9, g_lastMsg9Len))
          return false;
        ok = parseMatchControl(data, len, temp);
      }
      break;
#endif

    default:
      return false;
    }
  }

  if (!ok)
    return false;

  // Atomic state update
  xSemaphoreTake(g_mutex, portMAX_DELAY);
  g_state = temp;
  xSemaphoreGive(g_mutex);

  if (g_displayTaskHandle != nullptr)
    xTaskNotifyGive(g_displayTaskHandle);

  return true;
}

} // namespace FPA