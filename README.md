# socketRedisSentinel
tcp原生通信从redis的sentinel域名解析数据


#### 一、介绍
基于 Libevent 2.1.12 stable 和 OpenSSL 3.0 库开发。
```
root@typecodes:~# lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 22.04.3 LTS
Release:        22.04
Codename:       jammy
root@typecodes:~#
root@typecodes:~# openssl version
OpenSSL 3.0.2 15 Mar 2022 (Library: OpenSSL 3.0.2 15 Mar 2022)
root@typecodes:~#
```


#### 二、安装 libevent
ubuntu或者centos可以直接使用以下命令安装。安装后的库路径在`/usr/local/lib`。
```
## 安装命令
root@typecodes:~# wget -c https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
root@typecodes:~# tar -zxf libevent-2.1.12-stable.tar.gz
root@typecodes:~# cd libevent-2.1.12-stable/ && ./configure
root@typecodes:~/libevent-2.1.12-stable# make -j4 && make install
root@typecodes:~/libevent-2.1.12-stable# ldconfig

## 安装后的路径
root@typecodes:~/libevent-2.1.12-stable# ll /usr/local/lib
total 4804
drwxr-xr-x 1 root root     4096 Jul  2 15:07 ./
drwxr-xr-x 1 root root     4096 Mar 31  2017 ../
lrwxrwxrwx 1 root root       21 Jul  2 15:07 libevent-2.1.so.7 -> libevent-2.1.so.7.0.1*
-rwxr-xr-x 1 root root   960266 Jul  2 15:07 libevent-2.1.so.7.0.1*
-rw-r--r-- 1 root root  1341234 Jul  2 15:07 libevent.a
-rwxr-xr-x 1 root root      964 Jul  2 15:07 libevent.la*
lrwxrwxrwx 1 root root       21 Jul  2 15:07 libevent.so -> libevent-2.1.so.7.0.1*
lrwxrwxrwx 1 root root       26 Jul  2 15:07 libevent_core-2.1.so.7 -> libevent_core-2.1.so.7.0.1*
-rwxr-xr-x 1 root root   614923 Jul  2 15:07 libevent_core-2.1.so.7.0.1*
-rw-r--r-- 1 root root   882318 Jul  2 15:07 libevent_core.a
-rwxr-xr-x 1 root root      999 Jul  2 15:07 libevent_core.la*
lrwxrwxrwx 1 root root       26 Jul  2 15:07 libevent_core.so -> libevent_core-2.1.so.7.0.1*
lrwxrwxrwx 1 root root       27 Jul  2 15:07 libevent_extra-2.1.so.7 -> libevent_extra-2.1.so.7.0.1*
-rwxr-xr-x 1 root root   368014 Jul  2 15:07 libevent_extra-2.1.so.7.0.1*
-rw-r--r-- 1 root root   458990 Jul  2 15:07 libevent_extra.a
-rwxr-xr-x 1 root root     1006 Jul  2 15:07 libevent_extra.la*
lrwxrwxrwx 1 root root       27 Jul  2 15:07 libevent_extra.so -> libevent_extra-2.1.so.7.0.1*
lrwxrwxrwx 1 root root       29 Jul  2 15:07 libevent_openssl-2.1.so.7 -> libevent_openssl-2.1.so.7.0.1*
-rwxr-xr-x 1 root root    88594 Jul  2 15:07 libevent_openssl-2.1.so.7.0.1*
-rw-r--r-- 1 root root   111178 Jul  2 15:07 libevent_openssl.a
-rwxr-xr-x 1 root root     1035 Jul  2 15:07 libevent_openssl.la*
lrwxrwxrwx 1 root root       29 Jul  2 15:07 libevent_openssl.so -> libevent_openssl-2.1.so.7.0.1*
lrwxrwxrwx 1 root root       30 Jul  2 15:07 libevent_pthreads-2.1.so.7 -> libevent_pthreads-2.1.so.7.0.1*
-rwxr-xr-x 1 root root    19118 Jul  2 15:07 libevent_pthreads-2.1.so.7.0.1*
-rw-r--r-- 1 root root    12174 Jul  2 15:07 libevent_pthreads.a
-rwxr-xr-x 1 root root     1027 Jul  2 15:07 libevent_pthreads.la*
lrwxrwxrwx 1 root root       30 Jul  2 15:07 libevent_pthreads.so -> libevent_pthreads-2.1.so.7.0.1*
drwxr-xr-x 2 root root     4096 Jul  2 15:07 pkgconfig/
drwxrwsr-x 1 root staff    4096 Jun 12  2022 python2.7/
root@typecodes:~/libevent-2.1.12-stable#

## 安装后的头文件路径
root@typecodes:~/libevent-2.1.12-stable# ll /usr/local/include/
total 36
drwxr-xr-x 1 root root 4096 Jul  2 15:07 ./
drwxr-xr-x 1 root root 4096 Mar 31  2017 ../
-rw-r--r-- 1 root root 2019 Jul  2 15:07 evdns.h
-rw-r--r-- 1 root root 2744 Jul  2 15:07 event.h
drwxr-xr-x 2 root root 4096 Jul  2 15:07 event2/
-rw-r--r-- 1 root root 2035 Jul  2 15:07 evhttp.h
-rw-r--r-- 1 root root 2015 Jul  2 15:07 evrpc.h
-rw-r--r-- 1 root root 1782 Jul  2 15:07 evutil.h
root@typecodes:~/libevent-2.1.12-stable#


## 关于库路径的问题
If you ever happen to want to link against installed libraries
in a given directory, LIBDIR, you must either use libtool, and
specify the full pathname of the library, or use the '-LLIBDIR'
flag during linking and do at least one of the following:
   - add LIBDIR to the 'LD_LIBRARY_PATH' environment variable
     during execution
   - add LIBDIR to the 'LD_RUN_PATH' environment variable
     during linking
   - use the '-Wl,-rpath -Wl,LIBDIR' linker flag
   - have your system administrator add LIBDIR to '/etc/ld.so.conf'
```

