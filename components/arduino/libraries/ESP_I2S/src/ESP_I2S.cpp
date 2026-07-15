// Disable the automatic pin remapping of the API calls in this file
#define ARDUINO_CORE_BUILD

#include "ESP_I2S.h"

#if SOC_I2S_SUPPORTED

#include "esp32-hal-periman.h"
#include "wav_header.h"
#include "mp3dec.h"
#include <limits.h>

#define I2S_READ_CHUNK_SIZE 1920

#define I2S_DEFAULT_CFG() \
  { .id = I2S_NUM_AUTO, .role = I2S_ROLE_MASTER, .dma_desc_num = 6, .dma_frame_num = 240, .auto_clear = true, }

#define I2S_STD_CHAN_CFG(_sample_rate, _data_bit_width, _slot_mode)                                                                   \
  {                                                                                                                                   \
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(_sample_rate), .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(_data_bit_width, _slot_mode), \
    .gpio_cfg = {                                                                                                                     \
      .mclk = (gpio_num_t)_mclk,                                                                                                      \
      .bclk = (gpio_num_t)_bclk,                                                                                                      \
      .ws = (gpio_num_t)_ws,                                                                                                          \
      .dout = (gpio_num_t)_dout,                                                                                                      \
      .din = (gpio_num_t)_din,                                                                                                        \
      .invert_flags =                                                                                                                 \
        {                                                                                                                             \
          .mclk_inv = _mclk_inv,                                                                                                      \
          .bclk_inv = _bclk_inv,                                                                                                      \
          .ws_inv = _ws_inv,                                                                                                          \
        },                                                                                                                            \
    },                                                                                                                                \
  }

#if SOC_I2S_SUPPORTS_TDM
#define I2S_TDM_CHAN_CFG(_sample_rate, _data_bit_width, _slot_mode, _mask)                                                                   \
  {                                                                                                                                          \
    .clk_cfg = I2S_TDM_CLK_DEFAULT_CONFIG(_sample_rate), .slot_cfg = I2S_TDM_PHILIP_SLOT_DEFAULT_CONFIG(_data_bit_width, _slot_mode, _mask), \
    .gpio_cfg = {                                                                                                                            \
      .mclk = (gpio_num_t)_mclk,                                                                                                             \
      .bclk = (gpio_num_t)_bclk,                                                                                                             \
      .ws = (gpio_num_t)_ws,                                                                                                                 \
      .dout = (gpio_num_t)_dout,                                                                                                             \
      .din = (gpio_num_t)_din,                                                                                                               \
      .invert_flags =                                                                                                                        \
        {                                                                                                                                    \
          .mclk_inv = _mclk_inv,                                                                                                             \
          .bclk_inv = _bclk_inv,                                                                                                             \
          .ws_inv = _ws_inv,                                                                                                                 \
        },                                                                                                                                   \
    },                                                                                                                                       \
  }
#endif
#if SOC_I2S_SUPPORTS_PDM_TX
#if (SOC_I2S_PDM_MAX_TX_LINES > 1)
#define I2S_PDM_TX_CHAN_CFG(_sample_rate, _data_bit_width, _slot_mode)                                                               \
  {                                                                                                                                  \
    .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(_sample_rate), .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(_data_bit_width, _slot_mode), \
    .gpio_cfg = {                                                                                                                    \
      .clk = (gpio_num_t)_tx_clk,                                                                                                    \
      .dout = (gpio_num_t)_tx_dout0,                                                                                                 \
      .dout2 = (gpio_num_t)_tx_dout1,                                                                                                \
      .invert_flags =                                                                                                                \
        {                                                                                                                            \
          .clk_inv = _tx_clk_inv,                                                                                                    \
        },                                                                                                                           \
    },                                                                                                                               \
  }
#else
#define I2S_PDM_TX_CHAN_CFG(_sample_rate, _data_bit_width, _slot_mode)                                                               \
  {                                                                                                                                  \
    .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(_sample_rate), .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(_data_bit_width, _slot_mode), \
    .gpio_cfg = {                                                                                                                    \
      .clk = (gpio_num_t)_tx_clk,                                                                                                    \
      .dout = (gpio_num_t)_tx_dout0,                                                                                                 \
      .invert_flags =                                                                                                                \
        {                                                                                                                            \
          .clk_inv = _tx_clk_inv,                                                                                                    \
        },                                                                                                                           \
    },                                                                                                                               \
  }
#endif
#endif

#if SOC_I2S_SUPPORTS_PDM_RX
#if (SOC_I2S_PDM_MAX_RX_LINES > 1)
#define I2S_PDM_RX_CHAN_CFG(_sample_rate, _data_bit_width, _slot_mode)                                                               \
  {                                                                                                                                  \
    .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(_sample_rate), .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(_data_bit_width, _slot_mode), \
    .gpio_cfg = {                                                                                                                    \
      .clk = (gpio_num_t)_rx_clk,                                                                                                    \
      .dins =                                                                                                                        \
        {                                                                                                                            \
          (gpio_num_t)_rx_din0,                                                                                                      \
          (gpio_num_t)_rx_din1,                                                                                                      \
          (gpio_num_t)_rx_din2,                                                                                                      \
          (gpio_num_t)_rx_din3,                                                                                                      \
        },                                                                                                                           \
      .invert_flags =                                                                                                                \
        {                                                                                                                            \
          .clk_inv = _rx_clk_inv,                                                                                                    \
        },                                                                                                                           \
    },                                                                                                                               \
  }
