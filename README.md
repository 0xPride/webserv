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

### Serving a static website

we need a example of static website you can use any one you prefer for demonstration i will be using this repo as an example

```sh
git clone https://github.com/YaninaTrekhleb/restaurant-website.git website
```

Lets add a config to server this website

```
default website.com:1337 {
  route / {
    methods GET POST;
    root ./website/;
    index index.html;
  }
}
```

save this in a file i will be saving it in a file called website.conf

make sure that root directive points exactly where the folder of the repo you cloned

now server the webstie

```sh
./http website.conf
```
