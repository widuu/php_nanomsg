/*
  +----------------------------------------------------------------------+
  | PHP Nanomsg Extension (php7 & php5)                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2016-2017 Widuu Inc                                    |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: widuu<admin@widuu.com> http://www.widuu.com                  |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_NANOMSG_H
#define PHP_NANOMSG_H

extern zend_module_entry nanomsg_module_entry;
#define phpext_nanomsg_ptr &nanomsg_module_entry

#define PHP_NANOMSG_VERSION "0.0.1"

#ifdef PHP_WIN32
# define PHP_NANOMSG_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define PHP_NANOMSG_API __attribute__ ((visibility("default")))
#else
# define PHP_NANOMSG_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(nanomsg)
  zend_bool use_namespace;
ZEND_END_MODULE_GLOBALS(nanomsg)

#ifdef ZTS
#define NANOMSG_G(v) TSRMG(nanomsg_globals_id, zend_nanomsg_globals *, v)
#else
#define NANOMSG_G(v) (nanomsg_globals.v)
#endif


/* {{{ 注册命名空间 */
#define NANOMSG_INIT_CLASS_ENTRY(ce, name, ns_name, methods) \
        if( NANOMSG_G(use_namespace) ){ \
            INIT_CLASS_ENTRY(ce, ns_name, methods); \
        }else{ \
            INIT_CLASS_ENTRY(ce, name, methods); \
        }
/* }}} */

/* {{{  NanoMsg Object */
typedef struct
{
    zend_object std;
    int  s;
}php_nanomsg_object;
/* }}} */

// 更新错误代码
#define PHP_NANOMSG_UPDATE_ERROR_CODE(ce, value) zend_update_property_long(ce, getThis(), ZEND_STRL("errorCode"), (long)value TSRMLS_CC);
// 更新错误信息
#define PHP_NANOMSG_UPDATE_ERROR_INFO(ce, info) zend_update_property_string(ce, getThis(), ZEND_STRL("errorInfo"), info TSRMLS_CC);


static inline php_nanomsg_object *php_nanomsg_object_fetch(zend_object *obj) {
  return (php_nanomsg_object *)((char *)obj - XtOffsetOf(php_nanomsg_object, std));
}

#ifndef Z_OBJ_P
#define Z_OBJ_P(pvz) ((zend_object *) zend_object_store_get_object(pvz TSRMLS_CC))
#endif

#define PHP_NANOMSG_GET_OBJECT(obj) php_nanomsg_object_fetch(Z_OBJ_P(obj))

#endif  /* PHP_NANOMSG_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
