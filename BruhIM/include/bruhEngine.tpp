#ifndef BRUHENGINE_TPP
#define BRUHENGINE_TPP

#include <libconfig.h++>

template <typename T>
T bruhEngine::lookupConfig(std::string cfgKey, libconfig::Config * config) {
    try
    {
        const libconfig::Setting& cfgRoot = config->getRoot();
        
        T lVal;

        if (!cfgRoot.lookupValue(cfgKey, lVal)) {
            std::cerr << "Couldn't find setting "<< cfgKey << " : " << lVal << "\n";
        }

        return lVal;
    }
    catch (const libconfig::SettingNotFoundException& nfex)
    {
        // Ignore.
    }
}

#endif