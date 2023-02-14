# ptyspawn
Tool for executing a command in a new PTY (pseudo-terminal) with new PGID and SID

## To get started:

* **Download the latest revision:**
```
git clone https://github.com/VHSgunzo/ptyspawn.git && cd ptyspawn
```

* **Build:**
```
make
```

* Or take an already precompiled binary file from the [releases](https://github.com/VHSgunzo/ptyspawn/releases)

* **Usage:**
```
./ptyspawn [ -vhV ] command {command args}
      -h     Print this help
      -V     Verbose
      -v     Version
```

* **You can also start a new session with logging to a file:**
```
./ptyspawn command {command args} | tee /path/to/file.log
```

* **Features:**
- [x] Spawn new full PTY session
- [x] Set new PGID and SID
- [x] Dynamically changing the size of the terminal along with the size of the window
- [x] Return the execution code when exiting

* **Exported environment variables:**
* `PTYSPAWN_PTY` - Slave PTY `(/dev/pts/X)`
* `PTYSPAWN_PID` - Self process id