#else
#define I2S_PDM_RX_CHAN_CFG(_sample_rate, _data_bit_width, _slot_mode)                                                               \
  {                                                                                                                                  \
    .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(_sample_rate), .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(_data_bit_width, _slot_mode), \
    .gpio_cfg = {                                                                                                                    \
      .clk = (gpio_num_t)_rx_clk,                                                                                                    \
      .din = (gpio_num_t)_rx_din0,                                                                                                   \
      .invert_flags =                                                                                                                \
        {                                                                                                                            \
          .clk_inv = _rx_clk_inv,                                                                                                    \
        },                                                                                                                           \
    },                                                                                                                               \
  }
#endif
#endif

#define I2S_ERROR_CHECK_RETURN(x, r)                   \
  do {                                                 \
    last_error = (x);                                  \
    if (unlikely(last_error != ESP_OK)) {              \
      log_e("ERROR: %s", esp_err_to_name(last_error)); \
      return (r);                                      \
    }                                                  \
  } while (0)
#define I2S_ERROR_CHECK_RETURN_FALSE(x) I2S_ERROR_CHECK_RETURN(x, false)

// Default read, no resmpling and temp buffer necessary
static esp_err_t i2s_channel_read_default(i2s_chan_handle_t handle, char *tmp_buf, void *dst, size_t len, size_t *bytes_read, uint32_t timeout_ms) {
  return i2s_channel_read(handle, (char *)dst, len, bytes_read, timeout_ms);
}

// Resample the 32bit SPH0645 microphone data into 16bit. SPH0645 is actually 18 bit, but this trick helps save some space
static esp_err_t i2s_channel_read_32_to_16(i2s_chan_handle_t handle, char *read_buff, void *dst, size_t len, size_t *bytes_read, uint32_t timeout_ms) {
  size_t out_len = 0;
  size_t read_buff_len = len * 2;
  if (read_buff == NULL) {
    log_e("Temp buffer is NULL!");
    return ESP_FAIL;
  }
  esp_err_t err = i2s_channel_read(handle, read_buff, read_buff_len, &out_len, timeout_ms);
  if (err != ESP_OK) {
    *bytes_read = 0;
    return err;
  }
  out_len /= 4;
  uint16_t *ds = (uint16_t *)dst;
  uint32_t *src = (uint32_t *)read_buff;
  for (size_t i = 0; i < out_len; i++) {
    ds[i] = src[i] >> 16;
  }
  *bytes_read = out_len * 2;
  return ESP_OK;
}

// Resample the 16bit stereo microphone data into 16bit mono.
static esp_err_t i2s_channel_read_16_stereo_to_mono(i2s_chan_handle_t handle, char *read_buff, void *dst, size_t len, size_t *bytes_read, uint32_t timeout_ms) {
  size_t out_len = 0;
  size_t read_buff_len = len * 2;
  if (read_buff == NULL) {
    log_e("Temp buffer is NULL!");
    return ESP_FAIL;
  }
  esp_err_t err = i2s_channel_read(handle, read_buff, read_buff_len, &out_len, timeout_ms);
  if (err != ESP_OK) {
    *bytes_read = 0;
    return err;
  }
  out_len /= 2;
  uint16_t *ds = (uint16_t *)dst;
  uint16_t *src = (uint16_t *)read_buff;
  for (size_t i = 0; i < out_len; i += 2) {
    *ds++ = src[i];
  }
  *bytes_read = out_len;
  return ESP_OK;
}

I2SClass::I2SClass() {
  last_error = ESP_OK;

  tx_chan = NULL;
  tx_sample_rate = 0;
  tx_data_bit_width = I2S_DATA_BIT_WIDTH_16BIT;
  tx_slot_mode = I2S_SLOT_MODE_STEREO;
  _tx_chan_enabled = false;
  tx_configure_callback = NULL;
  tx_configure_context = NULL;

  rx_fn = i2s_channel_read_default;
  rx_transform = I2S_RX_TRANSFORM_NONE;
  rx_transform_buf = NULL;
  rx_transform_buf_len = 0;

  rx_chan = NULL;
  rx_sample_rate = 0;
  rx_data_bit_width = I2S_DATA_BIT_WIDTH_16BIT;
  rx_slot_mode = I2S_SLOT_MODE_STEREO;

  _mclk = -1;
  _bclk = -1;
  _ws = -1;
  _dout = -1;
  _din = -1;
  _mclk_inv = false;
  _bclk_inv = false;
  _ws_inv = false;

#if SOC_I2S_SUPPORTS_PDM_TX
  _tx_clk = -1;
  _tx_dout0 = -1;
  _tx_dout1 = -1;
  _tx_clk_inv = false;
#endif
#if SOC_I2S_SUPPORTS_PDM_RX
  _rx_clk = -1;
  _rx_din0 = -1;
  _rx_din1 = -1;
  _rx_din2 = -1;
  _rx_din3 = -1;
  _rx_clk_inv = false;
#endif
}

I2SClass::~I2SClass() {
  end();
}

bool I2SClass::i2sDetachBus(void *bus_pointer) {
  I2SClass *bus = (I2SClass *)bus_pointer;
  if (bus->tx_chan != NULL || bus->rx_chan != NULL) {
    return bus->end();
  }
  return true;
}

// Set pins for STD and TDM mode
void I2SClass::setPins(int8_t bclk, int8_t ws, int8_t dout, int8_t din, int8_t mclk) {
  _mclk = digitalPinToGPIONumber(mclk);
  _bclk = digitalPinToGPIONumber(bclk);
  _ws = digitalPinToGPIONumber(ws);
  _dout = digitalPinToGPIONumber(dout);
  _din = digitalPinToGPIONumber(din);
}

void I2SClass::setInverted(bool bclk, bool ws, bool mclk) {
  _mclk_inv = mclk;
  _bclk_inv = bclk;
  _ws_inv = ws;
}