#### 三、OpenSSL 命令生成证书
如下所示，使用openssl命令即可生成服务器签名证书。当然还可以追加参数`-days 365`来设置有效期，另外也可以去掉`-subj`参数改动手工输入。
```
vfhky@typecodes:~/bin/sentinel$ openssl req -newkey rsa:1024 -keyout server.key -out server.crt -nodes -subj "/C=CN/ST=GD/L=SZ/O=YY/OU=IT/CN=yy.com"
Generating a 1024 bit RSA private key
......................++++++
.++++++
writing new private key to 'server.key'
-----
vfhky@typecodes:~/bin/sentinel$

## 也可以生成X509格式的证书
vfhky@typecodes:~/bin/sentinel$ openssl req -x509 -newkey rsa:1024 -keyout server.key -out server.crt -nodes -subj "/C=CN/ST=GD/L=SZ/O=YY/OU=IT/CN=yy.com"
Generating a 1024 bit RSA private key
................................++++++
.........................................................................................++++++
writing new private key to 'server.key'
-----
vfhky@typecodes:~/bin/sentinel$
```

其中，`-subj`里面的具体参数说明：
```
Country Name (2 letter code)	:  The two-letter country code where your company is legally located.
State or Province Name (full name)	:  The state/province where your company is legally located.
Locality Name (e.g., city)	:  The city where your company is legally located.
Organization Name (e.g., company)	:  Your company's legally registered name (e.g., YourCompany, Inc.).
Organizational Unit Name (e.g., section)	:  The name of your department within the organization. (You can leave this option blank; simply press Enter.)
Common Name (e.g., server FQDN)	:  The fully-qualified domain name (FQDN) (e.g., www.example.com).
Email Address	:  Your email address. (You can leave this option blank; simply press Enter.)
A challenge password	:  Leave this option blank (simply press Enter).
An optional company name	:  Leave this option blank (simply press Enter).
```

