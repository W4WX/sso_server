mysql: redis.cpp mysql.cpp
	g++  -std=c++11 `/usr/local/mariadb/bin/mysql_config --cflags --libs` -I/root/_test/libbcrypt/include -I/usr/local/mariadb/include -I/usr/include/mysql/include/jdbc -L/usr/local/lib  -lcpp_redis -ltacopie -L/usr/lib64/mysql/lib64 -Wl,-rpath=/usr/lib64/mysql/lib64 -lhiredis -lbcrypt -lmysqlcppconn mysql.cpp -o mysql

clean:
	rm mysql
