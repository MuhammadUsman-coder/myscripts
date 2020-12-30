#!/bin/bash

for i in $(cat servers.txt);do
    ip=$(cut -d "|" -f2 <<< $i)
    servername=$(cut -d "|" -f1 <<< $i)
    now=$1
    after=$2
    response=$(timeout 20s ssh -o "StrictHostKeyChecking no" root@$ip "sed -i 's/'"${now}"'/'"${after}"'/g' /etc/iptables/rules.v4;iptables-restore < /etc/iptables/rules.v4")

    if [ "0" == "$?" ]; then
                echo "Successfully replaced $now with $after."
        else
                echo "Error occurred while deleting rule, this might mean server is not accessble."
        fi;
done
