#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"

#include <ctype.h>
#include <math.h>

enum {
    WULFTEST_CODE_SUCCESS = 0,
    WULFTEST_CODE_ARG_COUNT,
    WULFTEST_CODE_OBJECT_CONVERT,
    WULFTEST_CODE_INVALID_TYPE
};

#define WULFTEST_THROW(code, ...) { zend_throw_exception_ex(wulftest_exception, WULFTEST_CODE_ ## code TSRMLS_CC, __VA_ARGS__); return; }

#define WULFTEST_ERROR_CONSTANT(ce, code) zend_declare_class_constant_long(ce, "ERROR" #code, sizeof ("ERROR" #code) - 1, (long) WULFTEST_CODE_ ## code)

static zend_class_entry *wulftest_exception;

static PHP_METHOD(WULFTEST, parseint)
{
    zval *arg;
    long value;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg) == FAILURE) {
        WULFTEST_THROW(ARG_COUNT, "Invalid number of arguments");
    }

    switch (arg->type) {
        case IS_OBJECT: case IS_STRING: {
            char *str;
            int len;
            int state = 0;
            if (arg->type == IS_OBJECT) {
                if (zend_parse_parameter(0, 1, &arg, "s", &str, &len) == FAILURE) {
                    WULFTEST_THROW(OBJECT_CONVERT, "Cannot convert object to string");
                }
            } else {
                str = Z_STRVAL_P(arg);
                len = Z_STRLEN_P(arg);
            }
            // error checking missing
            value = strtol(str, NULL, 0);
        } break;

        case IS_LONG: {
            value = Z_LVAL_P(arg);
        } break;

        case IS_DOUBLE: {
            double dv = Z_DVAL_P(arg);
            double tmp = nearbyint(dv);
            value = (long) tmp;
        } break;

        default:
            WULFTEST_THROW(INVALID_TYPE, "Parameter type not supported");
    }

    RETURN_LONG(value);
}


ZEND_BEGIN_ARG_INFO(arginfo_wulftest, 0)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static const zend_function_entry wulftest_functions[] = {
    PHP_ME(WULFTEST, parseint, arginfo_wulftest, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_FE_END
};

PHP_MINIT_FUNCTION(wulftest)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "WulfTest_Exception", NULL);
    wulftest_exception = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);

    INIT_CLASS_ENTRY(ce, "WulfTest_Tools", wulftest_functions);
    zend_class_entry *class = zend_register_internal_class(&ce TSRMLS_CC);
    WULFTEST_ERROR_CONSTANT(class, SUCCESS);
    WULFTEST_ERROR_CONSTANT(class, ARG_COUNT);
    WULFTEST_ERROR_CONSTANT(class, OBJECT_CONVERT);
    WULFTEST_ERROR_CONSTANT(class, INVALID_TYPE);

    return SUCCESS;
}

zend_module_entry wulftest_module_entry = {
    STANDARD_MODULE_HEADER,
    "wulftest", // extension name
    /*wulftest_functions,*/NULL,
    PHP_MINIT(wulftest),
    NULL, NULL, NULL, NULL,
    "1.0", // extension version
    STANDARD_MODULE_PROPERTIES
};
 
ZEND_GET_MODULE(wulftest)