// Set pins for PDM TX mode
#if SOC_I2S_SUPPORTS_PDM_TX
void I2SClass::setPinsPdmTx(int8_t clk, int8_t dout0, int8_t dout1) {
  _tx_clk = digitalPinToGPIONumber(clk);
  _tx_dout0 = digitalPinToGPIONumber(dout0);
#if (SOC_I2S_PDM_MAX_TX_LINES > 1)
  _tx_dout1 = digitalPinToGPIONumber(dout1);
#endif
}
#endif

// Set pins for PDM RX mode
#if SOC_I2S_SUPPORTS_PDM_RX
void I2SClass::setPinsPdmRx(int8_t clk, int8_t din0, int8_t din1, int8_t din2, int8_t din3) {
  _rx_clk = digitalPinToGPIONumber(clk);
  _rx_din0 = digitalPinToGPIONumber(din0);
#if (SOC_I2S_PDM_MAX_RX_LINES > 1)
  _rx_din1 = digitalPinToGPIONumber(din1);
  _rx_din2 = digitalPinToGPIONumber(din2);
  _rx_din3 = digitalPinToGPIONumber(din3);
#endif
}
#endif

#if SOC_I2S_SUPPORTS_PDM_TX || SOC_I2S_SUPPORTS_PDM_RX
void I2SClass::setInvertedPdm(bool clk) {
#if SOC_I2S_SUPPORTS_PDM_TX
  _tx_clk_inv = clk;
#endif
#if SOC_I2S_SUPPORTS_PDM_RX
  _rx_clk_inv = clk;
#endif
}
#endif

