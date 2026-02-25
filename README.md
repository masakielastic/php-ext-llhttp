# PHP llhttp Extension

A PHP extension that provides high-performance HTTP parsing using the [llhttp](https://github.com/nodejs/llhttp) C library. This extension offers a simple, direct API for parsing HTTP requests and responses with excellent performance.

## Features

- **High Performance**: Built on the fast llhttp C library used by Node.js
- **Simple API**: Clean, direct method calls without complex event systems
- **Object-Oriented**: Modern PHP interface with intuitive methods
- **Request & Response Parsing**: Support for both HTTP requests and responses
- **Built-in Data Collection**: Automatic header, URL, and body collection
- **Memory Efficient**: Streaming parser that handles large HTTP messages
- **State Management**: Reset and reuse parsers efficiently

## Installation

### Requirements

- PHP 8.0 or higher
- PHP development headers (`php-dev` package)
- C compiler (gcc/clang)
- autotools (autoconf, automake, libtool)

### Install with PIE

[PIE](https://github.com/php/pie) (PHP Installer for Extensions) is the recommended installation method.

```bash
pie install masakielastic/llhttp
```

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

// Create a request parser
$parser = new Parser(Parser::TYPE_REQUEST);

$request = "GET /api/users?page=1 HTTP/1.1\r\n" .
           "Host: example.com\r\n" .
           "Content-Type: application/json\r\n" .
           "Authorization: Bearer token123\r\n" .
           "\r\n" .
           '{"data": "request body"}';

// Parse the request
$parser->parse($request);
$parser->parseComplete();

// Get parsed information
echo "Method: " . $parser->getMethodName() . "\n";
echo "URL: " . $parser->getUrl() . "\n";
echo "HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
echo "Body: " . $parser->getBody() . "\n";

// Get specific header
$host = $parser->getHeader('Host');
echo "Host: $host\n";

// Get all headers
$headers = $parser->getHeaders();
foreach ($headers as $name => $value) {
    echo "Header: $name = $value\n";
}
```

### HTTP Response Parsing

```php
<?php

use Llhttp\Parser;

// Create a response parser
$parser = new Parser(Parser::TYPE_RESPONSE);

$response = "HTTP/1.1 200 OK\r\n" .
            "Content-Type: application/json\r\n" .
            "Content-Length: 27\r\n" .
            "Server: nginx/1.20.1\r\n" .
            "\r\n" .
            '{"message": "Hello, World!"}';

// Parse the response
$parser->parse($response);
$parser->parseComplete();

// Get response information
echo "Status Code: " . $parser->getStatusCode() . "\n";
echo "HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
echo "Server: " . $parser->getHeader('Server') . "\n";
echo "Content-Type: " . $parser->getHeader('Content-Type') . "\n";
echo "Body: " . $parser->getBody() . "\n";
echo "Keep-Alive: " . ($parser->shouldKeepAlive() ? 'Yes' : 'No') . "\n";
```

### Streaming Parsing

```php
<?php

use Llhttp\Parser;

$parser = new Parser(Parser::TYPE_REQUEST);

// Parse data in chunks
$chunks = [
    "GET /path HTTP/1.1\r\n",
    "Host: example.com\r\n",
    "Content-Length: 13\r\n",
    "\r\n",
    '{"test": true}'
];

foreach ($chunks as $chunk) {
    $parser->parse($chunk);
    
    // Check if parsing is complete
    if ($parser->isComplete()) {
        break;
    }
}

echo "URL: " . $parser->getUrl() . "\n";
echo "Body: " . $parser->getBody() . "\n";
```

### Auto-Detection with TYPE_BOTH

```php
<?php

use Llhttp\Parser;

// Create flexible parser that auto-detects request/response
$parser = new Parser(Parser::TYPE_BOTH);

// Can parse HTTP requests
$request = "GET /api HTTP/1.1\r\nHost: example.com\r\n\r\n";
$parser->parse($request);
$parser->parseComplete();
echo "Parsed as request: " . $parser->getMethodName() . "\n";

// Create new instance for response (recommended for TYPE_BOTH)
$responseParser = new Parser(Parser::TYPE_BOTH);
$response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello";
$responseParser->parse($response);
$responseParser->parseComplete();
echo "Parsed as response: " . $responseParser->getStatusCode() . "\n";
```

### Parser State Management

```php
<?php

use Llhttp\Parser;

$parser = new Parser(Parser::TYPE_REQUEST);

// Parse some data
$parser->parse("GET /test HTTP/1.1\r\nHost: example.com\r\n\r\n");

// Check parser state
echo "State: " . $parser->getState() . "\n";
echo "Complete: " . ($parser->isComplete() ? 'Yes' : 'No') . "\n";

// Reset parser for reuse
$parser->reset();
echo "State after reset: " . $parser->getState() . "\n";

// Parse new data
$parser->parse("POST /api HTTP/1.1\r\n\r\n");
echo "New method: " . $parser->getMethodName() . "\n";
```

## API Reference

### Parser Class

#### Constructor
- `new Parser(int $type)` - Create a new parser
  - `Parser::TYPE_BOTH` - Auto-detect request or response (flexible parsing)
  - `Parser::TYPE_REQUEST` - Parse HTTP requests only
  - `Parser::TYPE_RESPONSE` - Parse HTTP responses only

#### Parsing Methods
- `parse(string $data): void` - Parse HTTP data chunk
- `parseComplete(): void` - Signal end of data (optional)
- `reset(): void` - Reset parser state for reuse

#### HTTP Information Methods
- `getHttpMajor(): int` - Get HTTP major version
- `getHttpMinor(): int` - Get HTTP minor version
- `getMethod(): int` - Get HTTP method code (for requests)
- `getMethodName(): string` - Get HTTP method name (for requests)
- `getStatusCode(): int` - Get status code (for responses)
- `getUrl(): string` - Get request URL (for requests)
- `shouldKeepAlive(): bool` - Check if connection should be kept alive
- `messageNeedsEof(): bool` - Check if message needs EOF

#### Data Access Methods
- `getHeaders(): array` - Get all parsed headers as array
- `getHeader(string $name): ?string` - Get specific header value (case-insensitive)
- `getBody(): string` - Get request/response body

#### State Methods
- `isComplete(): bool` - Check if parsing is complete
- `getState(): int` - Get current parser state
  - `Parser::STATE_INIT` - Initial state
  - `Parser::STATE_PARSING` - Currently parsing
  - `Parser::STATE_COMPLETE` - Parsing complete
  - `Parser::STATE_ERROR` - Error occurred

### ErrorCodes Class

HTTP parsing error constants (see llhttp documentation for complete list).

### Exception Handling

```php
<?php

use Llhttp\Parser;
use Llhttp\ParseException;

$parser = new Parser(Parser::TYPE_REQUEST);

try {
    $parser->parse("INVALID HTTP DATA");
} catch (ParseException $e) {
    echo "Parse error: " . $e->getMessage() . "\n";
    echo "Error code: " . $e->getErrorCode() . "\n";
}
```

## Testing

Run the test suite:

```bash
# Test new API
php test_new_api.php

# Test basic functionality
php test_basic.php

# Test header collection
php test_getheaders.php

# Test response parsing
php test_simple_response.php
```

## Performance

This extension leverages the llhttp C library for maximum performance:

- **Zero-copy parsing** where possible
- **Minimal memory allocations**
- **Optimized state machine implementation** 
- **Direct C integration** without FFI overhead
- **Used in production** by Node.js

## Architecture

The extension follows a **streamlined, object-oriented design**:

- **Direct Method Calls**: Simple, intuitive API without complex event systems
- **Automatic Data Collection**: Headers, URL, and body are collected automatically
- **State Management**: Clean state tracking with reset/reuse capability
- **Memory Efficient**: Proper cleanup and efficient string handling

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- [llhttp](https://github.com/nodejs/llhttp) - The underlying HTTP parser
- PHP Extension Development documentation

- [ ] Add arginfo declarations for better reflection support
- [ ] Performance benchmarks vs other PHP HTTP parsers
- [ ] Additional utility methods for URL parsing
- [ ] Enhanced duplicate header handling
- [ ] Package for PECL distribution
- [ ] Comprehensive documentation and examples
