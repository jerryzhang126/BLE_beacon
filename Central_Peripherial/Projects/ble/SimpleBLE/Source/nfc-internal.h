/**
 * @file nfc-internal.h
 * @brief Internal defines and macros
 */

#ifndef __NFC_INTERNAL_H__
#define __NFC_INTERNAL_H__

#include <stdbool.h>
#include "nfc.h"

/**
 * @macro HAL
 * @brief Execute corresponding driver function if exists.
 */
#define HAL( FUNCTION, ... ) pnd->last_error = 0; \
  if (pnd->driver->FUNCTION) { \
    return pnd->driver->FUNCTION( __VA_ARGS__ ); \
  } else { \
    pnd->last_error = NFC_EDEVNOTSUPP; \
    return false; \
  }

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

/*
 * Buffer management macros.
 *
 * The following macros ease setting-up and using buffers:
 * BUFFER_INIT (data, 5);      // data -> [ xx, xx, xx, xx, xx ]
 * BUFFER_SIZE (data);         // size -> 0
 * BUFFER_APPEND (data, 0x12); // data -> [ 12, xx, xx, xx, xx ]
 * BUFFER_SIZE (data);         // size -> 1
 * uint16 x = 0x3456;        // We suppose we are little endian
 * BUFFER_APPEND_BYTES (data, x, 2);
 *                             // data -> [ 12, 56, 34, xx, xx ]
 * BUFFER_SIZE (data);         // size -> 3
 * BUFFER_APPEND_LE (data, x, 2, sizeof (x));
 *                             // data -> [ 12, 56, 34, 34, 56 ]
 * BUFFER_SIZE (data);         // size -> 5
 */

/*
 * Initialise a buffer named buffer_name of size bytes.
 */
#define BUFFER_INIT(buffer_name, size) \
  uint8 buffer_name[size]; \
  size_t __##buffer_name##_n = 0

/*
 * Create a wrapper for an existing buffer.
 * BEWARE!  It eats children!
 */
#define BUFFER_ALIAS(buffer_name, origin) \
  uint8 *buffer_name = (void *)origin; \
  size_t __##buffer_name##_n = 0;

#define BUFFER_SIZE(buffer_name) (__##buffer_name##_n)

#define BUFFER_CLEAR(buffer_name) (__##buffer_name##_n = 0)
/*
 * Append one byte of data to the buffer buffer_name.
 */
