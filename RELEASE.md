# Release

> This document explains how a new release is created for this docker container

* Update docker-compose files
  * Update .env image tag version
  * Manually update docker-compose.stack.yml image to new version (stack does not support the .env file)
* Create a new git tag and push it
  * `git tag vx.x.x`
  * `git push origin --tags`
* Draft new Github release with description
  * Title should be the version e.g. vx.x.x
  * Short description of what was added newly
* Update docker hub
  * Build dev tag `docker-compose -f docker-compose.dev.yml build`
  * Push image to dockerhub `docker push ragedunicorn/wow-vanilla:x.x.x-dev`
  * Build stable tag `docker-compose build`
  * Push image to dockerhub `docker push ragedunicorn/wow-vanilla:x.x.x-stable`
  * Tag and push stable version as latest version (default image for docker hub)
    * `docker tag ragedunicorn/wow-vanilla:x.x.x-stable ragedunicorn/wow-vanilla:latest`
    * `docker push ragedunicorn/wow-vanilla:latest`
* Update docker hub description links
