# http-1.1

this project is a basic implementation of the http 1.1 protocol that support pipelining and uploading files using chunks or multipart data

## Build Instructions

### dependencies
- unix complient system
- c++ compiler
- php cgi ( optional )

### Instructions

0. clone the repository

```sh
git clone https://github.com/0xhoussam/http-1.1.git && cd http-1.1
```

1. create the following directories

```sh
mkdir -p /tmp/webserv/cgi
```

2. compile the project

```sh
make
```

3. create a config file

```
default test.com:1337 {
  route / {
    methods GET;
    root ./static/;
    index index.html;
  }
}
```

the syntax is inspired by caddy server config file

4. run the server

```sh
./http config_file
```
