#!/bin/bash
cd `dirname $0`
sudo git pull

sudo chown -R www-data:itest ../
sudo chown -R no_priv:no_priv superstar/db.bak
sudo chown -R no_priv:no_priv superstar/backups
sudo chown -R no_priv:no_priv superstar/logs
sudo chown -R no_priv:no_priv interactive_code/code_editor/logs
