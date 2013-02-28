#!/bin/bash

SRC_FILES=`find src -type f -name *.cc -or -name *.h`
FILTER=-legal/copyright,-build/header_guard,-build/include_what_you_use,-build/include,-runtime/threadsafe_fn

./scripts/cpplint.py --filter=$FILTER $SRC_FILES
