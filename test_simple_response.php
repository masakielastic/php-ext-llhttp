<?php

echo "Testing simple HTTP response parsing...\n";

try {
    $parser = new Llhttp\Parser(Llhttp\Parser::TYPE_RESPONSE);
    
    $events = [];
    
    $parser->on(Llhttp\Events::MESSAGE_BEGIN, function() use (&$events) {
        $events[] = 'MESSAGE_BEGIN';
    });
    
    $parser->on(Llhttp\Events::STATUS, function($status) use (&$events) {
        $events[] = "STATUS: $status";
    });
    
    $parser->on(Llhttp\Events::HEADERS_COMPLETE, function() use (&$events) {
        $events[] = 'HEADERS_COMPLETE';
    });
    
    $parser->on(Llhttp\Events::MESSAGE_COMPLETE, function() use (&$events) {
        $events[] = 'MESSAGE_COMPLETE';
    });
    
    // Very simple HTTP response without body
    $response = "HTTP/1.1 200 OK\r\n\r\n";
    
    echo "Response: " . json_encode($response) . "\n";
    
    $parser->execute($response);
    $parser->finish();
    
    echo "Events received:\n";
    foreach ($events as $event) {
        echo "  - $event\n";
    }
    
    echo "Status Code: " . $parser->getStatusCode() . "\n";
    echo "HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
    
    echo "Test completed successfully!\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
}