bool I2SClass::initSTD(uint32_t rate, i2s_data_bit_width_t bits_cfg, i2s_slot_mode_t ch, int8_t slot_mask) {
  // Peripheral manager deinit previous peripheral if pin was used
  if (_mclk >= 0) {
    if (!perimanClearPinBus(_mclk)) {
      return false;
    }
  }
  if (_bclk >= 0) {
    if (!perimanClearPinBus(_bclk)) {
      return false;
    }
  }
  if (_ws >= 0) {
    if (!perimanClearPinBus(_ws)) {
      return false;
    }
  }
  if (_dout >= 0) {
    if (!perimanClearPinBus(_dout)) {
      return false;
    }
  }
  if (_din >= 0) {
    if (!perimanClearPinBus(_din)) {
      return false;
    }
  }

  // Set peripheral manager detach function for I2S
  if (_mclk >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_STD_MCLK, I2SClass::i2sDetachBus);
  }
  if (_bclk >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_STD_BCLK, I2SClass::i2sDetachBus);
  }
  if (_ws >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_STD_WS, I2SClass::i2sDetachBus);
  }
  if (_dout >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_STD_DOUT, I2SClass::i2sDetachBus);
  }
  if (_din >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_STD_DIN, I2SClass::i2sDetachBus);
  }

  // I2S configuration
  i2s_chan_config_t chan_cfg = I2S_DEFAULT_CFG();
  if (_dout >= 0 && _din >= 0) {
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));
  } else if (_dout >= 0) {
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_new_channel(&chan_cfg, &tx_chan, NULL));
  } else if (_din >= 0) {
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_new_channel(&chan_cfg, NULL, &rx_chan));
  }

  i2s_std_config_t i2s_config = I2S_STD_CHAN_CFG(rate, bits_cfg, ch);
  if (slot_mask >= 0 && (i2s_std_slot_mask_t)slot_mask <= I2S_STD_SLOT_BOTH) {
    i2s_config.slot_cfg.slot_mask = (i2s_std_slot_mask_t)slot_mask;
  }
  if (tx_chan != NULL) {
    _tx_chan_enabled = false;
    tx_sample_rate = rate;
    tx_data_bit_width = bits_cfg;
    tx_slot_mode = ch;
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_init_std_mode(tx_chan, &i2s_config));
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_enable(tx_chan));
    _tx_chan_enabled = true;
  }
  if (rx_chan != NULL) {
    rx_sample_rate = rate;
    rx_data_bit_width = bits_cfg;
    rx_slot_mode = ch;
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_init_std_mode(rx_chan, &i2s_config));
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_enable(rx_chan));
  }

  // Peripheral manager set bus type to I2S
  if (_mclk >= 0) {
    if (!perimanSetPinBus(_mclk, ESP32_BUS_TYPE_I2S_STD_MCLK, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_bclk >= 0) {
    if (!perimanSetPinBus(_bclk, ESP32_BUS_TYPE_I2S_STD_BCLK, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_ws >= 0) {
    if (!perimanSetPinBus(_ws, ESP32_BUS_TYPE_I2S_STD_WS, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_dout >= 0) {
    if (!perimanSetPinBus(_dout, ESP32_BUS_TYPE_I2S_STD_DOUT, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_din >= 0) {
    if (!perimanSetPinBus(_din, ESP32_BUS_TYPE_I2S_STD_DIN, (void *)(this), -1, -1)) {
      goto err;
    }
  }

  return true;
err:
  log_e("Failed to set all pins bus to I2S_STD");
  I2SClass::i2sDetachBus((void *)(this));
  return false;
}

#if SOC_I2S_SUPPORTS_TDM
bool I2SClass::initTDM(uint32_t rate, i2s_data_bit_width_t bits_cfg, i2s_slot_mode_t ch, int8_t slot_mask) {
  // Peripheral manager deinit previous peripheral if pin was used
  if (_mclk >= 0) {
    if (!perimanClearPinBus(_mclk)) {
      return false;
    }
  }
  if (_bclk >= 0) {
    if (!perimanClearPinBus(_bclk)) {
      return false;
    }
  }
  if (_ws >= 0) {
    if (!perimanClearPinBus(_ws)) {
      return false;
    }
  }
  if (_dout >= 0) {
    if (!perimanClearPinBus(_dout)) {
      return false;
    }
  }
  if (_din >= 0) {
    if (!perimanClearPinBus(_din)) {
      return false;
    }
  }

  // Set peripheral manager detach function for I2S
  if (_mclk >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_TDM_MCLK, I2SClass::i2sDetachBus);
  }
  if (_bclk >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_TDM_BCLK, I2SClass::i2sDetachBus);
  }
  if (_ws >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_TDM_WS, I2SClass::i2sDetachBus);
  }
  if (_dout >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_TDM_DOUT, I2SClass::i2sDetachBus);
  }
  if (_din >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_TDM_DIN, I2SClass::i2sDetachBus);
  }

  // I2S configuration
  i2s_chan_config_t chan_cfg = I2S_DEFAULT_CFG();
  if (_dout >= 0 && _din >= 0) {
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));
  } else if (_dout >= 0) {
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_new_channel(&chan_cfg, &tx_chan, NULL));
  } else if (_din >= 0) {
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_new_channel(&chan_cfg, NULL, &rx_chan));
  }

  i2s_tdm_config_t i2s_tdm_config = I2S_TDM_CHAN_CFG(rate, bits_cfg, ch, (i2s_tdm_slot_mask_t)slot_mask);
  if (tx_chan != NULL) {
    _tx_chan_enabled = false;
    tx_sample_rate = rate;
    tx_data_bit_width = bits_cfg;
    tx_slot_mode = ch;
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_init_tdm_mode(tx_chan, &i2s_tdm_config));
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_enable(tx_chan));
    _tx_chan_enabled = true;
  }
  if (rx_chan != NULL) {
    rx_sample_rate = rate;
    rx_data_bit_width = bits_cfg;
    rx_slot_mode = ch;
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_init_tdm_mode(rx_chan, &i2s_tdm_config));
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_enable(rx_chan));
  }

  // Peripheral manager set bus type to I2S
  if (_mclk >= 0) {
    if (!perimanSetPinBus(_mclk, ESP32_BUS_TYPE_I2S_TDM_MCLK, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_bclk >= 0) {
    if (!perimanSetPinBus(_bclk, ESP32_BUS_TYPE_I2S_TDM_BCLK, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_ws >= 0) {
    if (!perimanSetPinBus(_ws, ESP32_BUS_TYPE_I2S_TDM_WS, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_dout >= 0) {
    if (!perimanSetPinBus(_dout, ESP32_BUS_TYPE_I2S_TDM_DOUT, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_din >= 0) {
    if (!perimanSetPinBus(_din, ESP32_BUS_TYPE_I2S_TDM_DIN, (void *)(this), -1, -1)) {
      goto err;
    }
  }

  return true;
err:
  log_e("Failed to set all pins bus to I2S_TDM");
  I2SClass::i2sDetachBus((void *)(this));
  return false;
}
#endif

#if SOC_I2S_SUPPORTS_PDM_TX
bool I2SClass::initPDMtx(uint32_t rate, i2s_data_bit_width_t bits_cfg, i2s_slot_mode_t ch) {
  // Peripheral manager deinit previous peripheral if pin was used
  if (_tx_clk >= 0) {
    if (!perimanClearPinBus(_tx_clk)) {
      return false;
    }
  }
  if (_tx_dout0 >= 0) {
    if (!perimanClearPinBus(_tx_dout0)) {
      return false;
    }
  }
  if (_tx_dout1 >= 0) {
    if (!perimanClearPinBus(_tx_dout1)) {
      return false;
    }
  }

  // Set peripheral manager detach function for I2S
  if (_tx_clk >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_PDM_TX_CLK, I2SClass::i2sDetachBus);
  }
  if (_tx_dout0 >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_PDM_TX_DOUT0, I2SClass::i2sDetachBus);
  }
  if (_tx_dout1 >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_PDM_TX_DOUT1, I2SClass::i2sDetachBus);
  }

  // I2S configuration
  i2s_chan_config_t chan_cfg = I2S_DEFAULT_CFG();
  I2S_ERROR_CHECK_RETURN_FALSE(i2s_new_channel(&chan_cfg, &tx_chan, NULL));

  i2s_pdm_tx_config_t i2s_pdm_tx_config = I2S_PDM_TX_CHAN_CFG(rate, bits_cfg, ch);
  if (tx_chan != NULL) {
    _tx_chan_enabled = false;
    tx_sample_rate = rate;
    tx_data_bit_width = bits_cfg;
    tx_slot_mode = ch;
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_init_pdm_tx_mode(tx_chan, &i2s_pdm_tx_config));
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_enable(tx_chan));
    _tx_chan_enabled = true;
  }

  // Peripheral manager set bus type to I2S
  if (_tx_clk >= 0) {
    if (!perimanSetPinBus(_tx_clk, ESP32_BUS_TYPE_I2S_PDM_TX_CLK, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_tx_dout0 >= 0) {
    if (!perimanSetPinBus(_tx_dout0, ESP32_BUS_TYPE_I2S_PDM_TX_DOUT0, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_tx_dout1 >= 0) {
    if (!perimanSetPinBus(_tx_dout1, ESP32_BUS_TYPE_I2S_PDM_TX_DOUT1, (void *)(this), -1, -1)) {
      goto err;
    }
  }

  return true;
err:
  log_e("Failed to set all pins bus to I2S_TDM");
  I2SClass::i2sDetachBus((void *)(this));
  return false;
}
#endif

#if SOC_I2S_SUPPORTS_PDM_RX
bool I2SClass::initPDMrx(uint32_t rate, i2s_data_bit_width_t bits_cfg, i2s_slot_mode_t ch) {
  // Peripheral manager deinit previous peripheral if pin was used
  if (_rx_clk >= 0) {
    if (!perimanClearPinBus(_rx_clk)) {
      return false;
    }
  }
  if (_rx_din0 >= 0) {
    if (!perimanClearPinBus(_rx_din0)) {
      return false;
    }
  }
  if (_rx_din1 >= 0) {
    if (!perimanClearPinBus(_rx_din1)) {
      return false;
    }
  }
  if (_rx_din2 >= 0) {
    if (!perimanClearPinBus(_rx_din2)) {
      return false;
    }
  }
  if (_rx_din3 >= 0) {
    if (!perimanClearPinBus(_rx_din3)) {
      return false;
    }
  }

  // Set peripheral manager detach function for I2S
  if (_rx_clk >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_PDM_RX_CLK, I2SClass::i2sDetachBus);
  }
  if (_rx_din0 >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_PDM_RX_DIN0, I2SClass::i2sDetachBus);
  }
  if (_rx_din1 >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_PDM_RX_DIN1, I2SClass::i2sDetachBus);
  }
  if (_rx_din2 >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_PDM_RX_DIN2, I2SClass::i2sDetachBus);
  }
  if (_rx_din3 >= 0) {
    perimanSetBusDeinit(ESP32_BUS_TYPE_I2S_PDM_RX_DIN3, I2SClass::i2sDetachBus);
  }

  // I2S configuration
  i2s_chan_config_t chan_cfg = I2S_DEFAULT_CFG();
  I2S_ERROR_CHECK_RETURN_FALSE(i2s_new_channel(&chan_cfg, NULL, &rx_chan));

  i2s_pdm_rx_config_t i2s_pdf_rx_config = I2S_PDM_RX_CHAN_CFG(rate, bits_cfg, ch);
  if (rx_chan != NULL) {
    rx_sample_rate = rate;
    rx_data_bit_width = bits_cfg;
    rx_slot_mode = ch;
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_init_pdm_rx_mode(rx_chan, &i2s_pdf_rx_config));
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_enable(rx_chan));
  }

  // Peripheral manager set bus type to I2S
  if (_rx_clk >= 0) {
    if (!perimanSetPinBus(_rx_clk, ESP32_BUS_TYPE_I2S_PDM_RX_CLK, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_rx_din0 >= 0) {
    if (!perimanSetPinBus(_rx_din0, ESP32_BUS_TYPE_I2S_PDM_RX_DIN0, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_rx_din1 >= 0) {
    if (!perimanSetPinBus(_rx_din1, ESP32_BUS_TYPE_I2S_PDM_RX_DIN1, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_rx_din2 >= 0) {
    if (!perimanSetPinBus(_rx_din2, ESP32_BUS_TYPE_I2S_PDM_RX_DIN2, (void *)(this), -1, -1)) {
      goto err;
    }
  }
  if (_rx_din3 >= 0) {
    if (!perimanSetPinBus(_rx_din3, ESP32_BUS_TYPE_I2S_PDM_RX_DIN3, (void *)(this), -1, -1)) {
      goto err;
    }
  }

  return true;
err:
  log_e("Failed to set all pins bus to I2S_TDM");
  I2SClass::i2sDetachBus((void *)(this));
  return false;
}
#endif

bool I2SClass::begin(i2s_mode_t mode, uint32_t rate, i2s_data_bit_width_t bits_cfg, i2s_slot_mode_t ch, int8_t slot_mask) {
  /* Setup I2S peripheral */
  if (mode >= I2S_MODE_MAX) {
    log_e("Invalid I2S mode selected.");
    return false;
  }
  _mode = mode;

  bool init = false;
  switch (_mode) {
    case I2S_MODE_STD: init = initSTD(rate, bits_cfg, ch, slot_mask); break;
#if SOC_I2S_SUPPORTS_TDM
    case I2S_MODE_TDM: init = initTDM(rate, bits_cfg, ch, slot_mask); break;
#endif
#if SOC_I2S_SUPPORTS_PDM_TX
    case I2S_MODE_PDM_TX: init = initPDMtx(rate, bits_cfg, ch); break;
#endif
#if SOC_I2S_SUPPORTS_PDM_RX
    case I2S_MODE_PDM_RX: init = initPDMrx(rate, bits_cfg, ch); break;
#endif
    default: break;
  }

  if (init == false) {
    log_e("I2S initialization failed.");
    return false;
  }
  return true;
}

bool I2SClass::end() {
  bool success = true;
  if (tx_chan != NULL) {
    last_error = i2s_channel_disable(tx_chan);
    if (last_error != ESP_OK && last_error != ESP_ERR_INVALID_STATE) {
      log_e("ERROR: %s", esp_err_to_name(last_error));
      success = false;
    } else {
      _tx_chan_enabled = false;
    }

    last_error = i2s_del_channel(tx_chan);
    if (last_error != ESP_OK) {
      log_e("ERROR: %s", esp_err_to_name(last_error));
      success = false;
    } else {
      tx_chan = NULL;
      _tx_chan_enabled = false;
      tx_sample_rate = 0;
    }
  }
  if (rx_chan != NULL) {
    last_error = i2s_channel_disable(rx_chan);
    if (last_error != ESP_OK && last_error != ESP_ERR_INVALID_STATE) {
      log_e("ERROR: %s", esp_err_to_name(last_error));
      success = false;
    }

    last_error = i2s_del_channel(rx_chan);
    if (last_error != ESP_OK) {
      log_e("ERROR: %s", esp_err_to_name(last_error));
      success = false;
    } else {
      rx_chan = NULL;
    }
  }
  if (tx_chan != NULL || rx_chan != NULL) {
    return false;
  }
  if (rx_transform_buf != NULL) {
    free(rx_transform_buf);
    rx_transform_buf = NULL;
    rx_transform_buf_len = 0;
  }

  //Peripheral manager deinit used pins
  switch (_mode) {
    case I2S_MODE_STD:
#if SOC_I2S_SUPPORTS_TDM
    case I2S_MODE_TDM:
#endif
      perimanClearPinBus(_mclk);
      perimanClearPinBus(_bclk);
      perimanClearPinBus(_ws);
      if (_dout >= 0) {
        perimanClearPinBus(_dout);
      }
      if (_din >= 0) {
        perimanClearPinBus(_din);
      }
      break;
#if SOC_I2S_SUPPORTS_PDM_TX
    case I2S_MODE_PDM_TX:
      perimanClearPinBus(_tx_clk);
      if (_tx_dout0 >= 0) {
        perimanClearPinBus(_tx_dout0);
      }
      if (_tx_dout1 >= 0) {
        perimanClearPinBus(_tx_dout1);
      }
      break;
#endif
#if SOC_I2S_SUPPORTS_PDM_RX
    case I2S_MODE_PDM_RX:
      perimanClearPinBus(_rx_clk);
      if (_rx_din0 >= 0) {
        perimanClearPinBus(_rx_din0);
      }
      if (_rx_din1 >= 0) {
        perimanClearPinBus(_rx_din1);
      }
      if (_rx_din2 >= 0) {
        perimanClearPinBus(_rx_din2);
      }
      if (_rx_din3 >= 0) {
        perimanClearPinBus(_rx_din3);
      }
      break;
#endif
    default: break;
  }
  return success;
}

bool I2SClass::configureTX(uint32_t rate, i2s_data_bit_width_t bits_cfg, i2s_slot_mode_t ch, int8_t slot_mask) {
  /* Setup I2S channels */
  if (tx_chan != NULL) {
    if (_tx_chan_enabled && tx_sample_rate == rate && tx_data_bit_width == bits_cfg && tx_slot_mode == ch) {
      return true;
    }
    i2s_std_config_t i2s_config = I2S_STD_CHAN_CFG(rate, bits_cfg, ch);
    if (slot_mask >= 0 && (i2s_std_slot_mask_t)slot_mask <= I2S_STD_SLOT_BOTH) {
      i2s_config.slot_cfg.slot_mask = (i2s_std_slot_mask_t)slot_mask;
    }

    if (_tx_chan_enabled) {
      I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_disable(tx_chan));
      _tx_chan_enabled = false;
    }
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_reconfig_std_clock(tx_chan, &i2s_config.clk_cfg));
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_reconfig_std_slot(tx_chan, &i2s_config.slot_cfg));
    I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_enable(tx_chan));
    _tx_chan_enabled = true;
    tx_sample_rate = rate;
    tx_data_bit_width = bits_cfg;
    tx_slot_mode = ch;
    return true;
  }
  return false;
}

void I2SClass::setTxConfigureCallback(i2s_tx_configure_callback_t callback, void *context) {
  tx_configure_callback = callback;
  tx_configure_context = context;
}

bool I2SClass::configureTXForPlayback(uint32_t rate, i2s_data_bit_width_t bits_cfg, i2s_slot_mode_t ch) {
  if (tx_configure_callback != NULL) {
    return tx_configure_callback(rate, bits_cfg, ch, tx_configure_context);
  }
  return configureTX(rate, bits_cfg, ch);
}

bool I2SClass::configureRX(uint32_t rate, i2s_data_bit_width_t bits_cfg, i2s_slot_mode_t ch, i2s_rx_transform_t transform) {
  /* Setup I2S channels */
  if (rx_chan != NULL) {
    if (rx_sample_rate != rate || rx_data_bit_width != bits_cfg || rx_slot_mode != ch) {
      i2s_std_config_t i2s_config = I2S_STD_CHAN_CFG(rate, bits_cfg, ch);
      I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_disable(rx_chan));
      I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_reconfig_std_clock(rx_chan, &i2s_config.clk_cfg));
      rx_sample_rate = rate;
      I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_reconfig_std_slot(rx_chan, &i2s_config.slot_cfg));
      rx_data_bit_width = bits_cfg;
      rx_slot_mode = ch;
      I2S_ERROR_CHECK_RETURN_FALSE(i2s_channel_enable(rx_chan));
      return transformRX(transform);
    }
    if (rx_transform != transform) {
      return transformRX(transform);
    }
    return true;
  }
  return false;
}

