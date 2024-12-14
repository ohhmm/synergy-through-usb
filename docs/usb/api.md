# USB API Documentation

## Core Classes

### CUSBDataLink
Main class for USB communication handling. Provides high-level interface for USB device communication with support for asynchronous operations and event-based notifications.

#### Key Methods
- `connect(const CBaseAddress&)`: Establish USB connection with the specified device address
  - Parameters:
    - addr: USB device address containing VID, PID, and endpoint information
  - Throws: XSocketConnect on connection failure

- `read(void* buffer, UInt32 n)`: Read data from the USB device
  - Parameters:
    - buffer: Destination buffer for read data
    - n: Maximum number of bytes to read
  - Returns: Number of bytes actually read
  - Thread-safe: Yes

- `write(const void* buffer, UInt32 n)`: Write data to the USB device
  - Parameters:
    - buffer: Source buffer containing data to write
    - n: Number of bytes to write
  - Thread-safe: Yes
  - Note: Asynchronous operation, use flush() to ensure data is written

#### Additional Methods
- `bind(const CBaseAddress&)`: Bind to a USB device for listening
- `close()`: Close the USB connection and cleanup resources
- `flush()`: Wait for all pending writes to complete
- `shutdownInput()`: Shutdown the input channel
- `shutdownOutput()`: Shutdown the output channel

### CUSBAddress
USB device addressing class. Handles device identification and endpoint configuration.

#### Configuration
- VID/PID Configuration
  - `GetVID()`: Get Vendor ID
  - `GetPID()`: Get Product ID
  - Used for device identification and matching

- Endpoint Setup
  - `GetIDBulkIN()`: Get bulk IN endpoint number
  - `GetIDBulkOut()`: Get bulk OUT endpoint number
  - Used for data transfer configuration

- Bus/Device Addressing
  - `GetIDBus()`: Get USB bus number
  - `GetIDDeviceOnBus()`: Get device address on the bus
  - Used for precise device selection

### Error Handling
The API uses exception handling for error reporting:
- `XSocketConnect`: Connection-related errors
- `XArchNetwork`: Network/USB communication errors
- `XArchUsbTransferFailure`: Data transfer failures

### Event System
The USB implementation supports event-based notifications for:
- Connection status changes
- Data availability
- Transfer completion
- Error conditions

### Thread Safety
- All public methods are thread-safe
- Internal synchronization using mutex and condition variables
- Supports concurrent read/write operations

### Performance Considerations
- Uses asynchronous transfers for better performance
- Configurable transfer timeouts (default: 2000ms)
- Supports bulk transfer optimization
