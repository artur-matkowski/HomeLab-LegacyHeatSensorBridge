# HomeLab-LegacyHeatSensorBridge

A C++ bridge service that receives temperature and flow sensor data via UDP from legacy heating system sensors and stores them in a PostgreSQL database. This service is designed for home automation and monitoring systems.

## Overview

This application listens for UDP messages containing temperature readings from various heating system components (MCU temperature, hot water, central heating) and flow sensor data. It parses the incoming messages, detects value changes, and stores the data in a PostgreSQL database for monitoring and analysis.

## Features

- **UDP Message Reception**: Listens on port 5005 for sensor data messages
- **Change Detection**: Only stores values when they change, reducing database writes
- **Multiple Sensor Support**: Handles various temperature and flow sensors:
  - MCU Temperature (`MCUt`)
  - Hot Water Temperature (`CWU`)
  - Hot Water High Temperature (`CWUh`)
  - Hot Water Low Temperature (`CWUc`)
  - Central Heating High Temperature (`COh`)
  - Central Heating Low Temperature (`COc`)
  - Flow Sensors (f1, f2)
- **PostgreSQL Integration**: Automatic storage of sensor readings with timestamps
- **Configurable Logging**: Supports DEBUG and INFO log levels
- **Docker Support**: Fully containerized with development and production images
- **Signal Handling**: Graceful shutdown on SIGINT (Ctrl+C)

## Prerequisites

- **Build Dependencies**:
  - C++17 compatible compiler (g++)
  - CMake or Make
  - Git (for submodules)

- **Runtime Dependencies**:
  - libssl-dev
  - libpoco-dev (Poco C++ Libraries)
  - libpqxx-dev (PostgreSQL C++ client library)
  - libpq-dev
  - PostgreSQL database (accessible via network)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/artur-matkowski/HomeLab-LegacyHeatSensorBridge.git
cd HomeLab-LegacyHeatSensorBridge
git submodule update --init --recursive
```

### 2. Install Dependencies

On Debian/Ubuntu-based systems:

```bash
make install-deps
```

Or manually:

```bash
apt install -y libssl-dev libpoco-dev libpqxx-dev libpq-dev
```

### 3. Build the Project

**Release Build** (default):
```bash
make
# or explicitly
make release
```

**Debug Build**:
```bash
make debug
```

The compiled binary will be located at:
- Release: `build/<architecture>/rel/HomeLab-LegacyHeatSensorBridge`
- Debug: `build/<architecture>/dbg/HomeLab-LegacyHeatSensorBridge`

### 4. Clean Build Artifacts

```bash
make clean
```

## Configuration

The application requires the following environment variables to be set:

| Variable | Description | Required |
|----------|-------------|----------|
| `DB_HOST` | PostgreSQL server hostname or IP address | Yes |
| `DB_NAME` | Name of the PostgreSQL database | Yes |
| `DB_PASSWD` | Password for database authentication | Yes |
| `LOG_LEVEL` | Logging level (`DEBUG` or `INFO`) | No (defaults to INFO) |

### Example Configuration

Create a `.env` file (already in `.gitignore`):

```bash
DB_HOST=192.168.1.100
DB_NAME=homelab
DB_PASSWD=your_secure_password
LOG_LEVEL=DEBUG
```

## Usage

### Running Locally

```bash
# Set environment variables
export DB_HOST=192.168.1.100
export DB_NAME=homelab
export DB_PASSWD=your_secure_password
export LOG_LEVEL=INFO

# Run the application
./build/amd64/rel/HomeLab-LegacyHeatSensorBridge
```

### Running with Docker

#### Build Docker Image

**Production Image**:
```bash
make docker-image-rel
```

**Development Image**:
```bash
make docker-image-dev
```

#### Run with Docker Compose

1. Create a `.env` file with your database configuration (see Configuration section)

2. Start the service:
```bash
docker-compose up -d
```

3. View logs:
```bash
docker-compose logs -f legacy_heat_sensor_bridge
```

4. Stop the service:
```bash
docker-compose down
```

## Message Format

The application expects UDP messages in the following format:

```
{MCUt:25.50}{CWU:45.30}{CWUh:60.00}{CWUc:30.00}{COh:70.00}{COc:40.00}{API:v1.0.0}{f1:100}{f2:150}
```

Where:
- Temperature values are in degrees Celsius
- Flow values are integers
- API field contains version information

## Database Schema

The application uses a PostgreSQL database with a message storage schema. The database connection is configured with:
- Default user: `admin`
- Port: `5432`
- Configurable host, database name, and password via environment variables

## Architecture

The project consists of:
- **main.cpp**: Main application entry point with UDP listener and message processing loop
- **src/PSQL/**: PostgreSQL storage implementation
- **modules/udp/**: UDP communication module (git submodule)
- **modules/hc12-message-definitions/**: HC12 message definitions (git submodule)

## Development

### Project Structure

```
.
├── main.cpp                    # Main application entry point
├── src/
│   ├── PSQL/                  # PostgreSQL storage implementation
│   └── Message/               # Message definitions
├── modules/                   # Git submodules
│   ├── udp/                   # UDP communication module
│   └── hc12-message-definitions/
├── makefile                   # Build configuration
├── docker-compose.yml         # Docker Compose configuration
├── Dockerfile-rel             # Production Docker image
├── Dockerfile-dev             # Development Docker image
├── version.sh                 # Semantic versioning script
└── README.md                  # This file
```

### Versioning

This project uses semantic versioning based on Git commit history. The version is automatically derived from commit messages:

- `api-break:` in commit message → bumps MAJOR version
- `release:` in commit message → bumps MINOR version
- Other commits → bumps PATCH version

Run `./version.sh` to see the current version.

### Debug Mode

When `LOG_LEVEL=DEBUG` is set, the application outputs detailed change detection logs:

```
[DEBUG] MCUt changed: 2500 -> 2550
[DEBUG] CWU changed: 4530 -> 4540
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes with appropriate prefixes:
   - `api-break:` for breaking changes
   - `release:` for new features
   - Regular commits for patches
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is part of a HomeLab setup. Please check with the repository owner for licensing information.

## Troubleshooting

### Cannot connect to database
- Verify `DB_HOST`, `DB_NAME`, and `DB_PASSWD` environment variables are set correctly
- Ensure PostgreSQL server is accessible from the application host
- Check firewall rules if running in Docker with `network_mode: host`

### No messages received
- Verify UDP port 5005 is not blocked by firewall
- Check that the legacy sensor is sending messages to the correct IP address
- Use `tcpdump` or `wireshark` to verify UDP packets are arriving

### Build errors
- Ensure all dependencies are installed: `make install-deps`
- Update git submodules: `git submodule update --init --recursive`
- Try cleaning and rebuilding: `make clean && make`

## Support

For issues, questions, or contributions, please open an issue in the GitHub repository.
