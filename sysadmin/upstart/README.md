On systems which use the Upstart init system (in our case, Ubuntu 14.04 LTS), putting these scripts in `/etc/init/` will allow you to run Superstar as a standard service, and use the standard service management commands to interact with them. For example:

	service superstar start
	service superstar stop
	service superstar status

On more recent Ubuntu versions (>= 15.04) and other modern Linux distributions, systemd is used instead, and these will not work. Most pre-systemd non-Ubuntu Linux distributions and other Unixes use SysVinit, where this won't work either.

I'll write init scripts for systemd and SysVinit at some point, but they're relatively low-priority at the moment.
