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
component from the consuming component.

This component uses the modern ESP-IDF I2C Master API (v1.x) and provides convenient
helpers for initializing and managing EZO sensor buses and devices.

### Basic Usage with I2C Master API

Initialize an I2C bus and create device handles:

```c
#include "wb_ezo_i2c.h"
#include "wb_ezo.h"

// Initialize I2C bus
i2c_master_bus_handle_t i2c_bus;
esp_err_t err = wb_ezo_i2c_bus_init(I2C_NUM_0,
                                     GPIO_NUM_22, // SCL
                                     GPIO_NUM_21, // SDA
                                     &i2c_bus);
if (err != ESP_OK) {
    return err;
}

// Create a device handle on the bus
i2c_master_dev_handle_t ph_device = wb_ezo_i2c_device_create(
    i2c_bus,
    EZO_ADDR_PH,      // 7-bit I2C address
    400000            // Clock speed in Hz
);

// Optionally assign a human-readable name
wb_ezo_i2c_device_set_name(ph_device, "pH Sensor");

// Probe whether a device is present
err = wb_ezo_i2c_bus_probe_device(i2c_bus, EZO_ADDR_PH, 1000);
if (err == ESP_OK) {
    // Device is present
}

// Clean up
wb_ezo_i2c_device_delete(ph_device);
wb_ezo_i2c_bus_delete(i2c_bus);
```

### Multiple Sensors on One Bus

Create multiple device handles on the same I2C bus:

```c
i2c_master_bus_handle_t i2c_bus;
wb_ezo_i2c_bus_init(I2C_NUM_0, GPIO_NUM_22, GPIO_NUM_21, &i2c_bus);

// Create handles for different EZO devices
i2c_master_dev_handle_t ph_dev = wb_ezo_i2c_device_create(i2c_bus, EZO_ADDR_PH, 400000);
i2c_master_dev_handle_t ec_dev = wb_ezo_i2c_device_create(i2c_bus, EZO_ADDR_EC, 400000);
i2c_master_dev_handle_t do_dev = wb_ezo_i2c_device_create(i2c_bus, EZO_ADDR_DO, 400000);

wb_ezo_i2c_device_set_name(ph_dev, "pH");
wb_ezo_i2c_device_set_name(ec_dev, "Conductivity");
wb_ezo_i2c_device_set_name(do_dev, "Dissolved Oxygen");

// Use devices...

// Clean up in reverse order
wb_ezo_i2c_device_delete(ph_dev);
wb_ezo_i2c_device_delete(ec_dev);
wb_ezo_i2c_device_delete(do_dev);
wb_ezo_i2c_bus_delete(i2c_bus);
```

### Using Sensor-Specific Helpers

The component includes dedicated helpers for pH, EC, DO, and RTD sensors. These provide
convenient functions for common operations. See `include/probes/` for available helpers.

## Reading and commands

The component provides a low-level API for reading from and sending commands to EZO
devices. Example:

```c
#include "wb_ezo_i2c.h"
#include "wb_ezo.h"

i2c_master_bus_handle_t i2c_bus;
i2c_master_dev_handle_t device;

// Initialize bus and device (see Integration section)
wb_ezo_i2c_bus_init(I2C_NUM_0, GPIO_NUM_22, GPIO_NUM_21, &i2c_bus);
device = wb_ezo_i2c_device_create(i2c_bus, EZO_ADDR_PH, 400000);

// Perform a read with automatic delay
char reading[32];
esp_err_t err = wb_ezo_read_string(device, reading, sizeof(reading));
if (err == ESP_OK) {
    // Parse or publish the reading
    float ph_value = atof(reading);
}

// Send a command and read the response
wb_ezo_execute_command(device, "Cal,mid,7.0", 900, reading, sizeof(reading));

// Send a command without reading status (e.g., configuration commands)
wb_ezo_send_command(device, "L,0");  // Turn off LED

// Get device info
char info[48];
wb_ezo_get_device_info(device, info, sizeof(info));

// Query calibration status
int cal_status = 0;
wb_ezo_get_calibration_status(device, &cal_status);

// Clean up
wb_ezo_i2c_device_delete(device);
wb_ezo_i2c_bus_delete(i2c_bus);
```

