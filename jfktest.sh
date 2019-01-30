#!/bin/bash

# http://practicalcryptography.com/ciphers/playfair-cipher/

echo
./playfair -v -p "ROYAL NEW ZEALAND NAVY" -d \
    "KXJEY  UREBE  ZWEHE  WRYTU  HEYFS \
     KREHE  GOYFI  WTTTU  OLKSY  CAJPO \
     BOTEI  ZONTX  BYBWT  GONEY  CUZWR \
     GDSON  SXBOU  YWRHE  BAAHY  USEDQ"

echo
./playfair -p "ROYAL NEW ZEALAND NAVY" -d \
    "KXJEY  UREBE  ZWEHE  WRYTU  HEYFS \
     KREHE  GOYFI  WTTTU  OLKSY  CAJPO \
     BOTEI  ZONTX  BYBWT  GONEY  CUZWR \
     GDSON  SXBOU  YWRHE  BAAHY  USEDQ"

echo
