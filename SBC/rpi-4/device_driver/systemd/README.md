## Running Radioberry 

The radioberry-SDR image is setup using a service in combination with a daemon process.

The service is installed in /etc/systemd/systemd/radioberry.service

The daemon is place in /etc/init.d/radioberryd


Now you have control to the radioberry by using systemctl functions.

sudo systemctl enable radioberry
sudo systemctl disable radioberry
sudo systemctl start radioberry
sudo systemctl stop radioberry
sudo systemctl restart radioberry
sudo systemctl status radioberry


Using:

sudo journalctl -u radioberry

gives you logging information.


Like to know the process id of the daemon:  sudo /bin/pidof radioberry 
gives you the process id.