### API Functions

**`wb_ezo_execute_command()`** atomically performs write, processing delay, and response
read while holding the per-device lock. This is the preferred API for commands that
return a status. **`wb_ezo_send_command()`** intentionally performs only the raw write and
therefore cannot confirm whether the device accepted the command.

When a device returns status `254` (pending), the command API automatically retries
reads according to `pending_retries` and `pending_retry_delay_ms` in the device config.

Use **`wb_ezo_execute_command_ex()`** and **`wb_ezo_read_response_ex()`** when you need
to preserve the raw EZO status and payload in a `wb_ezo_response_t` structure.

## I2C Bus Configuration

Initialize an I2C bus using `wb_ezo_i2c_bus_init()` with the desired port, SCL, and SDA
GPIO pins. The bus clock frequency is fixed at 400 kHz.

### Device Configuration

When creating a device with `wb_ezo_i2c_device_create()`, you specify:

- **I2C bus handle**: The bus created by `wb_ezo_i2c_bus_init()`.
- **Device address**: 7-bit I2C address (e.g., `EZO_ADDR_PH` = 0x63).
- **Clock speed**: Device clock frequency in Hz (typically 400000 or 100000).

You can optionally assign a human-readable name using `wb_ezo_i2c_device_set_name()`
for easier debugging and logging.

### Advanced: Custom Transport

For testing or alternate transports, use `wb_ezo_set_transport()` with a custom
`wb_ezo_transport_t`. Pass `NULL` to restore the default I2C transport.

## Thread Safety and Lifetime

Operations through one device handle are serialized; the I2C driver manages the bus lock.
The per-device mutex prevents two tasks from exchanging responses on the same device.

### Proper Cleanup

Always clean up resources in the reverse order of creation:

```c
// Clean up in reverse order
wb_ezo_i2c_device_delete(device1);
wb_ezo_i2c_device_delete(device2);
wb_ezo_i2c_bus_delete(i2c_bus);
```

Do not reuse a device or bus handle after deletion. Stop all tasks using the handles
before calling the delete functions.

## Error handling

All public functions return `esp_err_t`; the component does not call `ESP_ERROR_CHECK()`
internally. Common mappings are:

- `ESP_FAIL`: the EZO device rejected the command (status `2`).
- `ESP_ERR_TIMEOUT`: the device remained pending or a lock timed out.
- `ESP_ERR_NOT_FOUND`: the EZO device reported no data (status `255`).
- `ESP_ERR_INVALID_RESPONSE`: the status or response payload was malformed.
- Transport-specific errors: I2C driver, timeout, or NACK failures.

## Examples

Each directory in `examples/` is a standalone ESP-IDF project that demonstrates
component usage:

- **`examples/basic_read`**: Initializes the modern I2C Master bus, creates device
  handles for pH and temperature sensors, and reads values periodically. GPIO pins,
  I2C port, and reading interval are configurable through `idf.py menuconfig`.

- **`examples/multi_sensor`**: Creates handles for pH, EC, DO, and RTD sensors on one
  bus, applies compensation values where supported, and reads all devices sequentially.

- **`examples/custom_transport`**: Installs an in-memory fake transport and demonstrates
  a pending response followed by a successful structured response. No EZO or I2C hardware
  is required; useful for testing and CI/CD pipelines.

### Building and Running Examples

Load the ESP-IDF environment and navigate to an example directory:

```sh
cd examples/basic_read
idf.py set-target esp32
idf.py menuconfig       # Configure GPIO pins and settings
idf.py build
idf.py flash
idf.py monitor
```

Configure GPIO pins and I2C settings before flashing real hardware. Ensure every EZO
circuit is in I2C mode with the expected address.

The `custom_transport` example runs on any machine and requires no hardware—use it as
a smoke test for command execution and pending retries.
