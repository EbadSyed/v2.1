#include "dtm.h"

static struct testmode_config cfg;

#define SUPPORTED_FEATURES (FEATURE_PACKET_EXTENSION | FEATURE_2M_PHY)

#define MAX_PDU_OCTETS 255

#define T_MIN 6  // Inter byte timeout is 5 msec in spec, add 1 msec for tolerance

#define TRANSCEIVER_LENGTH_HIGH_MAX 3

#define BIT(x) (1 << (x))

enum phy{
  PHY_NONE = 0,
  PHY_1M = 1,
  PHY_2M = 2,
  PHY_CODED_S_8 = 3,
  PHY_CODED_S_2 = 4,
  PHY_MAX
};

#define CALC_MAX_PDU_TIME(octet_time, packet_overhead_time) (MAX_PDU_OCTETS * (octet_time) + (packet_overhead_time))
static const uint16_t MAX_PDU_TIME[] = {
  [PHY_1M] = CALC_MAX_PDU_TIME(8, 80),
  [PHY_2M] = CALC_MAX_PDU_TIME(4, 44),
  [PHY_CODED_S_8] = CALC_MAX_PDU_TIME(64, 720),
  [PHY_CODED_S_2] = CALC_MAX_PDU_TIME(16, 462),
};

static struct {
  uint8_t data[2];
  uint8_t len;
  uint32_t last_byte_time;
} cmd_buffer;

struct setup{
  uint8_t transceiver_length_high;
  uint8_t phy;
};

static const struct setup default_setup = {
  .transceiver_length_high = 0,
  .phy = PHY_1M,
};

static struct setup setup;

enum cmd_type{
  CMD_TYPE_SETUP = 0,
  CMD_TYPE_RXTEST = 1,
  CMD_TYPE_TXTEST = 2,
  CMD_TYPE_TESTEND = 3,
  CMD_TYPE_MAX
};

enum feature{
  FEATURE_PACKET_EXTENSION = BIT(0),
  FEATURE_2M_PHY = BIT(1),
  FEATURE_STABLE_MODULATION_INDEX = BIT(2),
};

struct setup_cmd_packet{
  uint8_t control;
  uint8_t parameter;
  uint8_t dc;
};

struct transceiver_cmd_packet{
  uint8_t frequency;
  uint8_t length;
  uint8_t pkt;
};

struct cmd_packet{
  enum cmd_type cmd_type;
  union {
    struct setup_cmd_packet setup;
    struct transceiver_cmd_packet transceiver;
  } cmd;
};

static struct {
  enum cmd_type transceiver_cmd;
} test_state;

enum setup_cmd{
  SETUP_CMD_RESET = 0,
  SETUP_CMD_TRANSCEIVER_LENGTH_HIGH = 1,
  SETUP_CMD_PHY = 2,
  SETUP_CMD_TX_MODULATION_INDEX = 3,
  SETUP_CMD_READ_SUPPORTED_FEATURES = 4,
  SETUP_CMD_READ_PDU_PARAMETERS = 5,
};

enum setup_parameter{
  STANDARD_MODULATION_INDEX = 0,
};

enum pdu_parameter{
  PDU_PARAMETER_MAX_TX_OCTETS = 0,
  PDU_PARAMETER_MAX_TX_TIME = 1,
  PDU_PARAMETER_MAX_RX_OCTETS = 2,
  PDU_PARAMETER_MAX_RX_TIME = 3,
};

enum test_status{
  TEST_STATUS_SUCCESS = 0,
  TEST_STATUS_ERROR = 1,
};

static void reset_setup()
{
  setup = default_setup;
}

static void reset_cmd_buffer()
{
  cmd_buffer.len = 0;
}

static void reset_transceiver_test_state()
{
  test_state.transceiver_cmd = CMD_TYPE_MAX;
}

static void set_transceiver_test_state(enum cmd_type cmd)
{
  test_state.transceiver_cmd = cmd;
}

static enum cmd_type get_transceiver_test_state()
{
  return test_state.transceiver_cmd;
}

static inline uint32_t t_min_in_ticks()
{
  return T_MIN * cfg.ticks_per_second / 1000;
}

static void send_test_status(uint8_t status, uint16_t response)
{
  response &= 0x3fff;

  uint8_t response_high = response >> 7;
  uint8_t response_low = response & 0x7f;

  cfg.write_response_byte(response_high);
  cfg.write_response_byte((response_low << 1) | (status ? TEST_STATUS_ERROR : TEST_STATUS_SUCCESS));
}

static void send_packet_counter(uint16 counter)
{
  counter |= 0x8000;  // EV bit on
  cfg.write_response_byte(counter >> 8);
  cfg.write_response_byte(counter & 0xff);
}

void testmode_init(const struct testmode_config *config)
{
  cfg = *config;
  reset_setup();
  reset_cmd_buffer();
  reset_transceiver_test_state();
}

void testmode_process_command_byte(uint8_t byte)
{
  if (cmd_buffer.len >= sizeof(cmd_buffer.data)) {
    // Processing previous command => ignore byte
    return;
  }

  uint32_t current_byte_time = cfg.get_ticks();

  if (cmd_buffer.len
      && current_byte_time - cmd_buffer.last_byte_time > t_min_in_ticks()) {
    // Inter byte timeout occurred
    reset_cmd_buffer();
  }

  cmd_buffer.last_byte_time = current_byte_time;
  cmd_buffer.data[cmd_buffer.len++] = byte;

  if (cmd_buffer.len == sizeof(cmd_buffer.data)) {
    gecko_external_signal(cfg.command_ready_signal);
  }
}

