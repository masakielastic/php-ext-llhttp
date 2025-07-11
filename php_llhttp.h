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
    
    /* Callback storage */
    HashTable *callbacks;
    
    /* Header collection */
    HashTable *headers;
    zend_string *current_header_field;
    zend_string *current_header_value;
    
    /* State management */
    zend_bool paused;
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
extern zend_class_entry *llhttp_events_ce;
extern zend_class_entry *llhttp_error_codes_ce;
extern zend_class_entry *llhttp_exception_ce;

/* Object handlers */
extern zend_object_handlers llhttp_parser_object_handlers;

/* Parser type constants - must match llhttp library values */
#define LLHTTP_TYPE_REQUEST  1  /* HTTP_REQUEST */
#define LLHTTP_TYPE_RESPONSE 2  /* HTTP_RESPONSE */

/* Event name constants */
#define LLHTTP_EVENT_MESSAGE_BEGIN     "messageBegin"
#define LLHTTP_EVENT_URL               "url"
#define LLHTTP_EVENT_STATUS            "status"
#define LLHTTP_EVENT_HEADER_FIELD      "headerField"
#define LLHTTP_EVENT_HEADER_VALUE      "headerValue"
#define LLHTTP_EVENT_HEADERS_COMPLETE  "headersComplete"
#define LLHTTP_EVENT_BODY              "body"
#define LLHTTP_EVENT_MESSAGE_COMPLETE  "messageComplete"

/* Function declarations */

/* Module functions */
PHP_MINIT_FUNCTION(llhttp);
PHP_MSHUTDOWN_FUNCTION(llhttp);
PHP_MINFO_FUNCTION(llhttp);

/* Parser class methods */
PHP_METHOD(LlhttpParser, __construct);
PHP_METHOD(LlhttpParser, on);
PHP_METHOD(LlhttpParser, off);
PHP_METHOD(LlhttpParser, execute);
PHP_METHOD(LlhttpParser, finish);
PHP_METHOD(LlhttpParser, pause);
PHP_METHOD(LlhttpParser, resume);
PHP_METHOD(LlhttpParser, reset);
PHP_METHOD(LlhttpParser, isPaused);
PHP_METHOD(LlhttpParser, getType);
PHP_METHOD(LlhttpParser, getHttpMajor);
PHP_METHOD(LlhttpParser, getHttpMinor);
PHP_METHOD(LlhttpParser, getMethod);
PHP_METHOD(LlhttpParser, getMethodName);
PHP_METHOD(LlhttpParser, getStatusCode);
PHP_METHOD(LlhttpParser, shouldKeepAlive);
PHP_METHOD(LlhttpParser, messageNeedsEof);
PHP_METHOD(LlhttpParser, getHeaders);

/* Events class methods */
PHP_METHOD(LlhttpEvents, getAll);
PHP_METHOD(LlhttpEvents, isValid);

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
void llhttp_call_user_callback(llhttp_parser_object *parser_obj, const char *event_name, zval *args, int arg_count);
void llhttp_add_header(llhttp_parser_object *parser_obj, zend_string *field, zend_string *value);
void llhttp_finalize_current_header(llhttp_parser_object *parser_obj);

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