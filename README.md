# php_nanomsg

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

## Usage Example

```php
<?php

if (!extension_loaded('nanomsg')) {
    die("nanomsg extension not loaded");
}

$file = '/tmp/test.ipc';
unlink($file);
$address = 'ipc://' . $file;

$sock1 = new Nanomsg( NanoMsg::AF_SP, NanoMsg::NN_PAIR );
$sock2 = new Nanomsg( NanoMsg::AF_SP, NanoMsg::NN_PAIR );

$sock1->bind($address);
$sock2->connect($address);

$sock1->send('It Works!', 0);

$sock2->setOption( NanoMsg::NN_SOL_SOCKET, NanoMsg::NN_RCVTIMEO, 100 );
$value = $sock2->getOption( NanoMsg::NN_SOL_SOCKET, NanoMsg::NN_RCVTIMEO );

$data = $sock2->recv(0, 0);

echo "received: " . $data . "\n";

?>
```
