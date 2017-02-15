#include "GameApplication.h"

#include "windows.h"


#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char *argv[])
    {
        // Create application object
        GameApplication app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {

            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);

            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;

        }

        return 0;
    }

#ifdef __cplusplus
}
#endif
