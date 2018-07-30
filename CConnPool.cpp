/*
 * CConnPool.cpp
 *
 *  Created on: Mar 15, 2018
 *      Author: root
 */

#include <stdexcept>
#include <exception>
#include <cstdio>

#include "CConnPool.h"

CConnPool *CConnPool::connPool = NULL;
//CConnPool* CConnPool::connPool = new CConnPool();

CConnPool::CConnPool()
{
    // TODO Auto-generated constructor stub
}

void CConnPool::InitConnpool(string url, string user, string password,
                             int maxSize)
{
    this->maxSize = maxSize;
    this->curSize = 0;
    this->user = user;
    this->password = password;
    this->url = url;
    try
    {
        this->driver = sql::mysql::get_driver_instance();
    }
    catch (sql::SQLException &e)
    {
        perror("驱动连接出错;\n");
    }
    catch (std::runtime_error &e)
    {
        perror("运行出错了\n");
    }
    this->InitConnection(maxSize / 2);
    pthread_mutex_init(&lock, NULL);
}

CConnPool::CConnPool(string url, string user, string password, int maxSize)
{
    this->maxSize = maxSize;
    this->curSize = 0;
    this->user = user;
    this->password = password;
    this->url = url;
    try
    {
        this->driver = sql::mysql::get_driver_instance();
    }
    catch (sql::SQLException &e)
    {
        perror("驱动连接出错;\n");
    }
    catch (std::runtime_error &e)
    {
        perror("运行出错了\n");
    }
    this->InitConnection(maxSize / 2);
    pthread_mutex_init(&lock, NULL);
}

CConnPool *CConnPool::GetInstance()
{
    if (connPool == NULL)
        connPool = new CConnPool("tcp://127.0.0.1:3306", "root", "123456",
                                 10);
    return connPool;
}

void CConnPool::InitConnection(int num)
{
    Connection *conn;
    pthread_mutex_lock(&lock);
    for (int i = 0; i < num; ++i)
    {
        conn = CreateConnection();
        if (conn)
        {
            connList.push_back(conn);
            ++curSize;
        }
        else
        {
            perror("创建CONNECTION出错");
        }
    }
    pthread_mutex_unlock(&lock);
}

Connection *CConnPool::CreateConnection()
{
    Connection *conn;
    try
    {
        conn = driver->connect(url, user, password); //建立连接
        return conn;
    }
    catch (sql::SQLException &e)
    {
        perror(e.what());
        return NULL;
    }
    catch (std::runtime_error &e)
    {
        perror(e.what());
        return NULL;
    }
}

Connection *CConnPool::GetConnection()
{
    Connection *conn;
    pthread_mutex_lock(&lock);

    if (connList.size() > 0)
    {
        conn = connList.front();
        connList.pop_front();
        if (conn->isClosed())
        {
            delete conn;
            conn = CreateConnection();
        }
        if (conn == NULL)
            --curSize;
        pthread_mutex_unlock(&lock);
        return conn;
    }
    else
    {
        if (curSize < maxSize)
        {
            conn = CreateConnection();
            if (conn)
            {
                ++curSize;
                pthread_mutex_unlock(&lock);
                return conn;
            }
            else
            {
                pthread_mutex_unlock(&lock);
                return NULL;
            }
        }
        else
        {
            pthread_mutex_unlock(&lock);
            return NULL;
        }
    }
}

void CConnPool::ReleaseConnection(Connection *conn)
{
    if (conn)
    {
        pthread_mutex_lock(&lock);
        connList.push_back(conn);
        pthread_mutex_unlock(&lock);
    }
}

CConnPool::~CConnPool()
{
    this->DestoryConnPool();
}

void CConnPool::DestoryConnPool()
{
    list<Connection *>::iterator iter;
    pthread_mutex_lock(&lock);
    for (iter = connList.begin(); iter != connList.end(); ++iter)
        this->DestoryConnection(*iter);
    curSize = 0;
    connList.clear();
    pthread_mutex_unlock(&lock);
}

void CConnPool::DestoryConnection(Connection *conn)
{
    if (conn)
    {
        try
        {
            conn->close();
        }
        catch (sql::SQLException &e)
        {
            perror(e.what());
        }
        catch (std::exception &e)
        {
            perror(e.what());
        }
        delete conn;
    }
}