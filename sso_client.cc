/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "sso.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using sso::SignupRequest;
using sso::LoginRequest;
using sso::SSOResponse;
using sso::Signup;

class SSOClient {
 public:
  SSOClient(std::shared_ptr<Channel> channel)
      : stub_(Signup::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string Signup(const std::string& user, const std::string& pwd) {
    // Data we are sending to the server.
    SignupRequest request;
    request.set_username(user);
    request.set_password(pwd);

    // Container for the data we expect from the server.
    SSOResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->signup(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.msg();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string Login(const std::string& user, const std::string& pwd, const std::string& deviceid) {
    // Data we are sending to the server.
    LoginRequest request;
    request.set_username(user);
    request.set_password(pwd);
    request.set_deviceid(deviceid);

    // Container for the data we expect from the server.
    SSOResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->login(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.msg();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Signup::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  SSOClient ssoclient(grpc::CreateChannel(
      "tx.mzozw.com", grpc::SslCredentials(grpc::SslCredentialsOptions())));
  std::string username("world");
  std::string password("123456");
  std::string reply = ssoclient.Signup(username, password);
  std::cout << "signup received: " << reply << std::endl;

  username = "world";
  password = "123456789";
  reply = ssoclient.Signup(username, password);
  std::cout << "signup received: " << reply << std::endl;

  username = "world";
  password = "12345678910";
  reply = ssoclient.Signup(username, password);
  std::cout << "signup received: " << reply << std::endl;

  username = "world";
  password = "123456789";
  std::string deviceID("android x");
  reply = ssoclient.Login(username, password, deviceID);
  std::cout << "Login received: " << reply << std::endl;

  password = "12345678910";
  reply = ssoclient.Login(username, password, deviceID);
  std::cout << "Login received: " << reply << std::endl;

  deviceID = "iphone x";
  password = "123456789";
  reply = ssoclient.Login(username, password, deviceID);
  std::cout << "Login received: " << reply << std::endl;

  return 0;
}
