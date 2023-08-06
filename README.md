# socketRedisSentinel
tcp原生通信从redis的sentinel域名解析数据



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
