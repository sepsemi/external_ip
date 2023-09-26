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

### Move it to the local bin directory to make it system wide.
mv external_ip /usr/local/bin/external_ip

### To run type
external_ip

### Example output (without debug)
```
/usr/local/bin/external_ip
142.151.125.127:57607
```

### Example output (with debug)
```
/usr/local/bin/external_ip
[DEBUG]: Connecting using: index=0, address=74.125.24.127, port=3478
[DEBUG]: Connected to 74.125.24.127:3478
[DEBUG]: Successfully send bind packet
142.151.125.127:57607
```

