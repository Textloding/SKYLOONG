#pragma once
#include "AppManagerLite.h"
class AppHome : public BaseApp
{
public:
    void init()
    {
        appid = 1;
    }
    void setup();
    void loop();
    void destroy();
};
class AppAPS : public BaseApp
{
public:
    void init()
    {
        appid = 2;
    }
    void setup();
    void loop();
    void destroy();
};

class AppGIF : public BaseApp
{
public:
    void init()
    {
        appid = 3;
    }
    void listFilesRecursively(char *basePath);
    void setup();
    void loop();
    void destroy();
    void stop(); 
};


class AppSettings : public BaseApp
{
public:
    void init()
    {
        appid = 3;
    }
    void setup();
    void loop();
    void destroy();
};