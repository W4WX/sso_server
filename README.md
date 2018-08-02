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


CREATE  PROCEDURE update_login (IN user_id INT(3),
                              IN user_name VARCHAR(50),
                              IN device_id VARCHAR(100),
                              IN status INT(30))
BEGIN
    UPDATE t_login_his SET status=1 WHERE userID=user_id and deviceID <> device_id;
    INSERT INTO t_login_his (`userID`, `userName`, `deviceID`, `status`) SELECT user_id, user_name, device_id, status FROM dual WHERE not exists(select `userID` from t_login_his as a where a.userID = user_id and a.deviceID = device_id);
    UPDATE t_login_his SET status=0 WHERE userID=user_id and deviceID = device_id;
END;