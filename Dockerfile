FROM ubuntu:bionic

LABEL com.ragedunicorn.maintainer="Michael Wiesendanger <michael.wiesendanger@gmail.com>"

#  _    _____    _   ________    __    ___       _____ __________ _    ____________
# | |  / /   |  / | / /  _/ /   / /   /   |     / ___// ____/ __ \ |  / / ____/ __ \
# | | / / /| | /  |/ // // /   / /   / /| |     \__ \/ __/ / /_/ / | / / __/ / /_/ /
# | |/ / ___ |/ /|  // // /___/ /___/ ___ |    ___/ / /___/ _, _/| |/ / /___/ _, _/
# |___/_/  |_/_/ |_/___/_____/_____/_/  |_|   /____/_____/_/ |_| |___/_____/_/ |_|

# software versions
ENV \
  WGET_VERSION=1.19.4-1ubuntu2.1 \
  CA_CERTIFICATES_VERSION=20180409 \
  DIRMNGR_VERSION=2.2.4-1ubuntu1.2 \
  GOSU_VERSION=1.10 \
  GPG_VERSION=2.2.4-1ubuntu1.2 \
  GPG_AGENT_VERSION=2.2.4-1ubuntu1.2 \
  GPG_CONF_VERSION=2.2.4-1ubuntu1.2 \
  BUILD_ESSENTIAL_VERSION=12.4ubuntu1 \
  MAKE_VERSION=4.1-9.1ubuntu1 \
  CMAKE_VERSION=3.10.2-1ubuntu2 \
  GCC_VERSION=4:7.3.0-3ubuntu2 \
  GPLUSPLUS_VERSION=4:7.3.0-3ubuntu2 \
  AUTOMAKE_VERSION=1:1.15.1-3ubuntu2 \
  AUTOCONF_VERSION=2.69-11 \
  LIBMYSQLPLUSPLUS_DEV_VERSION=3.2.2+pristine-2ubuntu3 \
  MYSQL_CLIENT_VERSION=5.7.25-0ubuntu0.18.04.2 \
  LIBTOOL_VERSION=2.4.6-2 \
  LIBSSL_1_0_DEV_VERSION=1.0.2n-1ubuntu5.2 \
  BINUTILS_VERSION=2.30-21ubuntu1~18.04 \
  ZLIBC_VERSION=0.9k-4.3 \
  LIBC_6_VERSION=2.27-3ubuntu1 \
  LIBBZ_2_DEV_VERSION=1.0.6-8.1 \
  NETCAT_VERSION=1.10-41.1 \
  GIT_VERSION=1:2.17.1-1ubuntu0.4 \
  LIBTBB_DEV_VERSION=2017~U7-8 \
  LIBACE_DEV_VERSION=6.4.5+dfsg-1build2 \
  NANO_VERSION=2.9.3-2

# image args
ARG WOW_USER=wow
ARG WOW_GROUP=wow
ARG WOW_INSTALL=/opt/vanilla
ARG WOW_HOME=/home/wow
# build extractory for extracting client data
# 0 - false, 1 - true
ARG BUILD_EXTRACTORS=0

ENV \
  WOW_USER="${WOW_USER}" \
  WOW_GROUP="${WOW_GROUP}" \
  WOW_INSTALL="${WOW_INSTALL}" \
  WOW_LOG_DIR="/var/log/wow" \
  WOW_HOME="${WOW_HOME}" \
  PUBLIC_IP="127.0.0.1" \
  REALM_NAME="ragedunicorn" \
  GOSU_GPGKEY="B42F6819007F00F88E364FD4036A9C25BF357DD4"

RUN groupadd -g 9999 -r "${WOW_USER}" && useradd -u 9999 -r -g "${WOW_GROUP}" "${WOW_USER}"

