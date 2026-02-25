#include "php_llhttp.h"

/* This function is no longer needed in the new API */

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
    /* No event emission needed in new API */
    return 0;
}

int llhttp_on_url_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    
    if (!parser_obj || !at || length == 0) {
        return 0;
    }
    
    /* Store URL data for getUrl() */
    llhttp_append_url(parser_obj, at, length);
    
    return 0;
}

int llhttp_on_status_cb(llhttp_t *parser, const char *at, size_t length) {
    /* Status text not needed in new API - use getStatusCode() instead */
    return 0;
}

int llhttp_on_header_field_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    
    if (!parser_obj || !at || length == 0) {
        return 0;
    }
    
    /* Store current header field for getHeaders() */
    if (parser_obj->current_header_field) {
        zend_string_release(parser_obj->current_header_field);
    }
    parser_obj->current_header_field = zend_string_init(at, length, 0);
    
    return 0;
}

int llhttp_on_header_value_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    
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
    
    return 0;
}

int llhttp_on_headers_complete_cb(llhttp_t *parser) {
    /* Headers are collected automatically - no event needed */
    return 0;
}

int llhttp_on_body_cb(llhttp_t *parser, const char *at, size_t length) {
    llhttp_parser_object *parser_obj = (llhttp_parser_object *)parser->data;
    
    if (!parser_obj || !at || length == 0) {
        return 0;
    }
    
    /* Store body data for getBody() */
    llhttp_append_body(parser_obj, at, length);
    
    return 0;
}

int llhttp_on_message_complete_cb(llhttp_t *parser) {
    /* Message completion handled by state management - no event needed */
    return 0;
}