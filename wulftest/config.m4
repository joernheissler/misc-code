PHP_ARG_ENABLE(wulftest, whether to enable wulftest support,
[  --enable-wulftest           Enable wulftest support])

if test "$PHP_WULFTEST" != "no"; then
    PHP_NEW_EXTENSION(wulftest, wulftest.c, $ext_shared)
fi
