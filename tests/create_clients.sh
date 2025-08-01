#!/bin/bash

for i in {1..100}; do
  echo "Test $i" | nc -q 0 127.0.0.1 6666
done
