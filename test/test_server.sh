#!/bin/bash

./bazel-bin/client/zirconium_client --username=shado --pin=123456 --to_user meow --amt=500 &
./bazel-bin/client/zirconium_client --username=meow  --pin=123456 --to_user shado --amt=500 &
# ./bazel-bin/client/zirconium_client --username=shado --pin=123456 --deposit=500 &
# ./bazel-bin/client/zirconium_client --username=meow   --pin=123456 --withdraw=500 &