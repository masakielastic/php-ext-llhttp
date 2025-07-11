<?php

echo "Testing simple HTTP parsing...\n";

try {
    $parser = new Llhttp\Parser(Llhttp\Parser::TYPE_REQUEST);
    
    $events = [];
    
    $parser->on(Llhttp\Events::MESSAGE_BEGIN, function() use (&$events) {
        $events[] = 'MESSAGE_BEGIN';
    });
    
    $parser->on(Llhttp\Events::URL, function($url) use (&$events) {
        $events[] = "URL: $url";
    });
    
    $parser->on(Llhttp\Events::HEADERS_COMPLETE, function() use (&$events) {
        $events[] = 'HEADERS_COMPLETE';
    });
    
    $parser->on(Llhttp\Events::MESSAGE_COMPLETE, function() use (&$events) {
        $events[] = 'MESSAGE_COMPLETE';
    });
    
    // Very simple request
    $request = "GET /test HTTP/1.1\r\n\r\n";
    
    echo "Request: " . json_encode($request) . "\n";
    
    $parser->execute($request);
    $parser->finish();
    
    echo "Events received:\n";
    foreach ($events as $event) {
        echo "  - $event\n";
    }
    
    echo "Parser info:\n";
    echo "  HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
    echo "  Method: " . $parser->getMethodName() . "\n";
    
    echo "Test completed successfully!\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
}