
#include "AppConfig.h"


using namespace mgp;

// Graphics
#define GP_GRAPHICS_WIDTH                           1280
#define GP_GRAPHICS_HEIGHT                          720
#define GP_GRAPHICS_FULLSCREEN                      false
#define GP_GRAPHICS_VSYNC                           true
#define GP_GRAPHICS_MULTISAMPLING                   1
#define GP_GRAPHICS_VALIDATION                      false

#define GP_ENGINE_NAME                  "mgp"
#define GP_ENGINE_VERSION_MAJOR         4
#define GP_ENGINE_VERSION_MINOR         0
#define GP_ENGINE_HOME_PATH             "./"
#define GP_ENGINE_CONFIG                "game.config"
#define GP_ENGINE_INPUT                 "game.input"
#define GP_ENGINE_MAGIC_NUMBER          { '\xAB', 'G', 'P', 'B', '\xBB', '\r', '\n', '\x1A', '\n' }

#define SPLASH_DURATION     2.0f

AppConfig::AppConfig():
    title(""),
    width(GP_GRAPHICS_WIDTH),
    height(GP_GRAPHICS_HEIGHT),
    fullscreen(GP_GRAPHICS_FULLSCREEN),
    vsync(GP_GRAPHICS_VSYNC),
    multisampling(GP_GRAPHICS_MULTISAMPLING),
    validation(GP_GRAPHICS_VALIDATION),
    homePath(GP_ENGINE_HOME_PATH),
    mainScene("main.scene")
{
}

AppConfig::~AppConfig()
{
}

Serializable* AppConfig::createObject()
{
    return new AppConfig();
}

std::string AppConfig::getClassName()
{
    return "mgp::AppConfig";
}

void AppConfig::onSerialize(Serializer* serializer)
{
    serializer->writeString("title", title.c_str(), "");
    serializer->writeInt("width", width, 0);
    serializer->writeInt("height", height, 0);
    serializer->writeBool("fullscreen", fullscreen, false);
    serializer->writeBool("vsync", vsync, false);
    serializer->writeInt("multisampling", (uint32_t)multisampling, 0);
    serializer->writeBool("validation", validation, false);
    serializer->writeString("homePath", homePath.c_str(), GP_ENGINE_HOME_PATH);
    serializer->writeList("splashScreens", splashScreens.size());
    for (size_t i = 0; i < splashScreens.size(); i++)
    {
        std::string splash = std::string(splashScreens[i].url) + ":" + std::to_string(splashScreens[i].duration);
        serializer->writeString(nullptr, splash.c_str(), "");
    }
    serializer->finishColloction();
    serializer->writeString("mainScene", mainScene.c_str(), "");
}

void AppConfig::onDeserialize(Serializer* serializer)
{
    serializer->readString("title", title, "");
    width = serializer->readInt("width", 0);
    height = serializer->readInt("height", 0);
    fullscreen = serializer->readBool("fullscreen", false);
    vsync = serializer->readBool("vsync", false);
    multisampling = serializer->readInt("multisampling", 0);
    validation = serializer->readBool("validation", false);
    serializer->readString("homePath", homePath, "");
    size_t splashScreensCount = serializer->readList("splashScreens");
    for (size_t i = 0; i < splashScreensCount; i++)
    {
        std::string splash;
        serializer->readString(nullptr, splash, "");
        if (splash.length() > 0)
        {
            SplashScreen splashScreen;
            size_t semiColonIdx = splash.find(':');
            if (semiColonIdx == std::string::npos)
            {
                splashScreen.url = splash;
                splashScreen.duration = SPLASH_DURATION;
            }
            else
            {
                splashScreen.url = splash.substr(0, semiColonIdx);
                std::string durationStr = splash.substr(semiColonIdx + 1, splash.length() - semiColonIdx);
                try
                {
                    splashScreen.duration = std::stof(durationStr);
                }
                catch (...)
                {
                    splashScreen.duration = SPLASH_DURATION;
                }
            }
            splashScreens.push_back(splashScreen);
        }
    }
    serializer->finishColloction();
    serializer->readString("mainScene", mainScene, "");
}