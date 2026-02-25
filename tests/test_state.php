<?php

echo "Testing state management methods...\n";

try {
    $parser = new Llhttp\Parser(Llhttp\Parser::TYPE_REQUEST);
    
    echo "Initial state:\n";
    echo "  Is paused: " . ($parser->isPaused() ? 'YES' : 'NO') . "\n";
    echo "  Type: " . $parser->getType() . "\n";
    
    // Test pause/resume
    echo "\nTesting pause/resume:\n";
    $parser->pause();
    echo "  After pause - Is paused: " . ($parser->isPaused() ? 'YES' : 'NO') . "\n";
    
    $parser->resume();
    echo "  After resume - Is paused: " . ($parser->isPaused() ? 'YES' : 'NO') . "\n";
    
    // Test reset
    echo "\nTesting reset:\n";
    $parser->pause();
    echo "  Before reset - Is paused: " . ($parser->isPaused() ? 'YES' : 'NO') . "\n";
    
    $parser->reset();
    echo "  After reset - Is paused: " . ($parser->isPaused() ? 'YES' : 'NO') . "\n";
    
    // Test basic parsing
    echo "\nTesting basic parsing after reset:\n";
    $events = [];
    
    $parser->on(Llhttp\Events::MESSAGE_BEGIN, function() use (&$events) {
        $events[] = 'MESSAGE_BEGIN';
    });
    
    $parser->on(Llhttp\Events::URL, function($url) use (&$events) {
        $events[] = "URL: $url";
    });
    
    $parser->on(Llhttp\Events::MESSAGE_COMPLETE, function() use (&$events) {
        $events[] = 'MESSAGE_COMPLETE';
    });
    
    $request = "GET /reset-test HTTP/1.1\r\n\r\n";
    $parser->execute($request);
    $parser->finish();
    
    echo "  Events: " . implode(', ', $events) . "\n";
    echo "  Method: " . $parser->getMethodName() . "\n";
    
    echo "\nState management test completed successfully!\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
}