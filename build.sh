#!/bin/bash

set -e

if [ ! -f "$1" ] || [ $2 == "" ]; then
  (echo "Usage: build.sh <config_file> <port_number>") >&2
  exit 1;
fi

docker build -t webserv .

docker run -p $2:1337 \
  --mount type=bind,source="$1",target=/etc/webserv/config.conf \
  webserv
