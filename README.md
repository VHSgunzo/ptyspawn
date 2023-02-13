# ptyspawn
Tool for executing a command in a new PTY (pseudo-terminal)

## To get started:

* **Download the latest revision**
```
git clone https://github.com/VHSgunzo/ptyspawn.git && cd ptyspawn
```

* **Build**
```
make
```

* Or take an already precompiled binary file from the [releases](https://github.com/VHSgunzo/ptyspawn/releases)

* **Usage**
```
./ptyspawn [ -d driver -einvrh ] command {command args}
      -h     Print this help
      -d     Set driver for stdin/stdout
      -e     Noecho for slave pty's line discipline
      -i     Ignore EOF on standard input
      -n     Not interactive
      -v     Verbose
      -r     Return exec code (enabled by default)
```