size_t I2SClass::readBytes(char *buffer, size_t size) {
  size_t bytes_read = 0;
  size_t total_size = 0;
  last_error = ESP_FAIL;
  if (rx_chan == NULL) {
    return total_size;
  }
  while (total_size < size) {
    bytes_read = size - total_size;
    if (rx_transform_buf != NULL && bytes_read > I2S_READ_CHUNK_SIZE) {
      bytes_read = I2S_READ_CHUNK_SIZE;
    }
    I2S_ERROR_CHECK_RETURN(rx_fn(rx_chan, rx_transform_buf, (char *)(buffer + total_size), bytes_read, &bytes_read, _timeout), 0);
    total_size += bytes_read;
  }
  return total_size;
}

size_t I2SClass::write(uint8_t *buffer, size_t size) {
  size_t written = 0;
  size_t bytes_sent = 0;
  last_error = ESP_FAIL;
  if (tx_chan == NULL) {
    return written;
  }
  while (written < size) {
    if (_stop.load()) {
      break;
    }
    bytes_sent = size - written;
    if (bytes_sent > 1024) {
      bytes_sent = 1024;
    }
    esp_err_t err = i2s_channel_write(tx_chan, (char *)(buffer + written), bytes_sent, &bytes_sent, _timeout);
    setWriteError(err);
    I2S_ERROR_CHECK_RETURN(err, written);
    written += bytes_sent;
  }
  return written;
}

