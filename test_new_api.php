<?php

echo "=== Testing New API ===\n";

try {
    // Test request parsing
    echo "1. Testing request parsing...\n";
    $parser = new Llhttp\Parser(Llhttp\Parser::TYPE_REQUEST);
    
    $body = '{"test": true}';
    $request = "GET /test?param=value HTTP/1.1\r\n" .
               "Host: example.com\r\n" .
               "Content-Type: application/json\r\n" .
               "Content-Length: " . strlen($body) . "\r\n" .
               "\r\n" .
               $body;
    
    $parser->parse($request);
    $parser->parseComplete();
    
    echo "  - Method: " . $parser->getMethodName() . "\n";
    echo "  - URL: " . $parser->getUrl() . "\n";
    echo "  - HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
    echo "  - Host header: " . $parser->getHeader('Host') . "\n";
    echo "  - Content-Type: " . $parser->getHeader('Content-Type') . "\n";
    echo "  - Body: " . $parser->getBody() . "\n";
    echo "  - Is Complete: " . ($parser->isComplete() ? 'Yes' : 'No') . "\n";
    echo "  - State: " . $parser->getState() . "\n";
    
    // Test response parsing
    echo "\n2. Testing response parsing...\n";
    $parser2 = new Llhttp\Parser(Llhttp\Parser::TYPE_RESPONSE);
    
    $responseBody = '{"message": "Hello, World!"}';
    $response = "HTTP/1.1 200 OK\r\n" .
                "Content-Type: application/json\r\n" .
                "Content-Length: " . strlen($responseBody) . "\r\n" .
                "Server: nginx/1.20.1\r\n" .
                "\r\n" .
                $responseBody;
    
    $parser2->parse($response);
    $parser2->parseComplete();
    
    echo "  - Status Code: " . $parser2->getStatusCode() . "\n";
    echo "  - HTTP Version: " . $parser2->getHttpMajor() . "." . $parser2->getHttpMinor() . "\n";
    echo "  - Content-Type: " . $parser2->getHeader('Content-Type') . "\n";
    echo "  - Server: " . $parser2->getHeader('Server') . "\n";
    echo "  - Body: " . $parser2->getBody() . "\n";
    echo "  - Is Complete: " . ($parser2->isComplete() ? 'Yes' : 'No') . "\n";
    
    // Test headers collection
    echo "\n3. Testing headers collection...\n";
    $headers = $parser2->getHeaders();
    echo "  - All headers: " . json_encode($headers) . "\n";
    
    // Test reset functionality
    echo "\n4. Testing reset functionality...\n";
    $parser->reset();
    echo "  - State after reset: " . $parser->getState() . "\n";
    echo "  - Is Complete after reset: " . ($parser->isComplete() ? 'Yes' : 'No') . "\n";
    
    echo "\n=== All tests completed successfully! ===\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
    echo "Trace: " . $e->getTraceAsString() . "\n";
}