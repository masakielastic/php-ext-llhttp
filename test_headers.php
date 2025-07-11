<?php

echo "Testing HTTP header parsing...\n";

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
    
    $parser->on(Llhttp\Events::BODY, function($chunk) use (&$events) {
        $events[] = "BODY: " . json_encode($chunk);
    });
    
    $parser->on(Llhttp\Events::MESSAGE_COMPLETE, function() use (&$events) {
        $events[] = 'MESSAGE_COMPLETE';
    });
    
    // Request with headers and body
    $request = "POST /api/test HTTP/1.1\r\n" .
               "Host: example.com\r\n" .
               "Content-Type: application/json\r\n" .
               "Content-Length: 13\r\n" .
               "\r\n" .
               '{"test":true}';
    
    echo "Request:\n" . $request . "\n\n";
    
    $parser->execute($request);
    $parser->finish();
    
    echo "Events received:\n";
    foreach ($events as $event) {
        echo "  - $event\n";
    }
    
    echo "\nParsed headers:\n";
    $headers = $parser->getHeaders();
    foreach ($headers as $name => $value) {
        echo "  $name: $value\n";
    }
    
    echo "\nParser info:\n";
    echo "  HTTP Version: " . $parser->getHttpMajor() . "." . $parser->getHttpMinor() . "\n";
    echo "  Method: " . $parser->getMethodName() . "\n";
    echo "  Keep-Alive: " . ($parser->shouldKeepAlive() ? 'YES' : 'NO') . "\n";
    
    echo "\nTest completed successfully!\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
    echo "Trace: " . $e->getTraceAsString() . "\n";
}