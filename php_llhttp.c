#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#include "php_llhttp.h"

/* Class entries */
zend_class_entry *llhttp_parser_ce;
zend_class_entry *llhttp_events_ce;
zend_class_entry *llhttp_error_codes_ce;
zend_class_entry *llhttp_exception_ce;

/* Object handlers */
zend_object_handlers llhttp_parser_object_handlers;

/* Globals */
ZEND_DECLARE_MODULE_GLOBALS(llhttp)

/* Parser object utility functions */
static inline llhttp_parser_object *llhttp_parser_object_from_zend_object(zend_object *obj) {
    return (llhttp_parser_object *)((char *)(obj) - XtOffsetOf(llhttp_parser_object, std));
}

static zend_object *llhttp_parser_object_create(zend_class_entry *ce) {
    llhttp_parser_object *intern = ecalloc(1, sizeof(llhttp_parser_object) + zend_object_properties_size(ce));
    
    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);
    
    /* Initialize parser object */
    intern->type = LLHTTP_TYPE_REQUEST;
    intern->paused = 0;
    intern->finished = 0;
    
    /* Initialize callbacks hash table */
    ALLOC_HASHTABLE(intern->callbacks);
    zend_hash_init(intern->callbacks, 8, NULL, ZVAL_PTR_DTOR, 0);
    
    /* Initialize headers hash table */
    ALLOC_HASHTABLE(intern->headers);
    zend_hash_init(intern->headers, 16, NULL, ZVAL_PTR_DTOR, 0);
    
    intern->current_header_field = NULL;
    intern->current_header_value = NULL;
    
    intern->std.handlers = &llhttp_parser_object_handlers;
    
    return &intern->std;
}

static void llhttp_parser_object_free(zend_object *obj) {
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(obj);
    
    /* Free callbacks hash table */
    if (intern->callbacks) {
        zend_hash_destroy(intern->callbacks);
        FREE_HASHTABLE(intern->callbacks);
    }
    
    /* Free headers hash table */
    if (intern->headers) {
        zend_hash_destroy(intern->headers);
        FREE_HASHTABLE(intern->headers);
    }
    
    /* Free current header strings */
    if (intern->current_header_field) {
        zend_string_release(intern->current_header_field);
    }
    if (intern->current_header_value) {
        zend_string_release(intern->current_header_value);
    }
    
    zend_object_std_dtor(obj);
}

/* Parser class methods */

/* __construct(int $type) */
PHP_METHOD(LlhttpParser, __construct) {
    zend_long type = LLHTTP_TYPE_REQUEST;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(type)
    ZEND_PARSE_PARAMETERS_END();
    
    if (type != LLHTTP_TYPE_REQUEST && type != LLHTTP_TYPE_RESPONSE) {
        zend_throw_exception(llhttp_exception_ce, "Invalid parser type. Use Llhttp\\Parser::TYPE_REQUEST or Llhttp\\Parser::TYPE_RESPONSE", 0);
        RETURN_THROWS();
    }
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    intern->type = (int)type;
    
    /* Initialize llhttp parser */
    llhttp_settings_init(&intern->settings);
    
    /* Set up callbacks */
    intern->settings.on_message_begin = llhttp_on_message_begin_cb;
    intern->settings.on_url = llhttp_on_url_cb;
    /* Note: llhttp doesn't have on_status callback, status is extracted from parser state */
    /* Enable header callbacks with event-only mode for now */
    intern->settings.on_header_field = llhttp_on_header_field_cb;
    intern->settings.on_header_value = llhttp_on_header_value_cb;
    intern->settings.on_headers_complete = llhttp_on_headers_complete_cb;
    intern->settings.on_body = llhttp_on_body_cb;
    intern->settings.on_message_complete = llhttp_on_message_complete_cb;
    
    llhttp_init(&intern->parser, (llhttp_type_t)type, &intern->settings);
    
    /* Set parser data to point to our object */
    intern->parser.data = intern;
}

