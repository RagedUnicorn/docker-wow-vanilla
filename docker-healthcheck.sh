#!/bin/bash

set -euo pipefail

if nc -z localhost 8085 && nc -z localhost 3724; then
	exit 0
fi

exit 1
