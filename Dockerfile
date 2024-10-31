FROM    alpine

RUN     apk update && apk add clang && apk add make

WORKDIR /app