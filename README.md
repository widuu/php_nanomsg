#PHP_NANOMSG

[![Build Status](https://travis-ci.org/widuu/php_nanomsg.svg?branch=master)](https://travis-ci.org/widuu/php_nanomsg)

## Requirements

* PHP 5.3+
* nanomsg 1.0.0

## INSTALL

First , you need to install nanomsg 

```
git clone https://github.com/widuu/php_nanomsg.git
phpize
./configure 
make CFLAGS="-I/yourpath/nanomsg/1.0.0/include" LDFLAGS="-L/yourpath/nanomsg/1.0.0/lib -lnanomsg" 
make install

```

Then add `extension=nanomsg.so` to your `php.ini`.