#include "gargoyleprofile.h"

GargoyleProfile::GargoyleProfile(Usage usage)
{
    // Set up the IP range
    minIp = usage.minIp;
    maxIp = usage.maxIp;

    // Set up the usage for the profile
    currentUsage = usage;
    setUsage(usage);
}

bool GargoyleProfile::operator==(GargoyleProfile profile) const noexcept
{
    return this->minIp == profile.minIp && this->maxIp == profile.maxIp;
}

bool GargoyleProfile::equals(uint32_t minIp, uint32_t maxIp) const
{
    return this->minIp == minIp && this->maxIp == maxIp;
}

bool GargoyleProfile::containsIp(uint32_t ip) const
{
    return ip >= minIp && ip <= maxIp;
}

void GargoyleProfile::setUsage(Usage usage)
{
    lastUsage = currentUsage;
    currentUsage = usage;

    usageDelta = lastUsage.current - currentUsage.current;
    timeDelta = lastUsage.time - currentUsage.time;

    updated = true;
}

Usage GargoyleProfile::getUsage() const
{
    return currentUsage;
}

Usage GargoyleProfile::getLastUsage() const
{
    return lastUsage;
}

int64_t GargoyleProfile::getUsageDelta() const
{
    return usageDelta;
}

int64_t GargoyleProfile::getTimeDelta() const
{
    return timeDelta;
}
