killall ovsdb-server
service openvswitch-switch force-reload-kmod
ovsdb-server --remote =punix:/usr/local/var/run/openvswitch/db.sock \--pidfile --detach
ovs-vsctl --no-wait init
ovs-vswitchd --pidfile --detach