size_t I2SClass::write(const uint8_t *buffer, size_t size) {
  return write((uint8_t *)buffer, size);
}

i2s_chan_handle_t I2SClass::txChan() {
  return tx_chan;
}
uint32_t I2SClass::txSampleRate() {
  return tx_sample_rate;
}
i2s_data_bit_width_t I2SClass::txDataWidth() {
  return tx_data_bit_width;
}
i2s_slot_mode_t I2SClass::txSlotMode() {
  return tx_slot_mode;
}

i2s_chan_handle_t I2SClass::rxChan() {
  return rx_chan;
}
uint32_t I2SClass::rxSampleRate() {
  return rx_sample_rate;
}
i2s_data_bit_width_t I2SClass::rxDataWidth() {
  return rx_data_bit_width;
}
i2s_slot_mode_t I2SClass::rxSlotMode() {
  return rx_slot_mode;
}

int I2SClass::lastError() {
  return (int)last_error;
}

int I2SClass::available() {
  if (rx_chan == NULL) {
    return -1;
  }
  return I2S_READ_CHUNK_SIZE;  // / (rx_data_bit_width/8);
};

int I2SClass::peek() {
  return -1;
};

int I2SClass::read() {
  int out = 0;
  if (readBytes((char *)&out, rx_data_bit_width / 8) == (rx_data_bit_width / 8)) {
    return out;
  }
  return -1;
};

