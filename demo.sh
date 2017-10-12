#!/usr/bin/env bash

./mtfsCreate -n demo
./mtfsCreate -i 0 -d 0 -b 0 -s 512 -r 0 demo
./mtfsCreate --add demo
./mtfsCreate -p 1 --add -c '{"highLimit": 2, "plName": "block", "params": {"devicePath": "/dev/sdb7", "fsType": "ext4"}}' demo
./mtfsCreate -p 1 --add -c '{"lowLimit": 1, "plName": "s3", "params": {"bucket": "mtfs", "region": "eu-central-1"}}' demo
./mtfsCreate --install demo