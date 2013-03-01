#!/bin/bash

FILTER=-legal/copyright,-build/header_guard,-build/include_what_you_use,-build/include,-runtime/threadsafe_fn
for d in src test
do
  echo Checking $d
  FILES=$(find $d -type f -name *.cc -or -name *.h)
  ./scripts/cpplint.py --filter=$FILTER $FILES
  echo 'complete'
done
