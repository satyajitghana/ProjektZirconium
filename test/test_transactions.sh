#!/bin/bash

./bazel-bin/client/zirconium_client --config client/client.config --username=shado --pin=123456 --to_user meow --amt=100 &
./bazel-bin/client/zirconium_client --config client/client.config --username=shado --pin=123456 --deposit=200 &
./bazel-bin/client/zirconium_client --config client/client.config --username=meow  --pin=123456 --to_user shado --amt=100 &
./bazel-bin/client/zirconium_client --config client/client.config --username=shado --pin=123456 --withdraw=100 &