#include "redis.h"

#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <memory>
#include <regex>

#include "bcrypt/BCrypt.hpp"

#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/driver.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/metadata.h"
#include "cppconn/exception.h"

// #include <cpp_redis/cpp_redis>
#include <hiredis/hiredis.h>

#include "DBPool.h"

#define DBHOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "PASSWORD" // TODO
#define DATABASE "test"

#define NUMOFFSET 100
#define COLNAME 200

using namespace std;
using namespace sql;
#pragma comment(lib, "mysqlcppconn.lib")

struct UserInfo
{
    unsigned id;     // 唯一标识id
    string userName; // 姓名
    string password; // 密码
    int status;      // 状态
};
struct LoginInfo
{
    string userName; // 姓名
    string password; // 密码
    string deviceID; // 设备ID
};

struct LoginHistory
{
    unsigned id;     // 唯一标识id
    string userName; // 姓名
    string deviceID; // 设备ID
    int status;      // 状态
};

struct Response
{
    int ret;
    string msg;
};

DBPool connpool = DBPool::GetInstance();

// void Demo();

// int main(int argc, char *argv[])
// {
//     Demo();

//     return 0;
// }

static UserInfo GetUserInfo(string userName)
{
    Statement *stmt;
    ResultSet *res;
    PreparedStatement *prep_stmt;
    Connection *con;
    string sql = "select `userID`, `userName`, `password`, `status` from t_signup where userName = ?";
    struct UserInfo userInfo;

    userInfo.id = 0;
    userInfo.userName = "";
    userInfo.password = "";
    userInfo.status = 0;

    int updatecount = 0;

    /* initiate url, user, password and database variables */
    const string database(DATABASE);

    try
    {
        con = connpool.GetConnection();

        /* alternate syntax using auto_ptr to create the db connection */
        //auto_ptr  con (driver -> connect(url, user, password));

        /* turn off the autocommit */
        con->setAutoCommit(0);

        cout << "\nDatabase connection/'s autocommit mode = " << con->getAutoCommit() << endl;

        /* select appropriate database schema */
        con->setSchema(database);

        cout << "Executing the Query: \"SELECT * FROM t_signup\" .." << endl;

        prep_stmt = con->prepareStatement(sql);

        prep_stmt->setString(1, userName);
        res = prep_stmt->executeQuery();

        while (res->next())
        {
            userInfo.userName = res->getString("userName");
            userInfo.password = res->getString("password");
            userInfo.id = res->getInt("userID");
            userInfo.status = res->getInt("status");
        }

        delete res;
        delete prep_stmt;
        connpool.ReleaseConnection(con);

        return userInfo;
    }
    catch (SQLException &e)
    {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047)
        {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
            */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        return userInfo;
    }
    catch (std::runtime_error &e)
    {
        cout << "ERROR: " << e.what() << endl;

        return userInfo;
    }
}

static int setUserInfo(UserInfo userInfo)
{
    Statement *stmt;
    ResultSet *res;
    PreparedStatement *prep_stmt;
    Connection *con;
    Savepoint *savept;
    string sql = "insert into  t_signup (`userName`, `password`, `status`) VALUES (?, ?, ?)";

    int updatecount = 0;

    /* initiate url, user, password and database variables */
    string url(DBHOST);
    const string user(USER);
    const string password(PASSWORD);
    const string database(DATABASE);

    try
    {
        con = connpool.GetConnection();

        /* alternate syntax using auto_ptr to create the db connection */
        //auto_ptr  con (driver -> connect(url, user, password));

        /* turn off the autocommit */
        con->setAutoCommit(0);

        cout << "\nDatabase connection/'s autocommit mode = " << con->getAutoCommit() << endl;

        /* select appropriate database schema */
        con->setSchema(database);

        cout << "Executing the sql: " + sql + " .." << endl;

        prep_stmt = con->prepareStatement(sql);

        prep_stmt->setString(1, userInfo.userName);
        prep_stmt->setString(2, userInfo.password);
        prep_stmt->setInt(3, userInfo.status);
        savept = con->setSavepoint("SAVEPT1");

        updatecount = prep_stmt->executeUpdate();

        con->rollback(savept);
        con->releaseSavepoint(savept);

        cout << "\tCommitting outstanding updates to the database .." << endl;
        con->commit();

        // delete res;
        delete prep_stmt;
        connpool.ReleaseConnection(con);
        return 0;
    }
    catch (SQLException &e)
    {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047)
        {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
            */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        return 1;
    }
    catch (std::runtime_error &e)
    {
        cout << "ERROR: " << e.what() << endl;

        return 1;
    }
}

