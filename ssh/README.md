# SSH

Perform lateral movement via ssh. Login into server, run commands, and transfer files via ssh. 

## To Run 

```
loader --load scythe.ssh
# To Check Credentials
scythe.ssh --host 192.168.2.2 --password letmein --user admin
# To Execute a Command on a remote server
scythe.ssh --host 192.168.2.2 --password letmein --user admin --command whoami; ip a;
# To Transfer a file from VFS to remote host
scythe.ssh --host 192.168.2.2 --password letmein --user admin --filename VFS:/users/BUILTIN/scythe/agent.out --dirpath /tmp # dirpath is the directory path on remote system.
# Or to transfer a file from VFS to remote host
scythe.ssh --host 192.168.2.2 --password letmein --user admin --filename /home/developer/Documents/agent.out --dirpath /tmp # dirpath is the directory path on remote system.
```

## Notes:

- SFTP is only supported for remote hosts, currently, you can not transfer file from remote host to localhost. 
- SSH only uses password authentication for now, however, GSSAPI, and Private Key files will be supported soon. 

## Libraries Used:
- [libssh](https://api.libssh.org/stable/index.html)
- [nxjson](https://github.com/thestr4ng3r/nxjson)