/* on(string $event, callable $callback): Parser */
PHP_METHOD(LlhttpParser, on) {
    zend_string *event;
    zval *callback;
    
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(event)
        Z_PARAM_ZVAL(callback)
    ZEND_PARSE_PARAMETERS_END();
    
    if (!zend_is_callable(callback, 0, NULL)) {
        zend_throw_exception(llhttp_exception_ce, "Second parameter must be a valid callback", 0);
        RETURN_THROWS();
    }
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    
    /* Store callback in hash table */
    Z_TRY_ADDREF_P(callback);
    zend_hash_update(intern->callbacks, event, callback);
    
    RETURN_OBJ_COPY(Z_OBJ_P(ZEND_THIS));
}

/* off(string $event): Parser */
PHP_METHOD(LlhttpParser, off) {
    zend_string *event;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(event)
    ZEND_PARSE_PARAMETERS_END();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    
    /* Remove callback from hash table */
    zend_hash_del(intern->callbacks, event);
    
    RETURN_OBJ_COPY(Z_OBJ_P(ZEND_THIS));
}

/* execute(string $data): void */
PHP_METHOD(LlhttpParser, execute) {
    zend_string *data;
    
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    
    if (intern->finished) {
        zend_throw_exception(llhttp_exception_ce, "Parser has already finished", 0);
        RETURN_THROWS();
    }
    
    /* Execute parser */
    llhttp_errno_t err = llhttp_execute(&intern->parser, ZSTR_VAL(data), ZSTR_LEN(data));
    
    if (err != HPE_OK && err != HPE_PAUSED) {
        const char *error_reason = llhttp_get_error_reason(&intern->parser);
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Parse error: %s", error_reason ? error_reason : "Unknown error");
        zend_throw_exception(llhttp_exception_ce, error_msg, err);
        RETURN_THROWS();
    }
    
    if (err == HPE_PAUSED) {
        intern->paused = 1;
    }
}

/* finish(): void */
PHP_METHOD(LlhttpParser, finish) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    
    if (intern->finished) {
        return;
    }
    
    llhttp_errno_t err = llhttp_finish(&intern->parser);
    
    if (err != HPE_OK) {
        const char *error_reason = llhttp_get_error_reason(&intern->parser);
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Finish error: %s", error_reason ? error_reason : "Unknown error");
        zend_throw_exception(llhttp_exception_ce, error_msg, err);
        RETURN_THROWS();
    }
    
    intern->finished = 1;
}

/* pause(): void */
PHP_METHOD(LlhttpParser, pause) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    intern->paused = 1;
    llhttp_pause(&intern->parser);
}

/* resume(): void */
PHP_METHOD(LlhttpParser, resume) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    
    if (intern->paused) {
        llhttp_resume(&intern->parser);
        intern->paused = 0;
    }
}

/* reset(): void */
PHP_METHOD(LlhttpParser, reset) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    
    /* Reset parser state */
    llhttp_reset(&intern->parser);
    intern->paused = 0;
    intern->finished = 0;
    
    /* Clear headers */
    zend_hash_clean(intern->headers);
    
    /* Clear current header strings */
    if (intern->current_header_field) {
        zend_string_release(intern->current_header_field);
        intern->current_header_field = NULL;
    }
    if (intern->current_header_value) {
        zend_string_release(intern->current_header_value);
        intern->current_header_value = NULL;
    }
}

/* isPaused(): bool */
PHP_METHOD(LlhttpParser, isPaused) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    RETURN_BOOL(intern->paused);
}

/* getType(): int */
PHP_METHOD(LlhttpParser, getType) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    RETURN_LONG(intern->type);
}

/* getHttpMajor(): int */
PHP_METHOD(LlhttpParser, getHttpMajor) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    RETURN_LONG(llhttp_get_http_major(&intern->parser));
}

/* getHttpMinor(): int */
PHP_METHOD(LlhttpParser, getHttpMinor) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    RETURN_LONG(llhttp_get_http_minor(&intern->parser));
}

/* getMethod(): int */
PHP_METHOD(LlhttpParser, getMethod) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    RETURN_LONG(llhttp_get_method(&intern->parser));
}

/* getMethodName(): string */
PHP_METHOD(LlhttpParser, getMethodName) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    uint8_t method = llhttp_get_method(&intern->parser);
    const char *method_name = llhttp_method_name((llhttp_method_t)method);
    
    RETURN_STRING(method_name);
}

/* getStatusCode(): int */
PHP_METHOD(LlhttpParser, getStatusCode) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    RETURN_LONG(llhttp_get_status_code(&intern->parser));
}

