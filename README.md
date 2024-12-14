# Synergy Through USB

A USB data link implementation for the Synergy project, enabling keyboard and mouse sharing through USB connections.

## Overview
This project implements USB device communication for Synergy, allowing direct USB connections between computers for sharing keyboard and mouse control. It uses libusb for cross-platform USB communication support.

## Features
- Cross-platform USB communication (Linux, Windows, macOS)
- Efficient data transfer using bulk endpoints
- Robust error handling and recovery
- Thread-safe implementation

## Requirements
- libusb-1.0
- CMake build system
- C++ compiler with C++11 support

## USB Support
For USB functionality documentation, see [USB Documentation](docs/usb/README.md).

## License
This package is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License. See the COPYING file for details.

## Contributing
Contributions are welcome! Please read through our documentation and feel free to submit pull requests.
