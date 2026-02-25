<?php

echo "Testing HTTP response parsing...\n";

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
    
    $parser->on(Llhttp\Events::BODY, function($chunk) use (&$events) {
        $events[] = "BODY: " . json_encode($chunk);
    });
    
    $parser->on(Llhttp\Events::MESSAGE_COMPLETE, function() use (&$events) {
        $events[] = 'MESSAGE_COMPLETE';
    });
    
    // Simple HTTP response
    $response = "HTTP/1.1 200 OK\r\n" .
                "Content-Length: 5\r\n" .
                "\r\n" .
                "Hello";
    
    echo "Response:\n" . $response . "\n\n";
    
    $parser->execute($response);
    $parser->finish();
    
    echo "Events received:\n";
    foreach ($events as $event) {
        echo "  - $event\n";
    }
    
    echo "\nParser info:\n";
    echo "  HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
    echo "  Status Code: " . $parser->getStatusCode() . "\n";
    echo "  Keep-Alive: " . ($parser->shouldKeepAlive() ? 'YES' : 'NO') . "\n";
    
    echo "\nResponse parsing test completed successfully!\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
}