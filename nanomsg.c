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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_nanomsg.h"
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

ZEND_DECLARE_MODULE_GLOBALS(nanomsg)

/*{{{ */
static zend_class_entry *nanomsg_ce;
static zend_object_handlers nanomsg_object_handlers;
/* }}} */

static int le_nanomsg;

/* {{{ PHP_INI Namespace Info
 */

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("nanomsg.use_namespace", "0", PHP_INI_SYSTEM, OnUpdateBool, use_namespace, zend_nanomsg_globals, nanomsg_globals)
PHP_INI_END()

/* }}} */

ZEND_BEGIN_ARG_INFO_EX(nanomsg_construct_args, 0, 0, 2)
    ZEND_ARG_INFO(0, domain)
    ZEND_ARG_INFO(0, protocol)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(nanomsg_bind_args, 0, 0, 1)
    ZEND_ARG_INFO(0, endpoint)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(nanomsg_connect_args, 0, 0, 1)
    ZEND_ARG_INFO(0, addr)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(nanomsg_send_args, 0, 0, 2)
    ZEND_ARG_INFO(0, message)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(nanomsg_recv_args, 0, 0, 2)
    ZEND_ARG_INFO(0, buf_length)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(nanomsg_shutdown_args, 0, 0, 1)
    ZEND_ARG_INFO(0, eid)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(nanomsg_setOption_args, 0, 0, 3)
    ZEND_ARG_INFO(0, level)
    ZEND_ARG_INFO(0, option)
    ZEND_ARG_INFO(0, optval)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(nanomsg_getOption_args, 0, 0, 2)
    ZEND_ARG_INFO(0, level)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

/* {{{ Free Object
 */
static void nanomsg_object_free(php_nanomsg_object *object TSRMLS_DC){
    php_nanomsg_object *std;
#ifdef ZEND_ENGINE_3  
    std = (php_nanomsg_object *)((char *)object - XtOffsetOf(php_nanomsg_object, std));
#else
    std = (php_nanomsg_object *) object;
#endif  
    if( !std ){
      return;
    }

    if( std->s >= 0 ){
      nn_close (std->s);
    }

    zend_object_std_dtor(&std->std);
    efree(object);
}
/* }}}
 */

/* {{{ Create new php_nanomsg_object
 */
#ifdef ZEND_ENGINE_3
static zend_object *nanomsg_create_new_object(zend_class_entry *ce TSRMLS_DC){
  
    php_nanomsg_object *object;
  
    object = ecalloc(1, sizeof(php_nanomsg_object) + zend_object_properties_size(ce));

    object->s = -1;

    zend_object_std_init(&(object->std), ce TSRMLS_CC);
    object_properties_init(&(object->std), ce);

    nanomsg_object_handlers.offset = XtOffsetOf(php_nanomsg_object, std);
    nanomsg_object_handlers.free_obj  = (zend_object_free_obj_t) nanomsg_object_free;
    nanomsg_object_handlers.clone_obj = NULL;
    object->std.handlers = &nanomsg_object_handlers;
    
    return &object->std;
}
#else
static zend_object_value nanomsg_create_new_object(zend_class_entry *ce TSRMLS_DC){
    zend_object_value retval;
    php_nanomsg_object *object;

    object = ecalloc(1,sizeof(php_nanomsg_object));
    object->std.ce = ce;

    object->s = -1;

    object_properties_init(&object->std, ce);
    retval.handle  = zend_objects_store_put(object, NULL, (zend_objects_free_object_storage_t) nanomsg_object_free, NULL TSRMLS_CC);
    retval.handlers = &nanomsg_object_handlers;

    return retval;
}
#endif
/* }}}
 */

/* {{{ new Nanomsg( NanoMsg::AF_SP, NanoMsg::NN_BUS );
 */
