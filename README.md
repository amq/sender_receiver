[![build](https://img.shields.io/travis/amq/sender_receiver.svg)](https://travis-ci.org/amq/sender_receiver)
[![analysis](https://img.shields.io/coverity/scan/8805.svg)](https://scan.coverity.com/projects/amq-sender_receiver)

- POSIX IPC and semaphores
- Signal handling

Building
```
git clone https://github.com/amq/sender_receiver.git
cd sender_receiver
mkdir -p build
cd build
cmake ..
make
```

Usage
```
./sender -m <buffer_size> < data.txt
./receiver -m <buffer_size>
```

Manual cleanup after `SIGKILL`
```
find /dev/shm -user $(whoami) -name "sem.$(id -u)*" -type f -delete
```

Useful links
- https://github.com/angrave/SystemProgramming/wiki
- http://www.hildstrom.com/projects/ipc_sysv_posix/index.html
- http://mij.oltrelinux.com/devel/unixprg/
- http://man7.org/linux/man-pages/man7/shm_overview.7.html
- http://stackoverflow.com/questions/21311080/linux-shared-memory-shmget-vs-mmap
- https://gist.github.com/aspyct/3462238