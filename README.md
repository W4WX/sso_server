# sso_server

- [x] 更新MySQL连接池
- [x] 连接redis
- [ ] 逻辑调整
- [ ] 生成GUID
- [ ] 接入GRPC







g++ `/usr/local/mariadb/bin/mysql_config --cflags --libs` -I/root/_test/libbcrypt/include -I/usr/local/mariadb/include -I/usr/include/mysql/include/jdbc -L/usr/local/lib  -lcpp_redis -ltacopie -L/usr/lib64/mysql/lib64 -Wl,-rpath=/usr/lib64/mysql/lib64 -lhiredis -lbcrypt -lmysqlcppconn  main.cpp DBPool.cpp -o main


g++ `/usr/local/mariadb/bin/mysql_config --cflags --libs` -I/root/_test/libbcrypt/include -I/usr/local/mariadb/include -I/usr/include/mysql/include/jdbc -L/usr/local/lib  -lcpp_redis -ltacopie -L/usr/lib64/mysql/lib64 -Wl,-rpath=/usr/lib64/mysql/lib64 -lhiredis -lbcrypt -lmysqlcppconn  main.cpp DBPool.cpp -o main








protoc -I protos --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` protos/sso.proto
protoc -I protos --cpp_out=. protos/sso.proto