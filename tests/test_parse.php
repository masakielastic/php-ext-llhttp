<?php

try {
    echo "Testing HTTP parsing functionality\n\n";
    
    $parser = new Llhttp\Parser(Llhttp\Parser::TYPE_REQUEST);
    
    // Set up event callbacks
    $parser->on(Llhttp\Events::MESSAGE_BEGIN, function() {
        echo "Event: MESSAGE_BEGIN\n";
    });
    
    $parser->on(Llhttp\Events::URL, function($url) {
        echo "Event: URL = '$url'\n";
    });
    
    $parser->on(Llhttp\Events::HEADER_FIELD, function($field) {
        echo "Event: HEADER_FIELD = '$field'\n";
    });
    
    $parser->on(Llhttp\Events::HEADER_VALUE, function($value) {
        echo "Event: HEADER_VALUE = '$value'\n";
    });
    
    $parser->on(Llhttp\Events::HEADERS_COMPLETE, function() {
        echo "Event: HEADERS_COMPLETE\n";
    });
    
    $parser->on(Llhttp\Events::BODY, function($chunk) {
        echo "Event: BODY = '$chunk'\n";
    });
    
    $parser->on(Llhttp\Events::MESSAGE_COMPLETE, function() {
        echo "Event: MESSAGE_COMPLETE\n";
    });
    
    // Simple HTTP request
    $request = "GET /test HTTP/1.1\r\n" .
               "Host: example.com\r\n" .
               "Content-Length: 5\r\n" .
               "\r\n" .
               "Hello";
    
    echo "Parsing HTTP request:\n";
    echo $request . "\n\n";
    
    echo "Parser output:\n";
    $parser->execute($request);
    $parser->finish();
    
    echo "\nParsing completed!\n";
    echo "HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
    echo "Method: " . $parser->getMethodName() . "\n";
    echo "Headers: " . json_encode($parser->getHeaders()) . "\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
    echo "Code: " . $e->getCode() . "\n";
    echo "Trace: " . $e->getTraceAsString() . "\n";
}