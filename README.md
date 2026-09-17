# wb_idf_ezo_i2c

ESP-IDF component for Atlas Scientific EZO devices using their I2C protocol.

This project is not affiliated with, endorsed by, or sponsored by Atlas Scientific.
EZO™ is a trademark of Atlas Scientific LLC.

## Device support

The generic command and reading API provides defaults for every device below. Dedicated
command helpers currently exist for pH, EC, DO, and RTD. Hardware verification is still
required for each target and firmware revision.

| Device | Generic API | Dedicated helpers | Hardware tested |
| :--- | :---: | :---: | :---: |
| **EZO-pH** | Yes | Yes | No |
| **EZO-ORP** | Yes | No | No |
| **EZO-DO** | Yes | Yes | No |
| **EZO-EC** | Yes | Yes | No |
| **EZO-RTD** | Yes | Yes | No |
| **EZO-CO2** | Yes | No | No |
| **EZO-O2** | Yes | No | No |
| **EZO-HUM** | Yes | No | No |
| **EZO-PRS** | Yes | No | No |
| **EZO-PMP** | Yes | No | No |
| **EZO-FLOW** | Yes | No | No |
| **EZO-RGB** | Yes | No | No |

## Integration

Add this repository to an ESP-IDF application's `components` directory and require the
component from the consuming component. The application owns the I2C driver lifecycle;
this component does not install or delete an I2C driver.

For example, initialize the legacy ESP-IDF I2C master driver before creating an EZO
handle:

```c
#include "driver/i2c.h"
#include "wb_ezo.h"

static esp_err_t init_ezo_ph(wb_ezo_device_handle_t *device)
{
    const i2c_config_t bus_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    esp_err_t err = i2c_param_config(I2C_NUM_0, &bus_config);
    if (err != ESP_OK) {
        return err;
    }
    err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK) {
        return err;
    }

    wb_ezo_device_config_t config;
    err = wb_ezo_get_default_config(EZO_TYPE_PH, &config);
    if (err != ESP_OK) {
        return err;
    }

    config.i2c_port = I2C_NUM_0;
    config.i2c_address = EZO_ADDR_PH; /* May be overridden for re-addressed devices. */
    config.io_timeout_ms = 200;
    return wb_ezo_init(device, &config);
}
```

The compatibility helper below uses the device defaults and `I2C_NUM_0`:

```c
wb_ezo_device_handle_t ph;
esp_err_t err = wb_ezo_init_desc(&ph, EZO_TYPE_PH);
```

## Reading and commands

```c
char reading[32];
esp_err_t err = wb_ezo_read_string(&ph, reading, sizeof(reading));
if (err == ESP_OK) {
    /* Parse or publish reading. */
}

/* Always invalidate the handle after all users have stopped. */
err = wb_ezo_deinit(&ph);
```

`wb_ezo_execute_command()` atomically performs write, processing delay, and response
read while holding the per-device mutex. It is the preferred API for commands that
return a status. `wb_ezo_send_command()` intentionally performs only the raw write and
therefore cannot confirm whether the device accepted the command.

When a device returns status `254` (pending), the command API retries reads according
to `pending_retries` and `pending_retry_delay_ms`. `wb_ezo_execute_command_ex()` and
`wb_ezo_read_response_ex()` additionally preserve the raw EZO status and payload in a
`wb_ezo_response_t`.

## Configuration

Start with `wb_ezo_get_default_config()` and override fields before calling
`wb_ezo_init()`:

- `i2c_port`: ESP-IDF I2C controller used by this device.
- `i2c_address`: 7-bit device address; address zero is rejected.
- `delay_ms`: processing delay used by `wb_ezo_read_string()`.
- `io_timeout_ms`: timeout for each transport read or write.
- `mutex_timeout_ms`: maximum wait for another operation on the same handle.
- `pending_retries`: additional reads after EZO status `254`.
- `pending_retry_delay_ms`: delay between pending retries.

A custom `wb_ezo_transport_t` can be installed with `wb_ezo_set_transport()`. This is
useful for alternate transports and deterministic host-side tests. Passing `NULL`
restores the default ESP-IDF I2C implementation.

## Thread safety and lifetime

Operations made through one initialized handle are serialized. The mutex covers the
complete write-delay-read transaction, preventing two tasks from exchanging responses.
Different handles are independent; synchronization of handles that point to the same
physical address is the application's responsibility.

Do not copy an initialized handle because it contains a static mutex. Configure the
handle and custom transport before sharing it with tasks. Stop all users before calling
`wb_ezo_deinit()`.

## Error handling

All public functions return `esp_err_t`; the component does not call `ESP_ERROR_CHECK()`
internally. Common mappings are:

- `ESP_FAIL`: the EZO device rejected the command (status `2`).
- `ESP_ERR_TIMEOUT`: the device remained pending or a lock timed out.
- `ESP_ERR_NOT_FOUND`: the EZO device reported no data (status `255`).
- `ESP_ERR_INVALID_RESPONSE`: the status or response payload was malformed.
- Transport-specific errors: I2C driver, timeout, or NACK failures.

## Examples

Each directory below is a standalone ESP-IDF project that includes this repository as
an external component:

- [`examples/basic_read`](examples/basic_read): initializes the I2C master and reads an
  EZO-pH circuit periodically. SDA, SCL, clock frequency, and reading interval are
  configurable through `idf.py menuconfig`.
- [`examples/multi_sensor`](examples/multi_sensor): configures EZO-pH, EZO-EC, EZO-DO,
  and EZO-RTD handles on one bus, applies compensation values where supported, and
  reads all devices sequentially.
- [`examples/custom_transport`](examples/custom_transport): installs an in-memory fake
  transport and demonstrates a pending response followed by a successful structured
  response. No EZO or I2C hardware is required.

Build an example from its project directory after loading the ESP-IDF environment:

```sh
cd examples/basic_read
idf.py set-target esp32
idf.py menuconfig
idf.py build
```

Flash and monitor it with the normal ESP-IDF commands for the connected target. For
`basic_read` and `multi_sensor`, configure GPIOs before flashing and ensure every EZO
circuit is in I2C mode with the expected address. The custom transport example can be
used as a smoke test for command execution and pending retries.
