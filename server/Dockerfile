FROM frolvlad/alpine-gcc

COPY . .
RUN apk add --no-cache g++ && /usr/bin/g++ main.cpp -o main

EXPOSE 80

CMD ./main