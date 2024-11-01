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
    void setup();
    void loop();
    void destroy();
    void stop(); 
};


class AppJPG : public BaseApp
{
public:
    void init()
    {
        appid = 4;
    }
    void setup();
    void loop();
    void destroy();
    void stop(); 
};

class AppWeather : public BaseApp
{
public:
    void init()
    {
        appid = 5;
    }
    void setup();
    void loop();
    void destroy();
};

class AppSysinfo : public BaseApp
{
public:
    void init()
    {
        appid = 6;
    }
    void setup();
    void loop();
    void destroy();
};

class AppSettings : public BaseApp
{
public:
    void init()
    {
        appid = 50;
    }
    void setup();
    void loop();
    void destroy();
};

class AppQRCode : public BaseApp {
public:
    void init()
    {
        appid = 100;
    }
    void setup();
    void loop();
    void destroy();
};