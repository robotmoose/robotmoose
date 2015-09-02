#!/bin/bash
cd `dirname $0`
sudo git pull
sudo chown -R www-data ../
sudo chgrp -R itest ../
sudo chown -R no_priv superstar/db.bak
sudo chgrp -R no_priv superstar/db.bak
sudo chown -R no_priv superstar/backups
sudo chgrp -R no_priv superstar/backups
sudo chown -R no_priv superstar/logs
sudo chgrp -R no_priv superstar/logs