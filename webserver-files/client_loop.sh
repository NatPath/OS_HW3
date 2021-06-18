#!/bin/bash
for i in {1..10}
    do
        ./client localhost 2323 output.cgi &
done