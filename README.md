# external_ip
A Small STUN client that gets the Public IP Address of a client using multiple STUN servers for redundancy.

## Prerequisites
1. Linux
2. Git
3. GCC

### To compile the application run

git clone https://github.com/sepsemi/external_ip/

cd external_ip/

#### Without debug
gcc main.c -o external_ip;

#### With debug
gcc main.c -DDEBUG -o external_ip;

mv external_ip /usr/local/bin/external_ip
