#!/bin/bash

Case=$1;

blockMesh -case $Case > $Case/log.blockMesh