/* shouldKeepAlive(): bool */
PHP_METHOD(LlhttpParser, shouldKeepAlive) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    RETURN_BOOL(llhttp_should_keep_alive(&intern->parser));
}

/* messageNeedsEof(): bool */
PHP_METHOD(LlhttpParser, messageNeedsEof) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    RETURN_BOOL(llhttp_message_needs_eof(&intern->parser));
}

/* getHeaders(): array */
PHP_METHOD(LlhttpParser, getHeaders) {
    ZEND_PARSE_PARAMETERS_NONE();
    
    llhttp_parser_object *intern = llhttp_parser_object_from_zend_object(Z_OBJ_P(ZEND_THIS));
    
    /* Return copy of headers array */
    array_init(return_value);
    
    zend_string *key;
    zval *val;
    ZEND_HASH_FOREACH_STR_KEY_VAL(intern->headers, key, val) {
        Z_TRY_ADDREF_P(val);
        zend_hash_update(Z_ARRVAL_P(return_value), key, val);
    } ZEND_HASH_FOREACH_END();
}

/* Method entries for Parser class */
const zend_function_entry llhttp_parser_methods[] = {
    PHP_ME(LlhttpParser, __construct,       NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, on,                NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, off,               NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, execute,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, finish,            NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, pause,             NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, resume,            NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, reset,             NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, isPaused,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, getType,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, getHttpMajor,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, getHttpMinor,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, getMethod,         NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, getMethodName,     NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, getStatusCode,     NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, shouldKeepAlive,   NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, messageNeedsEof,   NULL, ZEND_ACC_PUBLIC)
    PHP_ME(LlhttpParser, getHeaders,        NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* External declarations for method entries */
extern const zend_function_entry llhttp_events_methods[];
extern const zend_function_entry llhttp_error_codes_methods[];

/* Module initialization */
PHP_MINIT_FUNCTION(llhttp) {
    zend_class_entry ce;
    
    /* Register Parser class */
    INIT_CLASS_ENTRY(ce, "Llhttp\\Parser", llhttp_parser_methods);
    llhttp_parser_ce = zend_register_internal_class(&ce);
    llhttp_parser_ce->create_object = llhttp_parser_object_create;
    
    /* Set up object handlers */
    memcpy(&llhttp_parser_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    llhttp_parser_object_handlers.offset = XtOffsetOf(llhttp_parser_object, std);
    llhttp_parser_object_handlers.free_obj = llhttp_parser_object_free;
    
    /* Register constants */
    zend_declare_class_constant_long(llhttp_parser_ce, "TYPE_REQUEST", sizeof("TYPE_REQUEST")-1, LLHTTP_TYPE_REQUEST);
    zend_declare_class_constant_long(llhttp_parser_ce, "TYPE_RESPONSE", sizeof("TYPE_RESPONSE")-1, LLHTTP_TYPE_RESPONSE);
    
    /* Register Events class */
    INIT_CLASS_ENTRY(ce, "Llhttp\\Events", llhttp_events_methods);
    llhttp_events_ce = zend_register_internal_class(&ce);
    
    /* Register event constants */
    zend_declare_class_constant_string(llhttp_events_ce, "MESSAGE_BEGIN", sizeof("MESSAGE_BEGIN")-1, LLHTTP_EVENT_MESSAGE_BEGIN);
    zend_declare_class_constant_string(llhttp_events_ce, "URL", sizeof("URL")-1, LLHTTP_EVENT_URL);
    zend_declare_class_constant_string(llhttp_events_ce, "STATUS", sizeof("STATUS")-1, LLHTTP_EVENT_STATUS);
    zend_declare_class_constant_string(llhttp_events_ce, "HEADER_FIELD", sizeof("HEADER_FIELD")-1, LLHTTP_EVENT_HEADER_FIELD);
    zend_declare_class_constant_string(llhttp_events_ce, "HEADER_VALUE", sizeof("HEADER_VALUE")-1, LLHTTP_EVENT_HEADER_VALUE);
    zend_declare_class_constant_string(llhttp_events_ce, "HEADERS_COMPLETE", sizeof("HEADERS_COMPLETE")-1, LLHTTP_EVENT_HEADERS_COMPLETE);
    zend_declare_class_constant_string(llhttp_events_ce, "BODY", sizeof("BODY")-1, LLHTTP_EVENT_BODY);
    zend_declare_class_constant_string(llhttp_events_ce, "MESSAGE_COMPLETE", sizeof("MESSAGE_COMPLETE")-1, LLHTTP_EVENT_MESSAGE_COMPLETE);
    
    /* Register ErrorCodes class */
    INIT_CLASS_ENTRY(ce, "Llhttp\\ErrorCodes", llhttp_error_codes_methods);
    llhttp_error_codes_ce = zend_register_internal_class(&ce);
    
    /* Register error code constants */
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_OK", sizeof("HPE_OK")-1, HPE_OK);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INTERNAL", sizeof("HPE_INTERNAL")-1, HPE_INTERNAL);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_STRICT", sizeof("HPE_STRICT")-1, HPE_STRICT);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_LF_EXPECTED", sizeof("HPE_LF_EXPECTED")-1, HPE_LF_EXPECTED);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_UNEXPECTED_CONTENT_LENGTH", sizeof("HPE_UNEXPECTED_CONTENT_LENGTH")-1, HPE_UNEXPECTED_CONTENT_LENGTH);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_CLOSED_CONNECTION", sizeof("HPE_CLOSED_CONNECTION")-1, HPE_CLOSED_CONNECTION);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_METHOD", sizeof("HPE_INVALID_METHOD")-1, HPE_INVALID_METHOD);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_URL", sizeof("HPE_INVALID_URL")-1, HPE_INVALID_URL);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_CONSTANT", sizeof("HPE_INVALID_CONSTANT")-1, HPE_INVALID_CONSTANT);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_VERSION", sizeof("HPE_INVALID_VERSION")-1, HPE_INVALID_VERSION);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_HEADER_TOKEN", sizeof("HPE_INVALID_HEADER_TOKEN")-1, HPE_INVALID_HEADER_TOKEN);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_CONTENT_LENGTH", sizeof("HPE_INVALID_CONTENT_LENGTH")-1, HPE_INVALID_CONTENT_LENGTH);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_CHUNK_SIZE", sizeof("HPE_INVALID_CHUNK_SIZE")-1, HPE_INVALID_CHUNK_SIZE);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_STATUS", sizeof("HPE_INVALID_STATUS")-1, HPE_INVALID_STATUS);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_INVALID_EOF_STATE", sizeof("HPE_INVALID_EOF_STATE")-1, HPE_INVALID_EOF_STATE);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_PAUSED", sizeof("HPE_PAUSED")-1, HPE_PAUSED);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_PAUSED_UPGRADE", sizeof("HPE_PAUSED_UPGRADE")-1, HPE_PAUSED_UPGRADE);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_PAUSED_H2_UPGRADE", sizeof("HPE_PAUSED_H2_UPGRADE")-1, HPE_PAUSED_H2_UPGRADE);
    zend_declare_class_constant_long(llhttp_error_codes_ce, "HPE_USER", sizeof("HPE_USER")-1, HPE_USER);
    
    /* Register Exception class */
    INIT_CLASS_ENTRY(ce, "Llhttp\\Exception", NULL);
    llhttp_exception_ce = zend_register_internal_class_ex(&ce, zend_ce_exception);
    
    return SUCCESS;
}

/* Module shutdown */
PHP_MSHUTDOWN_FUNCTION(llhttp) {
    return SUCCESS;
}

/* Module info */
PHP_MINFO_FUNCTION(llhttp) {
    php_info_print_table_start();
    php_info_print_table_header(2, "llhttp support", "enabled");
    php_info_print_table_row(2, "Extension version", PHP_LLHTTP_VERSION);
    php_info_print_table_end();
}

/* Module entry */
zend_module_entry llhttp_module_entry = {
    STANDARD_MODULE_HEADER,
    "llhttp",
    NULL,                       /* function entries */
    PHP_MINIT(llhttp),
    PHP_MSHUTDOWN(llhttp),
    NULL,                       /* request startup */
    NULL,                       /* request shutdown */
    PHP_MINFO(llhttp),
    PHP_LLHTTP_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_LLHTTP
ZEND_GET_MODULE(llhttp)
#endif