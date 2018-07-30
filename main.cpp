/*
 * main.cpp
 *
 *  Created on: 2013-3-26
 *      Author: chenxun
 */

#include "DBPool.h"
#include <stdio.h>

/*--------------------------------------------------------------
    单例模式，全局唯一 db pool，程序中使用onnpool中获取一个
    db连接使用，使用完之后调用ReleaseConnection把conn放回pool中去.
----------------------------------------------------------------*/
DBPool  connpool = DBPool::GetInstance();

int main(int argc, char* argv[])
{
    //初始化连接，创建参数中maxSize一半的连接
    connpool.initPool("tcp://127.0.0.1:3306", "root", "toor", 100);     // TODO

    Connection *con;
    Statement *state;
    ResultSet *result;
    con = connpool.GetConnection();//get a db conn
    state = con->createStatement();
    state->execute("use test");

    // 查询
    result = state->executeQuery("select * from t_signup");

    // 输出查询
    while (result->next())
    {
        try{
            string user = result->getString("userName");
            string name = result->getString("password");
            cout << user << " : " << name << endl;
        }catch(sql::SQLException& e){
            std::cout << e.what() << std::endl;
        }
    }

    /*result = state->executeQuery("select cust_id,cust_name from customers");
    while (result->next())
    {
        try{
            string user = result->getString("cust_id");
            string name = result->getString("cust_name");
            cout << user << " : " << name << endl;
        }catch(sql::SQLException& e){
            std::cout << e.what() << std::endl;
        }
    }
*/


    delete result;
    delete state;
    connpool.ReleaseConnection(con);

    return 0;
}

// #include <iostream>
// #include <string>

// #include "CConnPool.h"

// using std::cout;
// using std::endl;
// using std::string;

// CConnPool *connpool = CConnPool::GetInstance();

// int main(int argc, char *argv[])
// {

//     connpool->InitConnpool("tcp://127.0.0.1:3306", "root", "toor", 100); // TODO

//     Connection *conn;
//     Statement *state;
//     ResultSet *result;

//     conn = connpool->GetConnection();
//     state = conn->createStatement();
//     state->execute("use test");

//     result = state->executeQuery("select * from t_signup");
//     while (result->next())
//     {
//         try
//         {
//             string user = result->getString("userName");
//             string host = result->getString("password");
//             cout << user << "@" << host << endl;
//         }
//         catch (sql::SQLException &e)
//         {
//             cout << e.what() << endl;
//         }
//     }

//     delete result;
//     delete state;
//     connpool->ReleaseConnection(conn);

//     return 0;
// }