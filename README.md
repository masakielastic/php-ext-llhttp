# PHP llhttp Extension

A PHP extension that provides high-performance HTTP parsing using the [llhttp](https://github.com/nodejs/llhttp) C library. This extension offers an event-driven, object-oriented API for parsing HTTP requests and responses.

## Features

- **High Performance**: Built on the fast llhttp C library used by Node.js
- **Event-Driven**: Asynchronous parsing with customizable callbacks
- **Object-Oriented API**: Clean, modern PHP interface
- **Request & Response Parsing**: Support for both HTTP requests and responses
- **State Management**: Pause, resume, and reset parsing operations
- **Header Processing**: Extract and process HTTP headers
- **Memory Efficient**: Streaming parser that handles large HTTP messages

## Installation

### Requirements

- PHP 8.0 or higher
- PHP development headers (`php-dev` package)
- C compiler (gcc/clang)
- autotools (autoconf, automake, libtool)

### Build from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/php-ext-llhttp.git
cd php-ext-llhttp

# Build the extension
phpize
./configure
make

# Install (requires sudo)
sudo make install

# Add to php.ini
echo "extension=llhttp.so" >> /etc/php/8.x/cli/php.ini
```

## Usage

### Basic HTTP Request Parsing

```php
<?php

use Llhttp\Parser;
use Llhttp\Events;

// Create a request parser
$parser = new Parser(Parser::TYPE_REQUEST);

// Set up event handlers
$parser->on(Events::MESSAGE_BEGIN, function() {
    echo "Parsing started\n";
});

$parser->on(Events::URL, function($url) {
    echo "URL: $url\n";
});

$parser->on(Events::HEADER_FIELD, function($field) {
    echo "Header field: $field\n";
});

$parser->on(Events::HEADER_VALUE, function($value) {
    echo "Header value: $value\n";
});

$parser->on(Events::HEADERS_COMPLETE, function() {
    echo "Headers complete\n";
});

$parser->on(Events::BODY, function($data) {
    echo "Body chunk: $data\n";
});

$parser->on(Events::MESSAGE_COMPLETE, function() {
    echo "Message complete\n";
});

// Parse HTTP request
$request = "GET /path?query=value HTTP/1.1\r\n" .
           "Host: example.com\r\n" .
           "Content-Length: 0\r\n\r\n";

$parser->execute($request);
$parser->finish();
```

### HTTP Response Parsing

```php
<?php

use Llhttp\Parser;
use Llhttp\Events;

// Create a response parser
$parser = new Parser(Parser::TYPE_RESPONSE);

$parser->on(Events::STATUS, function($status) {
    echo "Status: $status\n";
});

$parser->on(Events::HEADERS_COMPLETE, function() use ($parser) {
    echo "Status Code: " . $parser->getStatusCode() . "\n";
    echo "HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
});

// Parse HTTP response
$response = "HTTP/1.1 200 OK\r\n" .
            "Content-Type: text/html\r\n" .
            "Content-Length: 13\r\n\r\n" .
            "Hello, World!";

$parser->execute($response);
$parser->finish();
```

### State Management

```php
<?php

$parser = new Parser(Parser::TYPE_REQUEST);

// Pause parsing
$parser->pause();

// Check if paused
if ($parser->isPaused()) {
    echo "Parser is paused\n";
}

// Resume parsing
$parser->resume();

// Reset parser for reuse
$parser->reset();
```

## API Reference

### Parser Class

#### Constructor
- `new Parser(int $type)` - Create a new parser
  - `Parser::TYPE_REQUEST` - Parse HTTP requests
  - `Parser::TYPE_RESPONSE` - Parse HTTP responses

#### Event Management
- `on(string $event, callable $callback): Parser` - Register event handler
- `off(string $event): Parser` - Remove event handler

#### Parsing Methods
- `execute(string $data): void` - Parse HTTP data
- `finish(): void` - Complete parsing
- `pause(): void` - Pause parsing
- `resume(): void` - Resume parsing
- `reset(): void` - Reset parser state

#### Information Methods
- `getType(): int` - Get parser type
- `getHttpMajor(): int` - Get HTTP major version
- `getHttpMinor(): int` - Get HTTP minor version
- `getMethod(): int` - Get HTTP method (for requests)
- `getMethodName(): string` - Get HTTP method name
- `getStatusCode(): int` - Get status code (for responses)
- `shouldKeepAlive(): bool` - Check if connection should be kept alive
- `messageNeedsEof(): bool` - Check if message needs EOF
- `getHeaders(): array` - Get parsed headers
- `isPaused(): bool` - Check if parser is paused

### Events Class

Available event constants:
- `Events::MESSAGE_BEGIN` - Message parsing started
- `Events::URL` - URL data received (requests only)
- `Events::STATUS` - Status line received (responses only)
- `Events::HEADER_FIELD` - Header field received
- `Events::HEADER_VALUE` - Header value received
- `Events::HEADERS_COMPLETE` - All headers received
- `Events::BODY` - Body data received
- `Events::MESSAGE_COMPLETE` - Message parsing complete

### ErrorCodes Class

HTTP parsing error constants (see llhttp documentation for complete list).

## Testing

Run the test suite:

```bash
# Basic functionality test
php test_simple.php

# Header parsing test
php test_header_events.php

# State management test
php test_state.php

# Response parsing test
php test_simple_response.php
```

## Performance

This extension leverages the llhttp C library, which is designed for high performance:

- Zero-copy parsing where possible
- Minimal memory allocations
- Optimized state machine implementation
- Used in production by Node.js

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [llhttp](https://github.com/nodejs/llhttp) - The underlying HTTP parser
- [php-ffi-llhttp](https://github.com/queldev/php-ffi-llhttp) - Reference FFI implementation
- PHP Extension Development documentation

## Changelog

### v0.1.0 (Current)

- Initial implementation
- Basic HTTP request/response parsing
- Event-driven callback system
- State management (pause/resume/reset)
- Header field and value events
- Comprehensive test suite

### Known Issues

- Response parsing may show "Invalid method" error in some cases
- Header collection system uses event-only approach for memory safety
- Missing arginfo declarations cause PHP warnings

## Roadmap

- [ ] Fix response parsing issues
- [ ] Implement automatic header collection
- [ ] Add arginfo declarations
- [ ] Performance benchmarks
- [ ] Documentation improvements
- [ ] Additional test coverage
- [ ] Error handling enhancements