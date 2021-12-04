#!/bin/bash

FILE=/dev/ttyACM1
READ_TIMEOUT=1

log() {
  date=$(date '+%d.%m.%Y %H:%M:%S:%N')
  echo "$date" "$@"  
}


(
while true; do
  if read -t $READ_TIMEOUT line; then
    if [ ! -z "$line" ];  then
      log $line
    fi
  fi
done
) < "$FILE"