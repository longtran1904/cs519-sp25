#! /bin/bash
echo 1 | sudo tee $TRACER/tracing_on
./multi-bench $1 $2
echo 0 | sudo tee $TRACER/tracing_on