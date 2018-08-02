#include "redis.h"

#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <memory>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "bcrypt/BCrypt.hpp"

#include <regex.h>
#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/driver.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/metadata.h"
#include "cppconn/exception.h"

// #include <cpp_redis/cpp_redis>
// #include <hiredis/hiredis.h>

#include "DBPool.h"

#define DBHOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "toor" // TODO
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
    int ret = 0;
    string msg = "OK";
    string data;
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
        // con->setSchema(database);

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

static string getUUID (){
    boost::uuids::uuid u = boost::uuids::random_generator()();

    return to_string(u);
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
        // con->setSchema(database);

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
    r->expire(key, 900);
    // redisCommand(r, "expire %s 900", key);
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
    // "insert into  t_login_his (`userID`, `userName`, `deviceID`, `status`) VALUES (?, ?, ?, ?)";

    // string sql = "INSERT INTO t_login_his (`userID`, `userName`, `deviceID`, `status`) SELECT ?, ?, ?, ? FROM dual WHERE not exists(select `userID` from t_login_his where userID = ? and deviceID = ?);select * from t_login_his;";

    string sql = "call update_login(?, ?, ?, ?)";

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
        // con->setSchema(database);

        cout << "Executing the sql: " + sql + " .." << endl;

        prep_stmt = con->prepareStatement(sql);

        int ret;
        // prep_stmt->setInt(1, loginHis.id);
        // prep_stmt->setString(2, loginHis.deviceID);

        prep_stmt->setInt(1, loginHis.id);
        prep_stmt->setString(2, loginHis.userName);
        prep_stmt->setString(3, loginHis.deviceID);
        prep_stmt->setInt(4, loginHis.status);
        // prep_stmt->setInt(5, loginHis.id);
        // prep_stmt->setString(6, loginHis.deviceID);
        // prep_stmt->setInt(7, loginHis.id);
        // prep_stmt->setString(8, loginHis.deviceID);
        savept = con->setSavepoint("setLoginHistory");

        updatecount = prep_stmt->executeUpdate();

        con->rollback(savept);
        con->releaseSavepoint(savept);

        cout << "\tCommitting outstanding updates to the database .." << endl;
        con->commit();

        // res.reset(prep_stmt->executeQuery("SELECT @ret AS _ret"));
        // while (res->next())
        //     cout << "procedure ret: "
        //          << res->getString("_ret") << endl;

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

static Response checkLogin(LoginInfo l)
{
    Response res;
    if (l.userName == "" || l.password == "")
    {
        res.ret = 1;
        return res;
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
        // 更新redis token
        string key = "user_" + to_string(user.id);
        string value = getUUID(); // TODO 生成GUID
        setRedisKey(key, value);

        res.ret = 0;
        res.data = value;
        return res;
    }
    cout << "password invalid..." << endl;
    res.ret = 1;
    return res;
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
        // con->setSchema(database);

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

    regex_t reg;                                                                //定义一个正则实例
    const char *pattern = "^[a-zA-Z]{4,16}$";                                   //定义模式串
    regcomp(&reg, pattern, REG_EXTENDED);                                       //编译正则模式串

    const size_t nmatch = 1;                            //定义匹配结果最大允许数
    regmatch_t pmatch[1];                               //定义匹配结果在待匹配串中的下标范围
    int status = regexec(&reg, userName.c_str(), nmatch, pmatch, 0); //匹配他


    cout << "signupServer in 2..." << endl;
    if (status == REG_NOMATCH)
    {
        cout << "signupServer in 3..." << endl;
        res.ret = 102;
        res.msg = "userName not match request";
        return res;
    }

    regex_t regPwd;
    const char *patternPwd = "^.{8,16}$";    //定义模式串
    regmatch_t pmatchPwd[1];
    regcomp(&regPwd, patternPwd, REG_EXTENDED);  //编译正则模式串

    status = regexec(&regPwd, pwd.c_str(), nmatch, pmatchPwd, 0); //匹配他

    if (status == REG_NOMATCH)
    {
        cout << "pwd: " << pwd << endl;
        res.ret = 103;
        res.msg = "pwd not match complication";
        return res;
    }
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

    user.password = BCrypt::generateHash(user.password, 12);

    res.ret = setUserInfo(user);

    if (res.ret != 0)
    {
        res.msg = "signup fail.";
    }

    return res;
}

static Response loginServer(string userName, string pwd, string device){
    cout << "loginServer in..." << endl;
    Response res;
    res.ret = 0;
    res.msg = "ok";

    LoginInfo info;
    info.userName = userName;
    info.password = pwd;
    info.deviceID = device;

    Response isLogin = checkLogin(info);

    if (isLogin.ret == 1)
    {
        res.ret = 106;
        res.msg = "login fail.";
        return res;
    }

    return isLogin;
}


static void initDBPool()
{
    connpool.initPool(DBHOST, USER, PASSWORD, DATABASE, 100);
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
