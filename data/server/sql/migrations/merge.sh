#!/bin/bash

scriptPath="$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"

merge() {
    local prefix=$1
    local file_path="${prefix}_db_updates.sql"

    [ -f "${file_path}" ] && rm -f "${file_path}"
    find "${scriptPath}" -type f -name "*_${prefix}.sql" | sort -n | xargs -I f cat f >> "${file_path}"
}

merge "characters"
merge "logon"
merge "logs"
merge "world"

find "${scriptPath}" -type f -size 0 -delete
