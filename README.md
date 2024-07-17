# clog - A logging library for C

`clog` is a threadsafe logging library that writes to STDOUT or a file.

The logger will only output levels of logging that are less than or equal to the level set, with `L_DEBUG` being the most verbose and `L_EMERG` being the least verbose.

## Features

- Follows Cisco syslog severity levels
- Supports logging to a file or to the console

## Example Usage:

```c
logger_init("log.txt", L_DEBUG);
clog(L_DEBUG, "This is a debug message");
clog(L_INFO, "This is an info message");
logger_destroy();
```