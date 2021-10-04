FROM frolvlad/alpine-gcc

COPY main.cpp main.cpp
RUN apk add --no-cache g++ && /usr/bin/g++ main.cpp -o main

EXPOSE 80

CMD ./main