#!/bin/bash

make simulation
hwloc-bind pu:2 ./sim_exp