// TODO
static int setRedisKey(string key, string value)
{
    Redis *r = new Redis();
    if (!r->connect("127.0.0.1", 6379))
    {
        printf("connect error!\n");
        return 1;
    }
    r->set(key, value);
    printf("set redis key: %s value: %s\n", key.c_str(), value.c_str());
    printf("Get the %s is %s\n", key.c_str(), r->get(key).c_str());
    delete r;
    return 0;
}

static int setLoginHistory(LoginHistory loginHis)
{
    Statement *stmt;
    PreparedStatement *prep_stmt;
    Connection *con;
    Savepoint *savept;
    string sql = "insert into  t_login_his (`userID`, `userName`, `deviceID`, `status`) VALUES (?, ?, ?, ?)";

    int updatecount = 0;

    /* initiate url, user, password and database variables */
    string url(DBHOST);
    const string user(USER);
    const string password(PASSWORD);
    const string database(DATABASE);

    try
    {
        con = connpool.GetConnection();

        /* alternate syntax using auto_ptr to create the db connection */
        //auto_ptr  con (driver -> connect(url, user, password));

        /* turn off the autocommit */
        con->setAutoCommit(0);

        cout << "\nDatabase connection/'s autocommit mode = " << con->getAutoCommit() << endl;

        /* select appropriate database schema */
        con->setSchema(database);

        cout << "Executing the sql: " + sql + " .." << endl;

        prep_stmt = con->prepareStatement(sql);

        prep_stmt->setInt(1, loginHis.id);
        prep_stmt->setString(2, loginHis.userName);
        prep_stmt->setString(3, loginHis.deviceID);
        prep_stmt->setInt(4, loginHis.status);
        savept = con->setSavepoint("setLoginHistory");

        updatecount = prep_stmt->executeUpdate();

        con->rollback(savept);
        con->releaseSavepoint(savept);

        cout << "\tCommitting outstanding updates to the database .." << endl;
        con->commit();

        delete prep_stmt;
        connpool.ReleaseConnection(con);
        return 0;
    }
    catch (SQLException &e)
    {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047)
        {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
            */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        return 1;
    }
    catch (std::runtime_error &e)
    {
        cout << "ERROR: " << e.what() << endl;

        return 1;
    }
}

static int checkLogin(LoginInfo l)
{

    if (l.userName == "" || l.password == "")
    {
        return 1;
    }

    UserInfo user = GetUserInfo(l.userName);

    if (BCrypt::validatePassword(l.password, user.password) == 1)
    {
        LoginHistory his;
        his.id = user.id;
        his.userName = user.userName;
        his.deviceID = l.deviceID;
        his.status = 0;
        setLoginHistory(his);
        // todo 更新redis token
        string key = "user_" + to_string(user.id);
        string value = to_string(rand()); // TODO 生成GUID
        setRedisKey(key, value);

        return 0;
    }
    cout << "password invalid..." << endl;
    return 1;
}

