<?php

echo "Testing header event callbacks...\n";

try {
    $parser = new Llhttp\Parser(Llhttp\Parser::TYPE_REQUEST);
    
    $events = [];
    
    $parser->on(Llhttp\Events::MESSAGE_BEGIN, function() use (&$events) {
        $events[] = 'MESSAGE_BEGIN';
    });
    
    $parser->on(Llhttp\Events::URL, function($url) use (&$events) {
        $events[] = "URL: $url";
    });
    
    $parser->on(Llhttp\Events::HEADER_FIELD, function($field) use (&$events) {
        $events[] = "HEADER_FIELD: $field";
    });
    
    $parser->on(Llhttp\Events::HEADER_VALUE, function($value) use (&$events) {
        $events[] = "HEADER_VALUE: $value";
    });
    
    $parser->on(Llhttp\Events::HEADERS_COMPLETE, function() use (&$events) {
        $events[] = 'HEADERS_COMPLETE';
    });
    
    $parser->on(Llhttp\Events::MESSAGE_COMPLETE, function() use (&$events) {
        $events[] = 'MESSAGE_COMPLETE';
    });
    
    // Simple request with one header
    $request = "GET /test HTTP/1.1\r\nHost: example.com\r\n\r\n";
    
    echo "Request: " . json_encode($request) . "\n";
    
    $parser->execute($request);
    $parser->finish();
    
    echo "Events received:\n";
    foreach ($events as $event) {
        echo "  - $event\n";
    }
    
    echo "Test completed successfully!\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
}