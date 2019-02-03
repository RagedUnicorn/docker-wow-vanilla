#!/bin/bash
# @author Michael Wiesendanger <michael.wiesendanger@gmail.com>
# @description stop script for docker-wow-vanilla-server container

set -euo pipefail

WD="${PWD}"

# variable setup
DOCKER_WOW_VANILLA_SERVER_NAME="wow-vanilla-server"

# get absolute path to script and change context to script folder
SCRIPTPATH="$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
cd "${SCRIPTPATH}"

# search running container
docker ps | grep "${DOCKER_WOW_VANILLA_SERVER_NAME}" > /dev/null

# if container is running - stop it
if [ $? -eq 0 ]; then
  echo "$(date) [INFO]: Stopping container "${DOCKER_WOW_VANILLA_SERVER_NAME}" ..."
  docker stop "${DOCKER_WOW_VANILLA_SERVER_NAME}" > /dev/null
else
  echo "$(date) [INFO]: No running container with name: ${DOCKER_WOW_VANILLA_SERVER_NAME} found"
fi

cd "${WD}"
