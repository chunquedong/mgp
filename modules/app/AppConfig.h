#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include "base/Base.h"
#include "base/Ptr.h"
#include "platform/Toolkit.h"
#include "base/Serializable.h"

namespace mgp
{
    /**
     * Defines a splash screen.
     */
    class SplashScreen
    {
    public:
        std::string url;
        float duration;
    };

    /**
     * Application configuration.
     */
    class AppConfig : public Serializable, public Refable
    {
    public:

        /**
         * Constructor
         */
        AppConfig();

        /**
         * Destructor
         */
        ~AppConfig();

        /**
         * @see Serializable::getClassName
         */
        std::string getClassName();

        /**
         * @see Serializable::onSerialize
         */
        void onSerialize(Serializer* serializer);

        /**
         * @see Serializable::onDeserialize
         */
        void onDeserialize(Serializer* serializer);

        /**
         * @see Activator::createObject
         */
        static Serializable* createObject();

        std::string title;
        int width;
        int height;
        bool fullscreen;
        bool vsync;
        size_t multisampling;
        bool validation;
        std::string homePath;
        std::vector<SplashScreen> splashScreens;
        std::string mainScene;
    };
}

#endif
