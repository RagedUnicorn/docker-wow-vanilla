INSERT INTO realmlist (name) VALUES ("${realm_name}");
Update realmlist SET address = "${public_ip}" WHERE name = "${realm_name}"
