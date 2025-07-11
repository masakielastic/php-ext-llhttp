<?php

try {
    echo "Testing basic llhttp extension functionality\n";
    
    // Test class existence
    echo "Parser class exists: " . (class_exists('Llhttp\\Parser') ? 'YES' : 'NO') . "\n";
    echo "Events class exists: " . (class_exists('Llhttp\\Events') ? 'YES' : 'NO') . "\n";
    echo "ErrorCodes class exists: " . (class_exists('Llhttp\\ErrorCodes') ? 'YES' : 'NO') . "\n";
    echo "Exception class exists: " . (class_exists('Llhttp\\Exception') ? 'YES' : 'NO') . "\n";
    
    // Test constants
    echo "\nParser Constants:\n";
    echo "TYPE_REQUEST: " . Llhttp\Parser::TYPE_REQUEST . "\n";
    echo "TYPE_RESPONSE: " . Llhttp\Parser::TYPE_RESPONSE . "\n";
    
    // Test Parser instantiation
    echo "\nCreating Parser instance...\n";
    $parser = new Llhttp\Parser(Llhttp\Parser::TYPE_REQUEST);
    echo "Parser created successfully!\n";
    echo "Parser type: " . $parser->getType() . "\n";
    echo "Is paused: " . ($parser->isPaused() ? 'YES' : 'NO') . "\n";
    
    echo "\nBasic test completed successfully!\n";
    
} catch (Exception $e) {
    echo "Error: " . $e->getMessage() . "\n";
    echo "Code: " . $e->getCode() . "\n";
}