PHP_METHOD(Nanomsg, __construct){

    php_nanomsg_object *object;
    long domain, protocol;

    if( zend_parse_parameters(ZEND_NUM_ARGS () TSRMLS_CC, "ll", &domain, &protocol) == FAILURE ){
      php_error_docref(NULL, E_ERROR, "Expected at least 2 parameter.");
      return;
    }

    object = PHP_NANOMSG_GET_OBJECT(getThis() TSRMLS_CC);

    object->s = nn_socket (domain, protocol);
    if (object->s < 0){
      char *errorInfo;
      spprintf(&errorInfo,0,"Error creating nanomsg socket:%s",nn_strerror (errno));
      PHP_NANOMSG_UPDATE_ERROR_CODE(nanomsg_ce, 400);
      PHP_NANOMSG_UPDATE_ERROR_INFO(nanomsg_ce, errorInfo);
      efree(errorInfo);
      return;
    }
}
/* }}}
 */

/* {{{  Nanomsg bind $nanomsg->bind('ipc:///tmp/node0.ipc');
 */
PHP_METHOD(Nanomsg, bind){
  
    php_nanomsg_object *object;

    int endpoint_id,endpoint_length;
    char *endpoint;

    if(zend_parse_parameters(ZEND_NUM_ARGS () TSRMLS_CC, "s", &endpoint, &endpoint_length) == FAILURE) {
        return;
    }

    object = PHP_NANOMSG_GET_OBJECT(getThis() TSRMLS_CC);
    endpoint_id = nn_bind (object->s, endpoint);

    if (endpoint_id < 0) {
      char *errorInfo;
      spprintf(&errorInfo,0,"Nanomsg bind error:%s",nn_strerror (errno));
      PHP_NANOMSG_UPDATE_ERROR_CODE(nanomsg_ce, 401);
      PHP_NANOMSG_UPDATE_ERROR_INFO(nanomsg_ce, errorInfo);
      efree(errorInfo);
        return;
    }
    
    RETURN_LONG(endpoint_id);
}
/* }}}
 */

/* {{{  Nanomsg connect $nanomsg->connect('ipc:///tmp/node0.ipc');
 */
PHP_METHOD(Nanomsg, connect){
  
    php_nanomsg_object *object;

    int endpoint_id,endpoint_length;
    char *addr;

    if(zend_parse_parameters(ZEND_NUM_ARGS () TSRMLS_CC, "s", &addr, &endpoint_length) == FAILURE) {
        return;
    }

    object = PHP_NANOMSG_GET_OBJECT(getThis() TSRMLS_CC);
    endpoint_id = nn_connect (object->s, addr);

    if (endpoint_id < 0) {
      char *errorInfo;
      spprintf(&errorInfo,0,"Nanomsg connect error:%s",nn_strerror (errno));
      PHP_NANOMSG_UPDATE_ERROR_CODE(nanomsg_ce, 402);
      PHP_NANOMSG_UPDATE_ERROR_INFO(nanomsg_ce, errorInfo);
      efree(errorInfo);
        return;
    }
    
    RETURN_LONG(endpoint_id);
}
/* }}}
 */

/* {{{  Nanomsg send $nanomsg->send('data',0);
 */

PHP_METHOD(Nanomsg, send){
  
    php_nanomsg_object *object;

    int  message_length,send_length;
    char *message;
    long flags = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &message, &message_length, &flags) == FAILURE) {
        return;
    }

    object = PHP_NANOMSG_GET_OBJECT(getThis() TSRMLS_CC);
    
    if( (send_length = nn_send (object->s, message, message_length, flags)) < 0 ){
      if( flags & NN_DONTWAIT && errno == EAGAIN ){
        char *errorInfo;
        spprintf(&errorInfo,0,"Nanomsg send error:%s",nn_strerror (errno));
        PHP_NANOMSG_UPDATE_ERROR_CODE(nanomsg_ce, 403);
        PHP_NANOMSG_UPDATE_ERROR_INFO(nanomsg_ce, errorInfo);
        efree(errorInfo);
        RETURN_LONG(0);
      }
      return;
    }

    RETURN_LONG(send_length);
}
/* }}}
 */

