#ifndef PHP_LLHTTP_H
#define PHP_LLHTTP_H

/* Include PHP headers first */
#include "php.h"
#include "zend_exceptions.h"

extern zend_module_entry llhttp_module_entry;
#define phpext_llhttp_ptr &llhttp_module_entry

#define PHP_LLHTTP_VERSION "1.0.0"

#ifdef PHP_WIN32
#   define PHP_LLHTTP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_LLHTTP_API __attribute__ ((visibility("default")))
#else
#   define PHP_LLHTTP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* Include llhttp library headers */
#include "llhttp.h"

/* Forward declarations */
typedef struct _llhttp_parser_object llhttp_parser_object;
typedef struct _llhttp_callback_data llhttp_callback_data;

/* Parser object structure */
struct _llhttp_parser_object {
    llhttp_t parser;
    llhttp_settings_t settings;
    zend_object std;
    
    /* Parser type (request/response) */
    int type;
    
    /* Header collection */
    HashTable *headers;
    zend_string *current_header_field;
    zend_string *current_header_value;
    
    /* Data storage */
    zend_string *url;
    zend_string *body;
    
    /* State management */
    int state;
    zend_bool finished;
};

/* Callback data structure */
struct _llhttp_callback_data {
    llhttp_parser_object *parser_obj;
    const char *data;
    size_t length;
};

/* Class entry declarations */
extern zend_class_entry *llhttp_parser_ce;
extern zend_class_entry *llhttp_error_codes_ce;
extern zend_class_entry *llhttp_exception_ce;

/* Object handlers */
extern zend_object_handlers llhttp_parser_object_handlers;

/* Parser type constants - must match llhttp library values */
#define LLHTTP_TYPE_BOTH     0  /* HTTP_BOTH */
#define LLHTTP_TYPE_REQUEST  1  /* HTTP_REQUEST */
#define LLHTTP_TYPE_RESPONSE 2  /* HTTP_RESPONSE */

/* Parser state constants */
#define LLHTTP_STATE_INIT        0
#define LLHTTP_STATE_PARSING     1
#define LLHTTP_STATE_COMPLETE    2
#define LLHTTP_STATE_ERROR       3

/* Function declarations */

/* Module functions */
PHP_MINIT_FUNCTION(llhttp);
PHP_MSHUTDOWN_FUNCTION(llhttp);
PHP_MINFO_FUNCTION(llhttp);

/* Parser class methods */
PHP_METHOD(LlhttpParser, __construct);
PHP_METHOD(LlhttpParser, parse);
PHP_METHOD(LlhttpParser, parseComplete);
PHP_METHOD(LlhttpParser, reset);
PHP_METHOD(LlhttpParser, getHttpMajor);
PHP_METHOD(LlhttpParser, getHttpMinor);
PHP_METHOD(LlhttpParser, getMethod);
PHP_METHOD(LlhttpParser, getMethodName);
PHP_METHOD(LlhttpParser, getStatusCode);
PHP_METHOD(LlhttpParser, getUrl);
PHP_METHOD(LlhttpParser, getHeaders);
PHP_METHOD(LlhttpParser, getHeader);
PHP_METHOD(LlhttpParser, getBody);
PHP_METHOD(LlhttpParser, shouldKeepAlive);
PHP_METHOD(LlhttpParser, messageNeedsEof);
PHP_METHOD(LlhttpParser, isComplete);
PHP_METHOD(LlhttpParser, getState);

/* ErrorCodes class methods */
PHP_METHOD(LlhttpErrorCodes, getMessage);

/* Utility functions - declared static inline or static in .c file */

/* Callback functions */
int llhttp_on_message_begin_cb(llhttp_t *parser);
int llhttp_on_url_cb(llhttp_t *parser, const char *at, size_t length);
int llhttp_on_status_cb(llhttp_t *parser, const char *at, size_t length);
int llhttp_on_header_field_cb(llhttp_t *parser, const char *at, size_t length);
int llhttp_on_header_value_cb(llhttp_t *parser, const char *at, size_t length);
int llhttp_on_headers_complete_cb(llhttp_t *parser);
int llhttp_on_body_cb(llhttp_t *parser, const char *at, size_t length);
int llhttp_on_message_complete_cb(llhttp_t *parser);

/* Helper functions */
void llhttp_add_header(llhttp_parser_object *parser_obj, zend_string *field, zend_string *value);
void llhttp_finalize_current_header(llhttp_parser_object *parser_obj);
void llhttp_append_url(llhttp_parser_object *parser_obj, const char *at, size_t length);
void llhttp_append_body(llhttp_parser_object *parser_obj, const char *at, size_t length);

/* Error handling */
void llhttp_throw_exception(int llhttp_errno, const char *message);

/* Globals structure */
ZEND_BEGIN_MODULE_GLOBALS(llhttp)
    /* Add global variables if needed */
ZEND_END_MODULE_GLOBALS(llhttp)

#ifdef ZTS
#define LLHTTP_G(v) TSRMG(llhttp_globals_id, zend_llhttp_globals *, v)
#else
#define LLHTTP_G(v) (llhttp_globals.v)
#endif

#endif /* PHP_LLHTTP_H */