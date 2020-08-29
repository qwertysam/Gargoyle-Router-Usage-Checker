#include "gargoyleparser.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

GargoyleParser::GargoyleParser()
{

}

bool GargoyleParser::update(QString url, QList<GargoyleProfile> profiles)
{
    // Set up network manager
    QNetworkAccessManager manager;
    manager.setTransferTimeout(timeout);

    // Set up request and reply
    QNetworkRequest request(url);
    QNetworkReply *reply(manager.get(request));

    // Wait for reply
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != 0)
    {
        qDebug("Network error while updating: %u", reply->error());
        return false;
    }

    // Set all profiles to not updated
    foreach (GargoyleProfile profile, profiles)
    {
        profile.updated = false;
        profile.deviceProfile = false;
    }

    // For each HTML line, extract the current IP and profile data
    QMap<uint64_t, Usage> rangeUsages;

    uint32_t currentIp;
    while(reply->canReadLine())
    {
        QString line = reply->readLine();

        bool isLimit = false;
        bool isUsed = false;

        // Quota info
        if (line.startsWith("quotaLimits", Qt::CaseInsensitive))
        {
            isLimit = true;
        }
        else if (line.startsWith("quotaUsed", Qt::CaseInsensitive))
        {
            isUsed = true;
        }
        // Get current device IP
        else if (line.startsWith("var connectedIp", Qt::CaseInsensitive))
        {
            int start = line.indexOf('"') + 1;
            int end = line.lastIndexOf('"');

            currentIp = parseIp(line.midRef(start, end - start));

            qDebug("Your IP: %d.%d.%d.%d", (currentIp >> 24) & 255, (currentIp >> 16) & 255, (currentIp >> 8) & 255, currentIp & 255);

            continue;
        }

        // Process quota info
        if (isLimit || isUsed)
        {
            // Remove extra characters and split into sections between brackets
            QVector<QStringRef> sections = squareBracketSections(cleanString(line));

            if (sections.size() == 3)
            {
                QVector<QStringRef> usages = sections[2].split(',');

                if (usages.size() == 3)
                {
                    QVector<QStringRef> range = sections[1].split('-');

                    uint32_t minIp, maxIp;
                    if (range.size() == 1)
                    {
                        minIp = parseIp(range[0]);
                        maxIp = minIp;
                    }
                    else if (range.size() == 2)
                    {
                        minIp = parseIp(range[0]);
                        maxIp = parseIp(range[1]);
                    }
                    else
                    {
                        continue;
                    }

                    uint64_t rangeKey = createIpRange(minIp, maxIp);
                    uint64_t data = usages[1].toULongLong();

                    if (rangeUsages.contains(rangeKey))
                    {
                        if (isLimit)
                            rangeUsages[rangeKey].max = data;
                        else if (isUsed)
                            rangeUsages[rangeKey].current = data;
                    }
                    else
                    {
                        Usage rangeUsage;

                        rangeUsage.minIp = minIp;
                        rangeUsage.maxIp = maxIp;

                        if (isLimit)
                            rangeUsage.max = data;
                        else if (isUsed)
                            rangeUsage.current = data;

                        rangeUsages[rangeKey] = rangeUsage;
                    }

                    if (isLimit)
                        qDebug() << "Range" << qUtf8Printable(sections[1].toString()) << "with limit" << data;
                    else if (isUsed)
                        qDebug() << "Range" << qUtf8Printable(sections[1].toString()) << "with usage" << data;
                }
            }
        }
    }

    foreach (Usage rangeUsage, rangeUsages)
    {
        bool foundProfile = false;
        foreach (GargoyleProfile profile, profiles)
        {
            if (profile.equals(rangeUsage.maxIp, rangeUsage.maxIp))
            {
                foundProfile = true;
                profile.setUsage(rangeUsage);
                break;
            }
        }

        // If no profile is found, make a new one
        if (!foundProfile)
        {
            profiles.append(GargoyleProfile(rangeUsage));
        }
    }

    // Set the device profile
    foreach (GargoyleProfile profile, profiles)
    {
        if (profile.containsIp(currentIp))
            profile.deviceProfile = true;
    }

    return true;
}

QVector<QStringRef> GargoyleParser::squareBracketSections(QString string)
{
    QVector<QStringRef> sections;

    int bracketStart = -1;
    for(int i = 0; i < string.length(); i++)
    {
        if (string[i] == '[')
        {
            bracketStart = i + 1;
        }
        else if (string[i] == ']' && bracketStart >= 0)
        {
            sections.append(string.midRef(bracketStart, i - bracketStart));
            bracketStart = -1;
        }
    }

    return sections;
}

QString GargoyleParser::cleanString(QString string)
{
    QString cleaned;

    foreach (QChar character, string)
    {
        if (character.isDigit() || character == '.' || character == '-' || character == ',' || character == '[' || character == ']')
            cleaned.append(character);
    }

    return cleaned;
}

uint32_t GargoyleParser::parseIp(QStringRef ip)
{
    uint32_t intIp = 0;

    foreach (QStringRef subnet, ip.split('.'))
    {
        // Shift by 8 bits for each byte
        intIp <<= 8;

        // Add this level of subnet
        intIp |= subnet.toInt();
    }

    return intIp;
}

uint64_t GargoyleParser::createIpRange(uint32_t minIp, uint32_t maxIp)
{
    uint64_t ipRange = maxIp;

    ipRange <<= 32;
    ipRange |= minIp;

    return ipRange;
}
