#!/bin/bash
# @author Michael Wiesendanger <michael.wiesendanger@gmail.com>
# @description run script for docker-wow-vanilla-server container

set -euo pipefail

# variable setup
DOCKER_WOW_VANILLA_SERVER_TAG="ragedunicorn/wow-vanilla-server"
DOCKER_WOW_VANILLA_SERVER_NAME="wow-vanilla-server"
DOCKER_WOW_VANILLA_SERVER_ID=0

# get absolute path to script and change context to script folder
SCRIPTPATH="$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
cd "${SCRIPTPATH}"

# check if there is already an image created
docker inspect ${DOCKER_WOW_VANILLA_SERVER_NAME} &> /dev/null

if [ $? -eq 0 ]; then
  # start container
  docker start "${DOCKER_WOW_VANILLA_SERVER_NAME}"
else
  ## run image:
  # -d run in detached mode
  # -i Keep STDIN open even if not attached
  # -t Allocate a pseudo-tty
  # --name define a name for the container(optional)
  DOCKER_WOW_VANILLA_SERVER_ID=$(docker run \
  -p 8085:8085 \
  -p 3724:3724 \
  -dit \
  --name "${DOCKER_WOW_VANILLA_SERVER_NAME}" "${DOCKER_WOW_VANILLA_SERVER_TAG}")
fi

if [ $? -eq 0 ]; then
  # print some info about containers
  echo "$(date) [INFO]: Container info:"
  docker inspect -f '{{ .Config.Hostname }} {{ .Name }} {{ .Config.Image }} {{ .NetworkSettings.IPAddress }}' ${DOCKER_WOW_VANILLA_SERVER_NAME}
else
  echo "$(date) [ERROR]: Failed to start container - ${DOCKER_WOW_VANILLA_SERVER_NAME}"
fi