size_t I2SClass::write(uint8_t d) {
  return write(&d, 1);
}

bool I2SClass::transformRX(i2s_rx_transform_t transform) {
  switch (transform) {
    case I2S_RX_TRANSFORM_NONE:
      allocTranformRX(0);
      rx_fn = i2s_channel_read_default;
      break;

    case I2S_RX_TRANSFORM_32_TO_16:
      if (rx_data_bit_width != I2S_DATA_BIT_WIDTH_32BIT) {
        log_e("Wrong data width. Should be 32bit");
        return false;
      }
      if (!allocTranformRX(I2S_READ_CHUNK_SIZE * 2)) {
        return false;
      }
      rx_fn = i2s_channel_read_32_to_16;
      rx_data_bit_width = I2S_DATA_BIT_WIDTH_16BIT;
      break;

    case I2S_RX_TRANSFORM_16_STEREO_TO_MONO:
      if (rx_slot_mode != I2S_SLOT_MODE_STEREO) {
        log_e("Wrong slot mode. Should be Stereo");
        return false;
      }
      if (!allocTranformRX(I2S_READ_CHUNK_SIZE * 2)) {
        return false;
      }
      rx_fn = i2s_channel_read_16_stereo_to_mono;
      rx_slot_mode = I2S_SLOT_MODE_MONO;
      break;

    default: log_e("Unknown RX Transform %d", transform); return false;
  }
  rx_transform = transform;
  return true;
}

bool I2SClass::allocTranformRX(size_t buf_len) {
  char *buf = NULL;
  if (buf_len == 0) {
    if (rx_transform_buf != NULL) {
      free(rx_transform_buf);
      rx_transform_buf = NULL;
      rx_transform_buf_len = 0;
    }
    return true;
  }
  if (rx_transform_buf == NULL || rx_transform_buf_len != buf_len) {
    buf = (char *)malloc(buf_len);
    if (buf == NULL) {
      log_e("malloc %u failed!", buf_len);
      return false;
    }
    if (rx_transform_buf != NULL) {
      free(rx_transform_buf);
    }
    rx_transform_buf = buf;
    rx_transform_buf_len = buf_len;
  }
  return true;
}

const int WAVE_HEADER_SIZE = PCM_WAV_HEADER_SIZE;

