#!/bin/bash
# @author Michael Wiesendanger <michael.wiesendanger@gmail.com>
# @description launch script for wow_vanilla_server

set -euo pipefail

WD="${PWD}"

# get secrets from database container
mysql_app_user="/run/secrets/com.ragedunicorn.mysql.app_user"
mysql_app_password="/run/secrets/com.ragedunicorn.mysql.app_user_password"
database_hostname="${DATABASE_HOSTNAME:?localhost}"
database_port="3306"

function check_mysql_status {
  i=0

  while ! nc "${database_hostname}" "${database_port}" >/dev/null 2>&1; do
    i=$((i + 1))
    if [ "${i}" -ge 50 ]; then
      echo "$(date) [ERROR]: Mysql-Service not reachable aborting..."
      exit 1
    fi
    echo "$(date) [INFO]: Waiting for TCP connection to ${database_hostname}:${database_port}..."
    sleep 5
  done
  echo "$(date) [INFO]: Mysql connection established"
}

function create_log_dir {
  echo "$(date) [INFO]: Creating log directory ${WOW_LOG_DIR} and setting permissions"
  mkdir -p "${WOW_LOG_DIR}"
  chown -R "${WOW_USER}":"${WOW_GROUP}" "${WOW_LOG_DIR}"
}

# setup mangos database
function prepare_database {
  echo "$(date) [INFO]: Launching initial database setup ..."
  echo "$(date) [INFO]: Creating databases"
  # create databases
  mysql -u"${mysql_app_user}" -p"${mysql_app_password}" -h "${database_hostname}" < "${WOW_HOME}/init.sql"
  echo "$(date) [INFO]: Setup characters database"
  # init characters database
  mysql -u"${mysql_app_user}" -p"${mysql_app_password}" -h "${database_hostname}" characters < "${WOW_HOME}/server/sql/characters.sql"
  echo "$(date) [INFO]: Setup logs database"
  # init logs database
  mysql -u"${mysql_app_user}" -p"${mysql_app_password}" -h "${database_hostname}" logs < "${WOW_HOME}/server/sql/logs.sql"
  echo "$(date) [INFO]: Setup realmd database"
  # init logon database
  mysql -u"${mysql_app_user}" -p"${mysql_app_password}" -h "${database_hostname}" realmd < "${WOW_HOME}/server/sql/logon.sql"

  # init realm
  realm_name="${REALM_NAME:?Missing environment variable REALM_NAME}"
  public_ip="${PUBLIC_IP:?Missing environment variable PUBLIC_IP}"

  echo "$(date) [INFO]: Initialize configured realm"
  sed \
    -e "s/\${realm_name}/${realm_name}/" \
    -e "s/\${public_ip}/${public_ip}/" \
    "${WOW_INSTALL}/etc/init_realm.tpl" | tee "${WOW_HOME}/init_realm.sql"

  mysql -u"${mysql_app_user}" -p"${mysql_app_password}" -h "${database_hostname}" realmd < "${WOW_HOME}/init_realm.sql"
  echo "$(date) [INFO]: Setup world database"
  # init world database
  mysql -u"${mysql_app_user}" -p"${mysql_app_password}" -h "${database_hostname}" mangos < "${WOW_HOME}/world_full.sql"
  echo "$(date) [INFO]: Creating migrations"
  # create full migrations
  cd "${WOW_HOME}/server/sql/migrations/"
  ./merge.sh

  cd "${WD}"

  echo "$(date) [INFO]: Loading migrations"
  # update world with migration
  mysql -u"${mysql_app_user}" -p"${mysql_app_password}" -h "${database_hostname}" mangos < "${WOW_HOME}/server/sql/migrations/world_db_updates.sql"
  echo "$(date) [INFO]: Database setup done"
}

function check_database_setup {
  databases=("mangos" "realmd" "characters" "logs")

  for i in "${databases[@]}"; do
    if ! mysql -u"${mysql_app_user}" -p"${mysql_app_password}" -h "${database_hostname}" -e "USE ${i}" > /dev/null 2>&1; then
      prepare_database
      break
    fi
  done
}

function setup_configuration {
  if [ -f "${mysql_app_user}" ] && [ -f "${mysql_app_password}" ]; then
    echo "$(date) [INFO]: Found docker secrets - using secrets to setup configuration"

    mysql_app_user="$(cat ${mysql_app_user})"
    mysql_app_password="$(cat ${mysql_app_password})"
  else
    echo "$(date) [INFO]: No docker secrets found - using environment variables"

    mysql_app_user="${MYSQL_APP_USER:?Missing environment variable MYSQL_APP_USER}"
    mysql_app_password="${MYSQL_APP_PASSWORD:?Missing environment variable MYSQL_APP_PASSWORD}"
  fi

  sed \
    -e "s/\${wow_database_user}/${mysql_app_user}/" \
    -e "s/\${wow_database_user_password}/${mysql_app_password}/" \
    -e "s/\${database_hostname}/${database_hostname}/" \
    "${WOW_INSTALL}/etc/mangosd.conf.tpl" | tee "${WOW_INSTALL}/etc/mangosd.conf"

  sed \
    -e "s/\${wow_database_user}/${mysql_app_user}/" \
    -e "s/\${wow_database_user_password}/${mysql_app_password}/" \
    -e "s/\${database_hostname}/${database_hostname}/" \
    "${WOW_INSTALL}/etc/realmd.conf.tpl" | tee "${WOW_INSTALL}/etc/realmd.conf"

  echo "$(date) [INFO]: Finished setup for realmd and mangosd"
}

function launch_server {
  echo "$(date) - Launching realmd"
  # start realm in background
  cd "${WOW_INSTALL}"/bin
  exec gosu "${WOW_USER}" ./realmd &
  echo "$(date) - Launching mangosd"
  # start world in foreground preventing docker container from exiting
  exec gosu "${WOW_USER}" ./mangosd
}

function init {
  setup_configuration
  create_log_dir
  check_mysql_status
  check_database_setup
  launch_server
}

init
