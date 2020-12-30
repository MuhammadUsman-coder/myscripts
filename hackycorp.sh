#!/bin/bash

for i in {a..z}{a..z};do
        echo "http://0x$i.a.hackycorp.com" >> 1.test
done

for i in {0..9}{0..9};do
        echo "http://0x$i.a.hackycorp.com" >> 1.test
done 
