#ifndef CHEATING_CONFIG_H
#define CHEATING_CONFIG_H

#define ALLOW_CHEATING_MODE 1

#if ALLOW_CHEATING_MODE

constexpr int cheatingPort = 65535;

namespace __cheating
{
    inline bool &__getCheatingModeRef()
    {
        static bool __cheatingModeEnabled = false;
        return __cheatingModeEnabled;
    }
}

inline bool isCheatingModeEnabled()
{
    return __cheating::__getCheatingModeRef();
}

inline void setCheatingModeEnabled(bool isEnabled)
{
    __cheating::__getCheatingModeRef() = isEnabled;
}

#endif

#endif // CHEATING_CONFIG_H
