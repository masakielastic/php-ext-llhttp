#include "php_llhttp.h"

/* Callback helper function */
void llhttp_call_user_callback(llhttp_parser_object *parser_obj, const char *event_name, zval *args, int arg_count) {
    zval *callback;
    zend_string *event_key;
    
    if (!parser_obj || !parser_obj->callbacks || !event_name) {
        return;
    }
    
    event_key = zend_string_init(event_name, strlen(event_name), 0);
    callback = zend_hash_find(parser_obj->callbacks, event_key);
    zend_string_release(event_key);
    
    if (callback && Z_TYPE_P(callback) != IS_NULL && zend_is_callable(callback, 0, NULL)) {
        zval retval;
        zend_result result;
        
        ZVAL_UNDEF(&retval);
        result = call_user_function(NULL, NULL, callback, &retval, arg_count, args);
        
        if (result == SUCCESS && !Z_ISUNDEF(retval)) {
            zval_ptr_dtor(&retval);
        }
    }
}

/* Header management functions */
void llhttp_add_header(llhttp_parser_object *parser_obj, zend_string *field, zend_string *value) {
    zend_string *lower_field;
    zval header_val;
    
    if (!parser_obj || !parser_obj->headers || !field || !value) {
        return;
    }
    
    /* Simple approach: last header wins, no duplicate handling */
    lower_field = zend_string_tolower(field);
    ZVAL_STR_COPY(&header_val, value);
    zend_hash_update(parser_obj->headers, lower_field, &header_val);
    zend_string_release(lower_field);
}

void llhttp_finalize_current_header(llhttp_parser_object *parser_obj) {
    if (!parser_obj) {
        return;
    }
    
    if (parser_obj->current_header_field && parser_obj->current_header_value) {
        llhttp_add_header(parser_obj, parser_obj->current_header_field, parser_obj->current_header_value);
        
        zend_string_release(parser_obj->current_header_field);
        zend_string_release(parser_obj->current_header_value);
        parser_obj->current_header_field = NULL;
        parser_obj->current_header_value = NULL;
    }
}

/* llhttp callback functions */

int llhttp_on_message_begin_cb(llhttp_t *parser) {
    llhttp_parser_object *parser_obj;
    
    if (!parser || !parser->data) {
        return 0;
    }
    
    parser_obj = (llhttp_parser_object *)parser->data;
    llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_MESSAGE_BEGIN, NULL, 0);
    
    return 0;
}

int llhttp_on_url_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    zval args[1];
    
    ZVAL_STRINGL(&args[0], at, length);
    llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_URL, args, 1);
    zval_ptr_dtor(&args[0]);
    
    return 0;
}

int llhttp_on_status_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    zval args[1];
    
    ZVAL_STRINGL(&args[0], at, length);
    llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_STATUS, args, 1);
    zval_ptr_dtor(&args[0]);
    
    return 0;
}

int llhttp_on_header_field_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    zval args[1];
    
    if (!parser_obj || !at || length == 0) {
        return 0;
    }
    
    /* Store current header field for getHeaders() */
    if (parser_obj->current_header_field) {
        zend_string_release(parser_obj->current_header_field);
    }
    parser_obj->current_header_field = zend_string_init(at, length, 0);
    
    /* Emit event */
    ZVAL_STRINGL(&args[0], at, length);
    llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_HEADER_FIELD, args, 1);
    zval_ptr_dtor(&args[0]);
    
    return 0;
}

int llhttp_on_header_value_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    zval args[1];
    
    if (!parser_obj || !at || length == 0) {
        return 0;
    }
    
    /* Store header value and add to collection if we have a field */
    if (parser_obj->current_header_field) {
        zend_string *value = zend_string_init(at, length, 0);
        zend_string *lower_field = zend_string_tolower(parser_obj->current_header_field);
        
        /* Simple header storage - last value wins */
        zval header_val;
        ZVAL_STR_COPY(&header_val, value);
        zend_hash_update(parser_obj->headers, lower_field, &header_val);
        
        zend_string_release(value);
        zend_string_release(lower_field);
        zend_string_release(parser_obj->current_header_field);
        parser_obj->current_header_field = NULL;
    }
    
    /* Emit event */
    ZVAL_STRINGL(&args[0], at, length);
    llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_HEADER_VALUE, args, 1);
    zval_ptr_dtor(&args[0]);
    
    return 0;
}

int llhttp_on_headers_complete_cb(llhttp_t *parser) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    
    if (!parser_obj) {
        return 0;
    }
    
    /* For response parsers, emit STATUS event with status code */
    if (parser_obj->type == LLHTTP_TYPE_RESPONSE) {
        zval args[1];
        char status_str[16];
        int status_code = llhttp_get_status_code(parser);
        
        snprintf(status_str, sizeof(status_str), "%d", status_code);
        ZVAL_STRING(&args[0], status_str);
        llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_STATUS, args, 1);
        zval_ptr_dtor(&args[0]);
    }
    
    llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_HEADERS_COMPLETE, NULL, 0);
    
    return 0;
}

int llhttp_on_body_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    zval args[1];
    
    ZVAL_STRINGL(&args[0], at, length);
    llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_BODY, args, 1);
    zval_ptr_dtor(&args[0]);
    
    return 0;
}

int llhttp_on_message_complete_cb(llhttp_t *parser) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    
    llhttp_call_user_callback(parser_obj, LLHTTP_EVENT_MESSAGE_COMPLETE, NULL, 0);
    
    return 0;
}