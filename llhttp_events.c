#include "php_llhttp.h"

/* Events class methods */

/* getAll(): array */
PHP_METHOD(LlhttpEvents, getAll) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    array_init(return_value);
    
    add_next_index_string(return_value, LLHTTP_EVENT_MESSAGE_BEGIN);
    add_next_index_string(return_value, LLHTTP_EVENT_URL);
    add_next_index_string(return_value, LLHTTP_EVENT_STATUS);
    add_next_index_string(return_value, LLHTTP_EVENT_HEADER_FIELD);
    add_next_index_string(return_value, LLHTTP_EVENT_HEADER_VALUE);
    add_next_index_string(return_value, LLHTTP_EVENT_HEADERS_COMPLETE);
    add_next_index_string(return_value, LLHTTP_EVENT_BODY);
    add_next_index_string(return_value, LLHTTP_EVENT_MESSAGE_COMPLETE);
}

/* isValid(string $event): bool */
PHP_METHOD(LlhttpEvents, isValid) {
    zend_string *event;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(event)
    ZEND_PARSE_PARAMETERS_END();
    
    const char *event_cstr = ZSTR_VAL(event);
    
    if (strcmp(event_cstr, LLHTTP_EVENT_MESSAGE_BEGIN) == 0 ||
        strcmp(event_cstr, LLHTTP_EVENT_URL) == 0 ||
        strcmp(event_cstr, LLHTTP_EVENT_STATUS) == 0 ||
        strcmp(event_cstr, LLHTTP_EVENT_HEADER_FIELD) == 0 ||
        strcmp(event_cstr, LLHTTP_EVENT_HEADER_VALUE) == 0 ||
        strcmp(event_cstr, LLHTTP_EVENT_HEADERS_COMPLETE) == 0 ||
        strcmp(event_cstr, LLHTTP_EVENT_BODY) == 0 ||
        strcmp(event_cstr, LLHTTP_EVENT_MESSAGE_COMPLETE) == 0) {
        RETURN_TRUE;
    }
    
    RETURN_FALSE;
}

/* Method entries for Events class */
const zend_function_entry llhttp_events_methods[] = {
    PHP_ME(LlhttpEvents, getAll,    NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(LlhttpEvents, isValid,   NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};