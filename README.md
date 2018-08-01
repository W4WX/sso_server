# sso_server

- [x] 更新MySQL连接池
- [x] 连接redis
- [ ] 逻辑调整
- [ ] 生成GUID
- [ ] 接入GRPC

g++  -std=c++11 `/usr/local/mariadb/bin/mysql_config --cflags --libs` -I/root/_test/libbcrypt/include -I/usr/local/mariadb/include -I/usr/include/mysql/include/jdbc -L/usr/local/lib  -lcpp_redis -ltacopie -L/usr/lib64/mysql/lib64 -Wl,-rpath=/usr/lib64/mysql/lib64 -lhiredis -lbcrypt -lmysqlcppconn mysql.cpp DBPool.cpp -o mysql

g++ -std=c++11 -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -I/usr/include -lgrpc -lgpr  -lgrpc++_reflection -lprotobuf -ldl -lpthread -lprotoc -lprotobuf-lite sso.grpc.pb.cc sso.pb.cc  sso_server.cc -o sso_server

g++ -std=c++11 -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -I/usr/include -lgrpc -lgpr  -lgrpc++_reflection -lprotobuf -ldl -lpthread -lprotoc -lprotobuf-lite sso.grpc.pb.cc sso.pb.cc sso_client.cc -o sso_client

g++  -std=c++11 `/usr/local/mariadb/bin/mysql_config --cflags --libs` `pkg-config --libs protobuf grpc++ grpc` -I/root/_test/libbcrypt/include -I/usr/local/mariadb/include -I/usr/include/mysql/include/jdbc -L/usr/local/lib  -lcpp_redis -ltacopie -L/usr/lib64/mysql/lib64 -Wl,-rpath=/usr/lib64/mysql/lib64 -lhiredis -lbcrypt -lmysqlcppconn -lgrpc -lgpr  -lgrpc++_reflection -lprotobuf -ldl -lpthread -lprotoc -lprotobuf-lite DBPool.cpp sso.grpc.pb.cc sso.pb.cc  sso_server.cc -o sso_server

g++ -std=c++11 -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -I/usr/include -lgrpc -lgpr  -lgrpc++_reflection -lprotobuf -ldl -lpthread -lprotoc -lprotobuf-lite sso.grpc.pb.cc sso.pb.cc  sso_server.cc -o sso_server

protoc -I protos --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` protos/sso.proto
protoc -I protos --cpp_out=. protos/sso.proto