RUN \
  set -ex; \
  apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates="${CA_CERTIFICATES_VERSION}" \
    wget="${WGET_VERSION}" \
    gpg="${GPG_VERSION}" \
    gpg-agent="${GPG_AGENT_VERSION}" \
    gpgconf="${GPG_CONF_VERSION}" \
    dirmngr="${DIRMNGR_VERSION}" \
    nano="${NANO_VERSION}" && \
  dpkgArch="$(dpkg --print-architecture | awk -F- '{ print $NF }')" && \
  if ! wget -O /usr/local/bin/gosu "https://github.com/tianon/gosu/releases/download/${GOSU_VERSION}/gosu-${dpkgArch}"; then \
    echo >&2 "Error: Failed to download Gosu binary for '${dpkgArch}'"; \
    exit 1; \
  fi && \
  if ! wget -O /usr/local/bin/gosu.asc "https://github.com/tianon/gosu/releases/download/${GOSU_VERSION}/gosu-${dpkgArch}.asc"; then \
    echo >&2 "Error: Failed to download transport armor file for Gosu - '${dpkgArch}'"; \
    exit 1; \
  fi && \
  export GNUPGHOME && \
  GNUPGHOME="$(mktemp -d)" && \
  for server in \
    hkp://p80.pool.sks-keyservers.net:80 \
    hkp://keyserver.ubuntu.com:80 \
    hkp://pgp.mit.edu:80 \
  ;do \
    echo "Fetching GPG key ${GOSU_GPGKEY} from $server"; \
    gpg --keyserver "$server" --recv-keys "${GOSU_GPGKEY}" && found=yes && break; \
  done && \
  gpg --batch --verify /usr/local/bin/gosu.asc /usr/local/bin/gosu && \
  rm -rf "${GNUPGHOME}" /usr/local/bin/gosu.asc && \
  chmod +x /usr/local/bin/gosu && \
  gosu nobody true && \
  apt-get purge -y --auto-remove ca-certificates wget gpg gpg-agent dirmngr && \
  rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential="${BUILD_ESSENTIAL_VERSION}" \
  make="${MAKE_VERSION}" \
  cmake="${CMAKE_VERSION}" \
  gcc="${GCC_VERSION}" \
  g++="${GPLUSPLUS_VERSION}" \
  automake="${AUTOMAKE_VERSION}" \
  autoconf="${AUTOCONF_VERSION}" \
  libmysql++-dev="${LIBMYSQLPLUSPLUS_DEV_VERSION}" \
  mysql-client="${MYSQL_CLIENT_VERSION}" \
  libtool="${LIBTOOL_VERSION}" \
  libssl1.0-dev="${LIBSSL_1_0_DEV_VERSION}" \
  binutils="${BINUTILS_VERSION}" \
  zlibc="${ZLIBC_VERSION}" \
  libc6="${LIBC_6_VERSION}" \
  libbz2-dev="${LIBBZ_2_DEV_VERSION}" \
  netcat="${NETCAT_VERSION}" \
  git="${GIT_VERSION}" \
  libtbb-dev="${LIBTBB_DEV_VERSION}" \
  libace-dev="${LIBACE_DEV_VERSION}"

RUN mkdir "${WOW_INSTALL}" \
  && chown "${WOW_USER}":"${WOW_GROUP}" "${WOW_INSTALL}"

# build server
COPY data/server/server.tar.gz "${WOW_HOME}"/
RUN mkdir -p "${WOW_HOME}"/server/build

RUN \
  set -ex && \
  tar xvzf "${WOW_HOME}"/server.tar.gz -C "${WOW_HOME}"/server && \
  rm -rf "${WOW_HOME}"/server.tar.gz

WORKDIR "${WOW_HOME}"/server/build

RUN \
  cmake ../ -DCMAKE_INSTALL_PREFIX="${WOW_INSTALL}" -DUSE_EXTRACTORS="${BUILD_EXTRACTORS}" && \
  make && \
  make install

COPY data/sql "${WOW_HOME}"/

RUN \
  set -ex && \
  tar xvzf "${WOW_HOME}"/data.tar.gz -C "${WOW_HOME}" && \
  rm -rf "${WOW_HOME}"/data.tar.gz

COPY config/mangosd.conf.tpl config/realmd.conf.tpl config/init_realm.tpl "${WOW_INSTALL}/etc/"

WORKDIR /

# add healthcheck script
COPY docker-healthcheck.sh /

# add launch script
COPY docker-entrypoint.sh /

RUN \
  chmod 755 /docker-entrypoint.sh && \
  chmod 755 /docker-healthcheck.sh

EXPOSE 8085 3724

VOLUME ["${WOW_LOG_DIR}"]

ENTRYPOINT ["/docker-entrypoint.sh"]
