#!/bin/bash

# install bazel

bazel build //client:zirconium_client
bazel build //server:zirconium_server