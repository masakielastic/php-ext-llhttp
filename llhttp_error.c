#include "php_llhttp.h"

/* ErrorCodes class methods */

/* getMessage(int $code): string */
PHP_METHOD(LlhttpErrorCodes, getMessage) {
    zend_long code;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END();
    
    const char *message;
    
    switch (code) {
        case HPE_OK:
            message = "Success";
            break;
        case HPE_INTERNAL:
            message = "Internal parser error";
            break;
        case HPE_STRICT:
            message = "Strict mode assertion failed";
            break;
        case HPE_LF_EXPECTED:
            message = "Expected LF after CR";
            break;
        case HPE_UNEXPECTED_CONTENT_LENGTH:
            message = "Unexpected content-length header";
            break;
        case HPE_CLOSED_CONNECTION:
            message = "Connection closed before message completed";
            break;
        case HPE_INVALID_METHOD:
            message = "Invalid HTTP method";
            break;
        case HPE_INVALID_URL:
            message = "Invalid URL";
            break;
        case HPE_INVALID_CONSTANT:
            message = "Invalid constant string";
            break;
        case HPE_INVALID_VERSION:
            message = "Invalid HTTP version";
            break;
        case HPE_INVALID_HEADER_TOKEN:
            message = "Invalid header token";
            break;
        case HPE_INVALID_CONTENT_LENGTH:
            message = "Invalid content-length value";
            break;
        case HPE_INVALID_CHUNK_SIZE:
            message = "Invalid chunk size";
            break;
        case HPE_INVALID_STATUS:
            message = "Invalid status";
            break;
        case HPE_INVALID_EOF_STATE:
            message = "Invalid EOF state";
            break;
        case HPE_PAUSED:
            message = "Parser is paused";
            break;
        case HPE_PAUSED_UPGRADE:
            message = "Parser is paused on upgrade";
            break;
        case HPE_PAUSED_H2_UPGRADE:
            message = "Parser is paused on H2 upgrade";
            break;
        case HPE_USER:
            message = "User callback error";
            break;
        default:
            message = "Unknown error";
            break;
    }
    
    RETURN_STRING(message);
}

/* Arginfo for ErrorCodes */
ZEND_BEGIN_ARG_INFO_EX(arginfo_llhttp_error_codes_getMessage, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, code, IS_LONG, 0)
ZEND_END_ARG_INFO()

/* Method entries for ErrorCodes class */
const zend_function_entry llhttp_error_codes_methods[] = {
    PHP_ME(LlhttpErrorCodes, getMessage, arginfo_llhttp_error_codes_getMessage, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

/* Error handling helper */
void llhttp_throw_exception(int llhttp_errno, const char *message) {
    zval ex;
    
    object_init_ex(&ex, llhttp_exception_ce);
    
    /* Set message */
    zend_update_property_string(llhttp_exception_ce, Z_OBJ(ex), "message", sizeof("message")-1, message);
    
    /* Set code */
    zend_update_property_long(llhttp_exception_ce, Z_OBJ(ex), "code", sizeof("code")-1, llhttp_errno);
    
    zend_throw_exception_object(&ex);
}