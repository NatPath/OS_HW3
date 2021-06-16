#!/bin/bash
for i in {1..1000}
    do
        ./client localhost 2323 home.html &
        ./client localhost 2323 favicon.ico &
        ./client localhost 2323 output.cgi &
done