static uint16_t readWavLE16(const uint8_t *data) {
  return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t readWavLE32(const uint8_t *data) {
  return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

//Record PCM WAV with current RX settings
uint8_t *I2SClass::recordWAV(size_t rec_seconds, size_t *out_size) {
  uint32_t sample_rate = rxSampleRate();
  uint16_t sample_width = (uint16_t)rxDataWidth();
  uint16_t num_channels = (uint16_t)rxSlotMode();
  size_t rec_size = rec_seconds * ((sample_rate * (sample_width / 8)) * num_channels);
  const pcm_wav_header_t wav_header = PCM_WAV_HEADER_DEFAULT(rec_size, sample_width, sample_rate, num_channels);
  *out_size = 0;

  log_d("Record WAV: rate:%lu, bits:%u, channels:%u, size:%lu", sample_rate, sample_width, num_channels, rec_size);

  uint8_t *wav_buf = (uint8_t *)malloc(rec_size + WAVE_HEADER_SIZE);
  if (wav_buf == NULL) {
    log_e("Failed to allocate WAV buffer with size %u", rec_size + WAVE_HEADER_SIZE);
    return NULL;
  }
  memcpy(wav_buf, &wav_header, WAVE_HEADER_SIZE);
  size_t wav_size = readBytes((char *)(wav_buf + WAVE_HEADER_SIZE), rec_size);
  if (wav_size < rec_size) {
    log_e("Recorded %u bytes from %u", wav_size, rec_size);
  } else if (lastError()) {
    log_e("Read Failed! %d", lastError());
  } else {
    *out_size = rec_size + WAVE_HEADER_SIZE;
    return wav_buf;
  }
  free(wav_buf);
  return NULL;
}

bool I2SClass::playWAV(const uint8_t *data, size_t len) {
  static const size_t descriptor_size = 12;
  static const size_t chunk_header_size = 8;
  static const size_t pcm_fmt_size = 16;

  if (data == NULL || len < descriptor_size) {
    log_e("Invalid WAV buffer");
    return false;
  }
  if (memcmp(data, "RIFF", 4) != 0 || memcmp(data + 8, "WAVE", 4) != 0) {
    log_e("Invalid WAV descriptor");
    return false;
  }

  uint32_t riff_size = readWavLE32(data + 4);
  if (riff_size < 4 || riff_size > len - 8) {
    log_e("Invalid WAV RIFF size");
    return false;
  }

  size_t riff_end = (size_t)riff_size + 8;
  size_t chunk_offset = descriptor_size;
  bool fmt_found = false;
  bool data_found = false;
  uint16_t audio_format = 0;
  uint16_t num_channels = 0;
  uint16_t bits_per_sample = 0;
  uint32_t sample_rate = 0;
  const uint8_t *audio_data = NULL;
  size_t audio_size = 0;

  while (chunk_offset < riff_end) {
    if (riff_end - chunk_offset < chunk_header_size) {
      log_e("Truncated WAV chunk header");
      return false;
    }

    const uint8_t *chunk = data + chunk_offset;
    uint32_t chunk_size = readWavLE32(chunk + 4);
    size_t payload_offset = chunk_offset + chunk_header_size;
    if (chunk_size > riff_end - payload_offset) {
      log_e("Truncated WAV chunk");
      return false;
    }

    if (memcmp(chunk, "fmt ", 4) == 0 && !fmt_found) {
      if (chunk_size < pcm_fmt_size) {
        log_e("Invalid WAV fmt chunk");
        return false;
      }
      const uint8_t *fmt = data + payload_offset;
      audio_format = readWavLE16(fmt);
      num_channels = readWavLE16(fmt + 2);
      sample_rate = readWavLE32(fmt + 4);
      bits_per_sample = readWavLE16(fmt + 14);
      fmt_found = true;
    } else if (memcmp(chunk, "data", 4) == 0 && !data_found) {
      audio_data = data + payload_offset;
      audio_size = chunk_size;
      data_found = true;
    } else {
      log_d("Skip chunk: %c%c%c%c, len: %lu", chunk[0], chunk[1], chunk[2], chunk[3], (unsigned long)chunk_size + chunk_header_size);
    }

    size_t next_offset = payload_offset + chunk_size;
    if ((chunk_size & 1U) != 0) {
      if (next_offset >= riff_end) {
        log_e("Missing WAV chunk padding");
        return false;
      }
      ++next_offset;
    }
    chunk_offset = next_offset;
  }

  if (!fmt_found || !data_found) {
    log_e("WAV fmt or data chunk is missing");
    return false;
  }
  if (audio_format != WAVE_FORMAT_PCM) {
    log_e("Audio format is not PCM!");
    return false;
  }
  if (sample_rate == 0 || (num_channels != 1 && num_channels != 2) ||
      (bits_per_sample != 8 && bits_per_sample != 16 && bits_per_sample != 24 && bits_per_sample != 32)) {
    log_e("Unsupported WAV format");
    return false;
  }

  log_d("Play WAV: rate:%lu, bits:%u, channels:%u, size:%lu", sample_rate, bits_per_sample, num_channels, (unsigned long)audio_size);
  if (!configureTXForPlayback(sample_rate, (i2s_data_bit_width_t)bits_per_sample, (i2s_slot_mode_t)num_channels)) {
    log_e("Failed to configure I2S for WAV playback");
    return false;
  }
  return write(audio_data, audio_size) == audio_size;
}

bool I2SClass::playMP3(uint8_t *src, size_t src_len) {
  int16_t outBuf[MAX_NCHAN * MAX_NGRAN * MAX_NSAMP];
  uint8_t *readPtr = NULL;
  int bytesAvailable = 0, err = 0, offset = 0;
  MP3FrameInfo frameInfo;
  HMP3Decoder decoder = NULL;

  if (src == NULL || src_len == 0 || src_len > INT_MAX) {
    log_e("Invalid MP3 buffer");
    return false;
  }

  bytesAvailable = (int)src_len;
  readPtr = src;

  decoder = MP3InitDecoder();
  if (decoder == NULL) {
    log_e("Could not allocate decoder");
    return false;
  }
  do {
    if (_stop.load()) {
      break;
    }

    offset = MP3FindSyncWord(readPtr, bytesAvailable);
    if (offset < 0) {
      break;
    }

    readPtr += offset;
    bytesAvailable -= offset;
    err = MP3Decode(decoder, &readPtr, &bytesAvailable, outBuf, 0);
    if (err) {
      log_e("Decode ERROR: %d", err);
      MP3FreeDecoder(decoder);
      return false;
    } else {
      MP3GetLastFrameInfo(decoder, &frameInfo);
      if (!configureTXForPlayback(frameInfo.samprate, (i2s_data_bit_width_t)frameInfo.bitsPerSample, (i2s_slot_mode_t)frameInfo.nChans)) {
        log_e("Failed to configure I2S for MP3 playback");
        MP3FreeDecoder(decoder);
        return false;
      }
      size_t output_size = (size_t)((frameInfo.bitsPerSample / 8) * frameInfo.outputSamps);
      if (write((uint8_t *)outBuf, output_size) != output_size) {
        MP3FreeDecoder(decoder);
        return false;
      }
    }
  } while (true);

  MP3FreeDecoder(decoder);
  return true;
}

void I2SClass::stop() {
  _stop.store(true);
}

void I2SClass::clearStop() {
  _stop.store(false);
}

#endif /* SOC_I2S_SUPPORTED */