/* {{{  Nanomsg set socket option $nanomsg->setOption(NanoMsg::NN_SOL_SOCKET, NanoMsg::NN_RCVTIMEO, 100);
 */
PHP_METHOD(Nanomsg, setOption){
    php_nanomsg_object *object;
    long level, option;
    zval *optval;

    if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llz", &level, &option, &optval) == FAILURE ){
      return;
    }

    object = PHP_NANOMSG_GET_OBJECT(getThis() TSRMLS_CC);

    if( level == NN_SUB && (option == NN_SUB_SUBSCRIBE || option == NN_SUB_UNSUBSCRIBE) ){
      if( nn_setsockopt(object->s, level, option, Z_STRVAL_P(optval), Z_STRLEN_P(optval)) >= 0 ){
        RETURN_TRUE;
        return;
      }
    }else{
      const void *option_val = &Z_LVAL_P(optval);
      if( nn_setsockopt(object->s, level, option, option_val, sizeof(int)) >= 0 ){
        RETURN_TRUE;
        return;
      }
    }

    char *errorInfo;
    spprintf(&errorInfo,0,"Nanomsg set socket option error:%s",nn_strerror (errno));
    PHP_NANOMSG_UPDATE_ERROR_CODE(nanomsg_ce, 406);
    PHP_NANOMSG_UPDATE_ERROR_INFO(nanomsg_ce, errorInfo);
    efree(errorInfo);
    RETURN_FALSE;
}
/* }}}
 */

/* {{{  Nanomsg get socket option $nanomsg->getOption(NanoMsg::NN_SOL_SOCKET, NanoMsg::NN_RCVTIMEO);
 */
PHP_METHOD(Nanomsg, getOption){
    php_nanomsg_object *object;
    long level, option;
    int optval;

    if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &level, &option) == FAILURE ){
      return;
    }

    object = PHP_NANOMSG_GET_OBJECT(getThis() TSRMLS_CC);

    size_t sz = sizeof(optval);
    if( nn_getsockopt (object->s, level, option, &optval, &sz) >= 0 ){
      RETURN_LONG(optval);
      return;
    }

    char *errorInfo;
    spprintf(&errorInfo,0,"Nanomsg set socket option error:%s",nn_strerror (errno));
    PHP_NANOMSG_UPDATE_ERROR_CODE(nanomsg_ce, 406);
    PHP_NANOMSG_UPDATE_ERROR_INFO(nanomsg_ce, errorInfo);
    efree(errorInfo);
    RETURN_FALSE;
}
/* }}}
 */

/* {{{  Nanomsg recv $nanomsg->recv(0,0);
 */
PHP_METHOD(Nanomsg, recv){
  
    php_nanomsg_object *object;

    void *buf = NULL;
    long flags = 0;
    long buf_length = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS () TSRMLS_CC, "l|l", &buf_length, &flags) == FAILURE) {
        return;
    }
    
    object = PHP_NANOMSG_GET_OBJECT(getThis() TSRMLS_CC);

    if( buf_length > 0 ){
      char buf_recv[buf_length];
      if( nn_recv (object->s, buf_recv, buf_length, flags) > 0 ){
#ifdef ZEND_ENGINE_3
        ZVAL_STRINGL(return_value, buf_recv, buf_length);
#else
        ZVAL_STRINGL(return_value, buf_recv, buf_length, 1);
#endif
        return;
      }
    }else{
      if( (buf_length = nn_recv (object->s, &buf, NN_MSG, flags)) > 0 ){
#ifdef ZEND_ENGINE_3
        ZVAL_STRINGL(return_value, buf, buf_length);
#else
        ZVAL_STRINGL(return_value, buf, buf_length, 1);
#endif
        nn_freemsg(buf);
        return;
      }
    }

    char *errorInfo;
    spprintf(&errorInfo,0,"Nanomsg recv error:%s",nn_strerror (errno));
    PHP_NANOMSG_UPDATE_ERROR_CODE(nanomsg_ce, 404);
    PHP_NANOMSG_UPDATE_ERROR_INFO(nanomsg_ce, errorInfo);
    efree(errorInfo);
    RETURN_FALSE;
}
/* }}}
 */

