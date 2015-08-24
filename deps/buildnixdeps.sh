#!/bin/bash

git submodule init && git submodule update
cd qjsonrpc && qmake PREFIX=/usr/local && make -j4 && make install
cd ../