static void parse_cmd_buffer(struct cmd_packet *result)
{
  result->cmd_type = (enum cmd_type)(cmd_buffer.data[0] >> 6);
  uint8_t field_1 = cmd_buffer.data[0] & 63;
  uint8_t field_2 = cmd_buffer.data[1] >> 2;
  uint8_t field_3 = cmd_buffer.data[1] & 0x3;

  switch (result->cmd_type) {
    case CMD_TYPE_SETUP:
    case CMD_TYPE_TESTEND:
      result->cmd.setup.control = field_1;
      result->cmd.setup.parameter = field_2;
      result->cmd.setup.dc = field_3;
      break;

    case CMD_TYPE_RXTEST:
    case CMD_TYPE_TXTEST:
      result->cmd.transceiver.frequency = field_1;
      result->cmd.transceiver.length = field_2;
      result->cmd.transceiver.pkt = field_3;
      break;
    default:
      break;
  }
}

static void process_setup_command(enum cmd_type cmd_type,
                                  struct setup_cmd_packet *cmd)
{
  if (cmd->control == SETUP_CMD_RESET) {
    reset_setup();
    gecko_cmd_test_dtm_end();
    /* Don't send test status here, because when DTM end is processed, a DTM
     * completed event is emitted, and the test status is sent in the event
     * handler. */
    return;
  }

  uint8_t status = TEST_STATUS_SUCCESS;
  uint16_t response = 0;

  switch (cmd->control) {
    case SETUP_CMD_PHY:
      if (cmd->parameter == PHY_NONE || cmd->parameter >= PHY_MAX) {
        status = TEST_STATUS_ERROR;
      } else {
        setup.phy = cmd->parameter;
      }
      break;

    case SETUP_CMD_TRANSCEIVER_LENGTH_HIGH:
      if (cmd->parameter > TRANSCEIVER_LENGTH_HIGH_MAX) {
        status = TEST_STATUS_ERROR;
      } else {
        setup.transceiver_length_high = cmd->parameter;
      }
      break;

    case SETUP_CMD_TX_MODULATION_INDEX:
      // only standard modulation index is supported.
      if (cmd->parameter != STANDARD_MODULATION_INDEX) {
        status = TEST_STATUS_ERROR;
      }
      break;

    case SETUP_CMD_READ_SUPPORTED_FEATURES:
      if (cmd->parameter != 0) {
        status = TEST_STATUS_ERROR;
      } else {
        response = SUPPORTED_FEATURES;
      }
      break;

    case SETUP_CMD_READ_PDU_PARAMETERS:
      switch (cmd->parameter) {
        case PDU_PARAMETER_MAX_TX_OCTETS:
        case PDU_PARAMETER_MAX_RX_OCTETS:
          response = MAX_PDU_OCTETS;
          break;

        case PDU_PARAMETER_MAX_TX_TIME:
        case PDU_PARAMETER_MAX_RX_TIME:
          response = MAX_PDU_TIME[setup.phy] / 2;
          break;

        default:
          status = TEST_STATUS_ERROR;
          break;
      }
      break;

    default:
      /* Unsupported setup command */
      status = TEST_STATUS_ERROR;
      break;
  }

  send_test_status(status, response);
  reset_cmd_buffer();
}

static void process_transceiver_command(enum cmd_type cmd_type,
                                        struct transceiver_cmd_packet *cmd)
{
  uint16_t length = (setup.transceiver_length_high << 6) | cmd->length;

  switch (cmd_type) {
    case CMD_TYPE_RXTEST:
      gecko_cmd_test_dtm_rx(cmd->frequency, setup.phy);
      break;

    case CMD_TYPE_TXTEST:
      gecko_cmd_test_dtm_tx(cmd->pkt, length, cmd->frequency, setup.phy);
      break;

    default:
      break;
  }
}

static void process_command()
{
  struct cmd_packet cmd_packet;
  parse_cmd_buffer(&cmd_packet);

  switch (cmd_packet.cmd_type) {
    case CMD_TYPE_SETUP:
      process_setup_command(cmd_packet.cmd_type, &cmd_packet.cmd.setup);
      break;

    case CMD_TYPE_RXTEST:
    case CMD_TYPE_TXTEST:
      process_transceiver_command(cmd_packet.cmd_type, &cmd_packet.cmd.transceiver);
      set_transceiver_test_state(cmd_packet.cmd_type);
      break;

    case CMD_TYPE_TESTEND:
      gecko_cmd_test_dtm_end();
      break;
    default:
      break;
  }
}

static void handle_dtm_completed(struct gecko_cmd_packet *evt)
{
  struct cmd_packet cmd_packet;
  parse_cmd_buffer(&cmd_packet);

  if (get_transceiver_test_state() == CMD_TYPE_RXTEST && cmd_packet.cmd_type == CMD_TYPE_TESTEND) {
    send_packet_counter(evt->data.evt_test_dtm_completed.number_of_packets);
  } else {
    send_test_status(evt->data.evt_test_dtm_completed.result, 0);
  }

  // Command is executed and the response is sent => reset command buffer for next command
  reset_cmd_buffer();
  if (cmd_packet.cmd_type == CMD_TYPE_TESTEND) {
    reset_transceiver_test_state();
  }
}

int testmode_handle_gecko_event(struct gecko_cmd_packet *evt)
{
  switch (BGLIB_MSG_ID(evt->header)) {
    case gecko_evt_system_external_signal_id:
      if (evt->data.evt_system_external_signal.extsignals & cfg.command_ready_signal) {
        process_command();
        return 1;
      }
      break;

    case gecko_evt_test_dtm_completed_id:
      handle_dtm_completed(evt);
      return 1;
      break;
  }

  return 0;
}
