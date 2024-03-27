# http-1.1

This project is an exploration of the HTTP protocol by building a custom HTTP server in C++98. It provides a hands-on experience in understanding the intricacies of web communication, allowing users to test the server with a browser. The project covers both mandatory and bonus features, emphasizing non-blocking operations, configuration files, and CGI support.

## Build Instructions

### dependencies
- Unix-compliant system
- c++ compiler
- make
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

the syntax is inspired by Caddy server config file

4. run the server

```sh
./http config_file
```

### Serving a static website

0. we need an example of a static website you can use anyone you prefer for demonstration i will be using this repo as an example

```sh
git clone https://github.com/YaninaTrekhleb/restaurant-website.git website
```

1. let's add a config to server this website

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

make sure that the root directive points exactly where the folder of the repo you cloned

2. now serve the website

```sh
./http website.conf
```

### Serving project with CGI (common gateway interface)

in this example, I will use WordPress as an example

0. download WordPress from their website
1. configure a server

```
default example.com:1337 {
  route / {
    methods GET POST;
    root ./wordpress/;
    index index.php;
    cgi php /usr/bin/php-cgi;
  }
}
```

- make sure that the root points exactly to where you put WordPress
- make sure that php cgi is installed and write its path in the CGI

2. directive an easy way to find its path is by running

```sh
which php-cgi
```

3. if you open the browser and go to `localhost:1337`
WordPress should be served
