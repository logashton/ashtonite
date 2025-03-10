# Ashtonite Web Server

A lightweight, multi-threaded web server for static websites. Built from scratch in C++ and implements HTTP/1.1 .

## Features

- HTTP/1.1 compliant
- Multi-threaded request handling
- Keep-alive connection support
- Static file serving with MIME type detection
- Path traversal protection
- Configurable through command-line arguments
- Non-blocking I/O
- Graceful shutdown support

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
./ashtonite [options]
```

### Command-line Options

- `--address ADDR`: Address to bind to (default: 0.0.0.0)
- `--port PORT`: Port to listen on (default: 8080)
- `--public-dir DIR`: Directory to serve files from (default: public)
- `--threads N`: Number of worker threads (default: CPU cores)
- `--max-connections N`: Maximum number of pending connections (default: 1000)
- `--help`: Show help message

### Example

```bash
# Start server on port 8080 serving files from ./public directory
./ashtonite

# Start server on specific address and port
./ashtonite --address 127.0.0.1 --port 3000

# Start server with custom settings
./ashtonite --port 8080 --threads 8 --max-connections 2000
```

## Directory Structure

- `public/`: Place your static files in this directory
- `src/`: Source files
- `include/`: Header files
- `build/`: Build directory (created during build)

## Security

The server implements several security measures:

1. Path traversal protection
2. Request size limits
3. Connection timeouts
4. Non-root process execution
5. Proper error handling

## Performance

- Non-blocking I/O for connection acceptance
- Thread pool for request handling
- Keep-alive connection support
- Efficient static file serving
- Configurable number of worker threads