static LoginHistory getLoginHistory(string userID)
{
    Statement *stmt;
    ResultSet *res;
    PreparedStatement *prep_stmt;
    Connection *con;
    string sql = "select `userID`, `userName`, `deviceID`, `status` from t_login_his where userID = ?";
    struct LoginHistory loginHis;

    loginHis.id = 0;
    loginHis.userName = "";
    loginHis.deviceID = "";
    loginHis.status = 0;

    // if (userName == NULL) {
    //     return ;
    // }

    int updatecount = 0;

    /* initiate url, user, password and database variables */
    string url(DBHOST);
    const string user(USER);
    const string password(PASSWORD);
    const string database(DATABASE);

    try
    {
        con = connpool.GetConnection();

        /* alternate syntax using auto_ptr to create the db connection */
        //auto_ptr  con (driver -> connect(url, user, password));

        /* turn off the autocommit */
        con->setAutoCommit(0);

        cout << "\nDatabase connection/'s autocommit mode = " << con->getAutoCommit() << endl;

        /* select appropriate database schema */
        con->setSchema(database);

        cout << "Executing the sql: " + sql + " .." << endl;

        prep_stmt = con->prepareStatement(sql);

        prep_stmt->setString(1, userID);
        res = prep_stmt->executeQuery();

        while (res->next())
        {
            loginHis.userName = res->getString("userName");
            loginHis.deviceID = res->getString("deviceID");
            loginHis.id = res->getInt("userID");
            loginHis.status = res->getInt("status");
        }

        delete res;
        delete prep_stmt;
        connpool.ReleaseConnection(con);
        return loginHis;
    }
    catch (SQLException &e)
    {
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047)
        {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
            */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        return loginHis;
    }
    catch (std::runtime_error &e)
    {
        cout << "ERROR: " << e.what() << endl;

        return loginHis;
    }
}

/**
 * 注册
 */
static Response signupServer(string userName, string pwd)
{
    cout << "signupServer in..." << endl;
    Response res;
    res.ret = 0;
    res.msg = "ok";
    if (userName == "" && pwd == "")
    {
        cout << "signupServer in 0..." << endl;
        res.ret = 101;
        res.msg = "param error";
        return res;
    }
    cout << "signupServer in 1..." << endl;
    // regex eUser("[a-zA-Z]{8, 16}");
    // if (!std::regex_match(userName, eUser))
    // {
    //     res.ret = 102;
    //     res.msg = "userName not match request";
    //     return res;
    // }

    // cout << "signupServer in 2..." << endl;
    // regex ePwd("[\\S]{8, 16}");
    // if (!std::regex_match(pwd, ePwd))
    // {
    //     res.ret = 103;
    //     res.msg = "pwd not match complication";
    //     return res;
    // }
    UserInfo user;
    user = GetUserInfo(userName);

    cout << "GetUserInfo user:" << user.userName << endl;
    if (user.userName != "") {
        res.ret = 104;
        res.msg = "user has exist.";
        return res;
    }
    user.userName = userName;
    user.password = pwd;
    user.status = 0;

    res.ret = setUserInfo(user);

    if (res.ret != 0)
    {
        res.msg = "signup fail.";
    }

    return res;
}


static void initDBPool()
{
    connpool.initPool("tcp://127.0.0.1:3306", "root", PASSWORD, 100);
}

// void Demo()
// {
//     connpool.initPool("tcp://127.0.0.1:3306", "root", PASSWORD, 100);

//     UserInfo b;
//     b.userName = "123";
//     b.password = "456";
//     b.status = 0;

//     b.password = BCrypt::generateHash(b.password, 12);

//     setUserInfo(b);

//     UserInfo u = GetUserInfo("123");

//     cout << "userName: " << u.userName << endl;
//     cout << "password: " << u.password << endl;
//     cout << "userID: " << u.id << endl;

//     LoginInfo testUser;
//     testUser.userName = u.userName;
//     testUser.password = "456";
//     testUser.deviceID = "test deviceID";

//     int isLogin = checkLogin(testUser);
//     cout << "checking login: " << isLogin << endl;

//     return;
// }
