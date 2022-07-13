# How to reproduce
If you want to build and run the container, run `make`.\
If you prefer to pull the same image from DockerHub, run `docker run --rm -it erap320/issue-2870`.

The executed code can be found in [plugin.c](plugin.c), and produces the following error:
```
libyang[0]: Extension plugin "libyang 2 - Schema Mount, version 1": Invalid leafref value "test" - no existing target instance "/hardware/component/name". (path: Schema location "/ietf-hardware:hardware-state-oper-enabled/name", data location "/ietf-hardware:hardware-state-oper-enabled/name".)
```