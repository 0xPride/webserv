FROM frolvlad/alpine-gxx as build

EXPOSE 1337

WORKDIR /app/webserv

COPY . .

RUN apk update && apk upgrade

RUN apk add make

RUN make

FROM frolvlad/alpine-gxx

RUN apk update && apk upgrade

RUN apk add php82-cgi

COPY --from=build /app/webserv/webserv /usr/bin

RUN chmod 111 /usr/bin/webserv

RUN addgroup -S webserv && adduser -S webserv -G webserv

COPY --chown=webserv:webserv ./static /usr/share/webserv

USER webserv

RUN mkdir -p /tmp/webserv/cgi

CMD [ "webserv", "/etc/webserv/config.conf" ]
