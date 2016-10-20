dnl $Id$
dnl config.m4 for extension nanomsg

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(nanomsg, for nanomsg support,
dnl Make sure that the comment is aligned:
dnl [  --with-nanomsg             Include nanomsg support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(nanomsg, whether to enable nanomsg support,
Make sure that the comment is aligned:
[  --enable-nanomsg           Enable nanomsg support])

if test "$PHP_NANOMSG" != "no"; then
  PHP_NEW_EXTENSION(nanomsg, nanomsg.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