/* {{{  Nanomsg shutdown $nanomsg->shutdown($eid);
 */
PHP_METHOD(Nanomsg, shutdown){
    php_nanomsg_object *object;
    int eid;

    if(zend_parse_parameters(ZEND_NUM_ARGS () TSRMLS_CC, "l", &eid) == FAILURE) {
        return;
    }
    
    object = PHP_NANOMSG_GET_OBJECT(getThis() TSRMLS_CC);

    if( nn_shutdown(object->s, eid) >= 0 ){
      RETURN_TRUE;
      return;
    }

    char *errorInfo;
    spprintf(&errorInfo,0,"Nanomsg shutdown error:%s",nn_strerror (errno));
    PHP_NANOMSG_UPDATE_ERROR_CODE(nanomsg_ce, 405);
    PHP_NANOMSG_UPDATE_ERROR_INFO(nanomsg_ce, errorInfo);
    efree(errorInfo);
    RETURN_FALSE;
}
/* }}}
 */

/* {{{ nanomsg_functions[]
 *
 * Every user visible function must have an entry in nanomsg_functions[].
 */
const zend_function_entry nanomsg_method[] = {
    PHP_ME(Nanomsg, __construct, nanomsg_construct_args,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(Nanomsg, bind,        nanomsg_bind_args,       ZEND_ACC_PUBLIC)
    PHP_ME(Nanomsg, connect,     nanomsg_connect_args,    ZEND_ACC_PUBLIC)
    PHP_ME(Nanomsg, send,      	 nanomsg_send_args,       ZEND_ACC_PUBLIC)
    PHP_ME(Nanomsg, recv,      	 nanomsg_recv_args,       ZEND_ACC_PUBLIC)
    PHP_ME(Nanomsg, shutdown,    nanomsg_shutdown_args,   ZEND_ACC_PUBLIC)
    PHP_ME(Nanomsg, setOption,   nanomsg_setOption_args,  ZEND_ACC_PUBLIC)
    PHP_ME(Nanomsg, getOption,   nanomsg_getOption_args,  ZEND_ACC_PUBLIC)
    PHP_FE_END  /* Must be the last line in nanomsg_functions[] */
};
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(nanomsg)
{
    REGISTER_INI_ENTRIES();

    memcpy(&nanomsg_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    
    zend_class_entry ce;
    NANOMSG_INIT_CLASS_ENTRY(ce, "Nanomsg", "Widuu\\Nanomsg", nanomsg_method);
    ce.create_object = nanomsg_create_new_object;
    nanomsg_ce = zend_register_internal_class(&ce TSRMLS_CC);
    
    // 注册静态变量
    int value, i;
      for (i = 0; ; ++i) {
          const char *name = nn_symbol (i, &value);
          if (name == NULL)
              break;
      zend_declare_class_constant_long(nanomsg_ce, name, strlen(name), (long) value TSRMLS_CC);
    }

    // 最后一条错误信息
    zend_declare_property_string(nanomsg_ce, ZEND_STRL("errorInfo"), "", ZEND_ACC_PUBLIC TSRMLS_CC); 
    // 错误代码
    zend_declare_property_long(nanomsg_ce, ZEND_STRL("errorCode"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(nanomsg)
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(nanomsg)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "nanomsg support", "enabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */


/* {{{ nanomsg_module_entry
 */
zend_module_entry nanomsg_module_entry = {
    STANDARD_MODULE_HEADER,
    "nanomsg",
    NULL,
    PHP_MINIT(nanomsg),
    PHP_MSHUTDOWN(nanomsg),
    NULL,
    NULL,
    PHP_MINFO(nanomsg),
    PHP_NANOMSG_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_NANOMSG
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(nanomsg)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
