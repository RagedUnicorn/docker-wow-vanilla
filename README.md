# docker-wow-vanilla

> A docker base image to build a container for WoW Vanilla Server

This image is intended to build a base for running a WoW Vanilla Server based on the work of CMangos, Nostalrius, Elysium and Lights Hope. It uses [ragedunicorn/mysql](https://cloud.docker.com/repository/docker/ragedunicorn/mysql) as a provider for the database.

Its intent is to provide a WoW Server for experimenting and developing WoW Addons.

## Using the image

#### Start container

The container can be easily started with `docker-compose` command.

```
docker-compose up -d
```

#### Stop container

To stop all services from the docker-compose file

```
docker-compose down
```

#### Join a swarm

```
docker swarm init
```

#### Create secrets
```
echo "some_password" | docker secret create com.ragedunicorn.mysql.root_password -
echo "app_user" | docker secret create com.ragedunicorn.mysql.app_user -
echo "app_user_password" | docker secret create com.ragedunicorn.mysql.app_user_password -
```

#### Deploy stack
```
docker stack deploy --compose-file=docker-compose.stack.yml [stackname]
```

For a production deployment a stack should be deployed. Secrets will then be taken into account and both the MySQL container and the WoW container will be setup accordingly.

### Overwriting build args

Using `docker-compose.dev.yml` or `docker-compose.yml` allows for easy overriding of certain build arguments.

```
wow_vanilla_server:
  build:
    context: .
    args:
      WOW_USER: vanilla
      WOW_GROUP: vanilla
      WOW_HOME: /home/vanilla
      BUILD_EXTRACTORS: 1
```

### Create WoW Account

You will have to create an account by yourself. The server service allocates a tty for the mangos console. This allows to attach directly to the mangosd process and its console.

```
docker attach [CONTAINER]

account create [USERNAME] [PASSWORD]
account set gmlevel [USERNAME] [LEVEL]
```

Mangosd differentiates between 7 security levels.

```
SEC_PLAYER         = 0
SEC_MODERATOR      = 1
SEC_MODERATOR_CONF = 2
SEC_GAMEMASTER     = 3
SEC_BASIC_ADMIN    = 4
SEC_DEVELOPPER     = 5
SEC_ADMINISTRATOR  = 6
SEC_CONSOLE        = 7
```

See `data\server\src\game\Chat\Chat.cpp` for details on what each security level is required for what command.

## Dockery

In the dockery folder are some scripts that help out avoiding retyping long docker commands but are mostly intended for playing around with the container. For production use docker-compose should be used.

#### Build image

The build script builds an image with a defined name

```
sh dockery/dbuild.sh
```

#### Run container

Runs the built container. If the container was already run once it will `docker start` the already present container instead of using `docker run`

```
sh dockery/drun.sh
```

#### Attach container

Attaching to the container after it is running

```
sh dockery/dattach.sh
```

#### Stop container

Stopping the running container

```
sh dockery/dstop.sh
```

## Configuration

### Realm

By default a realm with the name `raged_unicorn` will be created. This can be modified by modifying `init_realm.sql` inside `data/sql` or by modifying `realmlist` inside the `realmd` database itself.

### realmd.conf

Configuration for realm is located in `config/realmd.conf.tpl` and copied into the image `opt/vanilla/etc`

### mangosd.conf

Configuration for world server is located in `config/mangosd.conf.tpl` and copied into the image `opt/vanilla/etc`

During the startup of the server the templates are rendered and the final configuration is created. This step is repeated each time the `docker-entrypoint.sh` script is executed. It is important that all modifications are done in the templates itself and not the generated configuration. Those will be overwritten.

Using this information configuration files can be overwritten by mounting different ones into the container. Again it is important that the template files are being used otherwise the configuration will be overwritten after each restart of the container.

In this example the same configurations that are copied into the image are mounted when using docker-compose. In most cases it makes more sense to have a separate configuration somewhere. This makes it easy having different configurations for different servers.

```
volumes:
  - ${PWD}/config/mangosd.conf.tpl:/opt/vanilla/etc/mangosd.conf.tpl
  - ${PWD}/config/realmd.conf.tpl:/opt/vanilla/etc/realmd.conf.tpl
```

#### Patch Level

By default patch level is set to 10 -> 1.12. This is essentially the highest patch available and enables all items. For developing wow addons this is usually fine. However if this is unwanted change the patch level value to whatever is required.

Search for `WoWPatch` in the `mangosd.conf` config and change the value appropriately.

## Development

To debug the container and get more insight into the container use the `docker-compose.dev.yml`
configuration.

```
docker-compose -f docker-compose.dev.yml up -d
```

By default the launchscript `/docker-entrypoint.sh` will not be used to start the `realmd` and `mangosd` processes. Instead the container will be setup to keep `stdin_open` open and allocating a pseudo `tty`. This allows for connecting to a shell and work on the container. A shell can be opened inside the container with `docker attach [container-id]`. The server itself can be started with `./docker-entrypoint.sh`. Note that this has to be done for the database container first otherwise both `realmd` and `mangosd` will not be able to connect to the database.

### Generating Server Files (optional)

This repository does not contain the necessary client files such as `dbc`, `maps`, `mmaps` and `vmaps`. Generate this files by following the steps below and then place them in the data folder. The data folder will be added as a local volume to the container.

When building the server from source make sure to include the extractors. This can be done with the build arg `BUILD_EXTRACTORS` set to `1` (default is `0`).

```bash
  cmake ../ -DCMAKE_INSTALL_PREFIX="${WOW_INSTALL}" -DUSE_EXTRACTORS="${BUILD_EXTRACTORS}" && \
  make && \
  make install
```

This will create the required extractors during the compilation of the server. With make install the binaries are copied to `/opt/vanilla/bin`. For extracting the data a WoW installation is required. It does not matter in this case whether that installation is for Mac or Windows.

Copy wow client to running docker container. WoW client is not contained in this repository.

`docker cp [client folder] [container-id]:/home/[user]/`

#### Generate maps and dbc

`/opt/vanilla/bin/mapextractor -i /home/wow/wow-vanilla-client_lights_hope -o /home/wow/wow-vanilla-client_lights_hope/`

This will generate a `dbc` and a `maps` folder in the chosen output path.

#### Generate vmaps base

Make sure that certain mpq files are all lowercase because the vmapextractor cannot handle those if their extension is uppercase

```bash
mv Data/terrain.MPQ Data/terrain.mpq
mv Data/model.MQP Data/model.mpq
mv Data/texture.MQP Data/texture.mpq
mv Data/wmo.MQP Data/wmo.mpq
mv Data/base.MQP Data/base.mpq
```

```bash
# extract vmaps
/opt/vanilla/bin/vmapextractor -d /home/wow/wow-vanilla-client_lights_hope/Data
# create assemble directory
mkdir /home/wow/wow-vanilla-client_lights_hope/vmaps
# assemble vmaps
/opt/vanilla/bin/vmap_assembler /home/wow/wow-vanilla-client_lights_hope/Buildings/ /home/wow/wow-vanilla-client_lights_hope/vmaps/
```

#### Generate mmaps

```bash
mkdir /home/wow/wow-vanilla-client_lights_hope/mmaps
/opt/vanilla/bin/MoveMapGen
```

*Note:* This will take a long time to generate

#### Package the Resources

Extract the data from the image with `docker cp`

`docker cp <containerId>:/[data-file] [host-path]`

Most of the data inside `data` is packaged into tar.gz files and for some even split into multiple files. This is not a requirement itself but was done to work around the 100MB file limit of github and to stay below 1GB for the complete allowed size of the repository. When regenerating the data files make sure to update this as well.

#### Source

Server source:

https://github.com/lh-server/core

Database source:

https://github.com/brotalnia/database

#### Misc

The server service keeps `stdin_open` open and allocates a pseudo `tty` for the server process startup successfully. The reason for this is that the server process provides a cli for sending commands to the server. If this cli cannot be started the whole process fails. Not allocation a `tty` and keeping `stdin_open` prevents the process from launching that cli.

## Test

To do basic tests of the structure of the container use the `docker-compose.test.yml` file.

`docker-compose -f docker-compose.test.yml up`

For more info see [container-test](https://github.com/RagedUnicorn/docker-container-test).

Tests can also be run by category such as command and metadata tests by starting single services in `docker-compose.test.yml`

```
# metadata tests
docker-compose -f docker-compose.test.yml up container-test-metadata
```

The same tests are also available for the development image.

```
# metadata tests
docker-compose -f docker-compose.test.yml up container-dev-test-metadata
```

## Links

Ubuntu packages database
- http://packages.ubuntu.com/

## License

Copyright (C) 2019 Michael Wiesendanger

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
