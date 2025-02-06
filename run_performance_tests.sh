#!/bin/bash

make performance
hwloc-bind pu:2 ./perf_exp_small
hwloc-bind pu:2 ./perf_exp_large