#define BUFFER_APPEND(buffer_name, data) \
  do { \
    buffer_name[__##buffer_name##_n++] = data; \
  } while (0)

/*
 * Append size bytes of data to the buffer buffer_name.
 */
#define BUFFER_APPEND_BYTES(buffer_name, data, size) \
  do { \
    size_t __n = 0; \
    while (__n < size) { \
      buffer_name[__##buffer_name##_n++] = ((uint8 *)data)[__n++]; \
    } \
  } while (0)

typedef enum {
  NOT_INTRUSIVE,
  INTRUSIVE,
  NOT_AVAILABLE,
} scan_type_enum;

struct nfc_driver {
  const char *name;
  const scan_type_enum scan_type;
  size_t (*scan)(const nfc_context *context, nfc_connstring connstrings[], const size_t connstrings_len);
  struct nfc_device *(*open)(const nfc_context *context, const nfc_connstring connstring);
  void (*close)(struct nfc_device *pnd);
  const char *(*strerror)(const struct nfc_device *pnd);

  int (*initiator_init)(struct nfc_device *pnd);
  int (*initiator_init_secure_element)(struct nfc_device *pnd);
  int (*initiator_select_passive_target)(struct nfc_device *pnd,  const nfc_modulation nm, const uint8 *pbtInitData, const size_t szInitData, nfc_target *pnt);
  int (*initiator_poll_target)(struct nfc_device *pnd, const nfc_modulation *pnmModulations, const size_t szModulations, const uint8 uiPollNr, const uint8 btPeriod, nfc_target *pnt);
  int (*initiator_select_dep_target)(struct nfc_device *pnd, const nfc_dep_mode ndm, const nfc_baud_rate nbr, const nfc_dep_info *pndiInitiator, nfc_target *pnt, const int timeout);
  int (*initiator_deselect_target)(struct nfc_device *pnd);
  int (*initiator_transceive_bytes)(struct nfc_device *pnd, const uint8 *pbtTx, const size_t szTx, uint8 *pbtRx, const size_t szRx, int timeout);
  int (*initiator_transceive_bits)(struct nfc_device *pnd, const uint8 *pbtTx, const size_t szTxBits, const uint8 *pbtTxPar, uint8 *pbtRx, uint8 *pbtRxPar);
  int (*initiator_transceive_bytes_timed)(struct nfc_device *pnd, const uint8 *pbtTx, const size_t szTx, uint8 *pbtRx, const size_t szRx, uint32 *cycles);
  int (*initiator_transceive_bits_timed)(struct nfc_device *pnd, const uint8 *pbtTx, const size_t szTxBits, const uint8 *pbtTxPar, uint8 *pbtRx, uint8 *pbtRxPar, uint32 *cycles);
  int (*initiator_target_is_present)(struct nfc_device *pnd, const nfc_target *pnt);

  int (*target_init)(struct nfc_device *pnd, nfc_target *pnt, uint8 *pbtRx, const size_t szRx, int timeout);
  int (*target_send_bytes)(struct nfc_device *pnd, const uint8 *pbtTx, const size_t szTx, int timeout);
  int (*target_receive_bytes)(struct nfc_device *pnd, uint8 *pbtRx, const size_t szRxLen, int timeout);
  int (*target_send_bits)(struct nfc_device *pnd, const uint8 *pbtTx, const size_t szTxBits, const uint8 *pbtTxPar);
  int (*target_receive_bits)(struct nfc_device *pnd, uint8 *pbtRx, const size_t szRxLen, uint8 *pbtRxPar);

  int (*device_set_property_bool)(struct nfc_device *pnd, const nfc_property property, const bool bEnable);
  int (*device_set_property_int)(struct nfc_device *pnd, const nfc_property property, const int value);
  int (*get_supported_modulation)(struct nfc_device *pnd, const nfc_mode mode, const nfc_modulation_type **const supported_mt);
  int (*get_supported_baud_rate)(struct nfc_device *pnd, const nfc_modulation_type nmt, const nfc_baud_rate **const supported_br);
  int (*device_get_information_about)(struct nfc_device *pnd, char **buf);

  int (*abort_command)(struct nfc_device *pnd);
  int (*idle)(struct nfc_device *pnd);
  int (*powerdown)(struct nfc_device *pnd);
};

#  define DEVICE_NAME_LENGTH  256
#  define DEVICE_PORT_LENGTH  64

#define MAX_USER_DEFINED_DEVICES 4

struct nfc_user_defined_device {
  char name[DEVICE_NAME_LENGTH];
  nfc_connstring connstring;
  bool optional;
};

/**
 * @struct nfc_context
 * @brief NFC library context
 * Struct which contains internal options, references, pointers, etc. used by library
 */
struct nfc_context {
  bool allow_autoscan;
  bool allow_intrusive_scan;
  uint32  log_level;
  struct nfc_user_defined_device user_defined_devices[MAX_USER_DEFINED_DEVICES];
  unsigned int user_defined_device_count;
};

nfc_context *nfc_context_new(void);
void nfc_context_free(nfc_context *context);

/**
 * @struct nfc_device
 * @brief NFC device information
 */
struct nfc_device {
  const nfc_context *context;
  const struct nfc_driver *driver;
  void *driver_data;
  void *chip_data;

  /** Device name string, including device wrapper firmware */
  char    name[DEVICE_NAME_LENGTH];
  /** Device connection string */
  nfc_connstring connstring;
  /** Is the CRC automaticly added, checked and removed from the frames */
  bool    bCrc;
  /** Does the chip handle parity bits, all parities are handled as data */
  bool    bPar;
  /** Should the chip handle frames encapsulation and chaining */
  bool    bEasyFraming;
  /** Should the chip try forever on select? */
  bool    bInfiniteSelect;
  /** Should the chip switch automatically activate ISO14443-4 when
      selecting tags supporting it? */
  bool    bAutoIso14443_4;
  /** Supported modulation encoded in a byte */
  uint8  btSupportByte;
  /** Last reported error */
  int     last_error;
};

nfc_device *nfc_device_new(const nfc_context *context, const nfc_connstring connstring);
void        nfc_device_free(nfc_device *dev);

void string_as_boolean(const char *s, bool *value);

void iso14443_cascade_uid(const uint8 abtUID[], const size_t szUID, uint8 *pbtCascadedUID, size_t *pszCascadedUID);

void prepare_initiator_data(const nfc_modulation nm, uint8 **ppbtInitiatorData, size_t *pszInitiatorData);

int connstring_decode(const nfc_connstring connstring, const char *driver_name, const char *bus_name, char **pparam1, char **pparam2);

#endif // __NFC_INTERNAL_H__