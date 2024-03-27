# http-1.1

this project is a basic implementation of the HTTP 1.1 protocol that supports pipelining and uploading files using chunks or multipart data

## Build Instructions

### dependencies
- Unix-compliant system
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

the syntax is inspired by Caddy server config file

4. run the server

```sh
./http config_file
```

### Serving a static website

we need an example of a static website you can use anyone you prefer for demonstration i will be using this repo as an example

```sh
git clone https://github.com/YaninaTrekhleb/restaurant-website.git website
```

Let's add a config to server this website

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

now server the website

```sh
./http website.conf
```

### Serving project with CGI (common gateway interface)

in this example, I will use WordPress as an example

1. download WordPress from their website
2. configure a server

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
directive an easy way to find its path is by running
```sh
which php-cgi
```

if you open the browser and go to `localhost:1337`
WordPress should be served
