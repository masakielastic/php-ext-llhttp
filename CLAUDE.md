# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This project develops a PHP extension for llhttp - a high-performance HTTP parser written in C. The extension provides a PHP API that mirrors the design patterns established in the `@php-ffi-llhttp` reference implementation while delivering native C performance.

### Architecture

The extension follows an event-driven, object-oriented design:

- **PHP Extension Layer**: Native C code that integrates llhttp with PHP's Zend Engine
- **Parser Class**: Main OOP interface for HTTP parsing operations  
- **Event System**: Callback-based architecture for different parsing phases
- **Error Handling**: Comprehensive error codes and exception handling
- **Memory Management**: Efficient handling of C structures and PHP objects

### Key Components

- `api.c/api.h`: Core llhttp API functions and structures
- `http.c`: HTTP protocol logic and helper functions  
- `llhttp.c/llhttp.h`: Main llhttp parser implementation
- PHP extension files (to be created): PHP class definitions and Zend Engine integration

### Reference Implementation

The `php-ffi-llhttp/` directory contains the FFI-based reference implementation that serves as the design blueprint:

- **Parser Class Design**: Event-driven API with `on()`, `execute()`, `finish()` methods
- **Event Types**: MESSAGE_BEGIN, URL, HEADERS_COMPLETE, BODY, MESSAGE_COMPLETE
- **State Management**: `pause()`, `resume()`, `reset()` functionality
- **Data Access**: Methods for retrieving HTTP version, method, status code, headers

## Development Commands

### Extension Build
```bash
phpize                    # Generate configure script
./configure              # Configure build environment  
make                     # Compile extension
make install             # Install extension
```

### Testing
```bash
make test                # Run extension test suite
php -m | grep llhttp     # Verify extension is loaded
```

### Development Workflow
```bash
make clean               # Clean build artifacts
phpize --clean          # Clean phpize files
```

## Commit Rules

This project follows Conventional Commits specification:

### Format
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

### Types
- **feat**: New functionality (classes, methods, features)
- **fix**: Bug fixes and error corrections
- **docs**: Documentation updates
- **style**: Code formatting and style changes
- **refactor**: Code restructuring without behavior changes
- **perf**: Performance optimizations
- **test**: Test additions and modifications
- **chore**: Build system, dependencies, tooling changes

### Examples
```
feat(parser): implement HTTP request parsing API
fix(memory): resolve leak in parser cleanup
docs: add extension installation guide
test(parser): add unit tests for header parsing
```

## Extension Development Notes

### Class Design Pattern

Follow the reference implementation structure:
- Parser constructor with type parameter (REQUEST/RESPONSE)
- Event registration via `on(event, callback)` method
- Data parsing via `execute(data)` method
- State control via `pause()`, `resume()`, `reset()` methods
- Information getters: `getMethod()`, `getStatusCode()`, `getHeaders()`

### Error Handling

Implement comprehensive error handling:
- Map llhttp error codes to PHP exceptions
- Provide detailed error messages and context
- Handle memory allocation failures gracefully

### Memory Management

- Properly manage C structure lifecycles
- Use PHP's reference counting for objects
- Clean up resources in destructor methods
- Handle circular references appropriately

### PHP Integration

- Define PHP classes using Zend Engine APIs
- Implement proper object handlers and methods
- Support PHP's error and exception mechanisms
- Ensure thread safety where applicable