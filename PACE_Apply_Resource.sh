#!/bin/env bash
# use the following command to request resource on the PACE cluster. Change mem if needed.
salloc -A gts-vsarkar9 -qinferno -N1 --mem=200G  --ntasks-per-node=1 -t01:00:00