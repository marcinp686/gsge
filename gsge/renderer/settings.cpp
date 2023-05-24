#include "settings.h"

Settings::Settings()
{
}

Settings &Settings::getInstance()
{
	static Settings instance;
    return instance;    
}
