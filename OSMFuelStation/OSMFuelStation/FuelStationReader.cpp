#include "FuelStationReader.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDomDocument>

FuelStationReader::FuelStationReader(void)
{
}


FuelStationReader::~FuelStationReader(void)
{
}

vector<FuelStation*> FuelStationReader::GetLukoilJSON(const string& path) {
    static const QString jsonId = "Id";
    static const QString jsonNumber = "Number";
    static const QString jsonAddress = "Address";
    static const QString jsonIsFranchise = "IsFranchise";
    static const QString jsonLat = "Lat";
    static const QString jsonLng = "Lng";
    static const QString jsonFuelIds = "FuelIds";
    static const QString jsonFuelIdDiesel = "diesel";
    static const QString jsonFuelIdEktoDiesel = "ekto-diesel";
    static const QString jsonFuelId80 = "80";
    static const QString jsonFuelId92 = "92";
    static const QString jsonFuelId95 = "95";
    static const QString jsonFuelId98 = "98";
    static const QString jsonFuelId98EktoSport = "ekto-sport";
    static const QString jsonServiceIds = "ServiceIds";
    static const QString jsonOperator = "Operator";
    static const QString jsonStations = "Stations";
    QString stOperator;
    vector<FuelStation*> stations;
    FuelStation* station = NULL;
    QFile file(QString::fromUtf8( path.data(), path.size() ));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return stations;

    QString content = file.readAll();
    file.close();

    const QJsonDocument jsonDoc = QJsonDocument::fromJson(content.toUtf8());
    if (jsonDoc.isObject())
    {
        QJsonValue root = jsonDoc.object().value(jsonOperator);
        if (root.isString()) stOperator = root.toString();
        root = jsonDoc.object().value(jsonStations);
        if (root.isArray()) {
            const QJsonArray array = root.toArray(); // list of station
            for (QJsonArray::const_iterator i = array.constBegin(); i != array.constEnd(); ++i) {
                if ((*i).isObject()) {
                    station = new FuelStation();
                    stations.push_back(station);
                    const QJsonObject obj = (*i).toObject();
                    QJsonValue value = obj.value(jsonAddress);
                    if (value.isString()) {
                        QString str = value.toString();
                        station->mAddress = value.toString().toLocal8Bit();
                    }
                    value = obj.value(jsonId);
                    if (value.isDouble()) {
                        station->mRef = static_cast<int>(value.toDouble());
                    }
                    value = obj.value(jsonNumber);
                    if (value.isString()) {
                        station->mLocalRef = value.toString().toInt();
                    }
                    value = obj.value(jsonLat); // coordinates
                    if (value.isDouble()) {
                        station->mLat = value.toDouble();
                    }
                    value = obj.value(jsonLng);
                    if (value.isDouble()) {
                        station->mLon = value.toDouble();
                    }
                    value = obj.value(jsonFuelIds); // get fuel types
                    if (value.isArray()) {
                        const QJsonArray fuelIds = value.toArray();
                        for (QJsonArray::const_iterator j = fuelIds.constBegin(); j != fuelIds.constEnd(); ++j) {
                            if ((*j).isString()) {
                                const QString str = (*j).toString();
                                if (str == jsonFuelIdDiesel || str == jsonFuelIdEktoDiesel)
                                    station->mFuelTypes.insert(DIESEL);
                                else if (str == jsonFuelId80)
                                    station->mFuelTypes.insert(AI80);
                                else if (str == jsonFuelId92)
                                    station->mFuelTypes.insert(AI92);
                                else if (str == jsonFuelId95)
                                    station->mFuelTypes.insert(AI95);
                                else if (str == jsonFuelId98 || str == jsonFuelId98EktoSport)
                                    station->mFuelTypes.insert(AI98);
                            }
                        }
                    }
                    station->mBrand="������";
                    station->mOperator=stOperator.toLocal8Bit();
                }
            }
        }
    }
    return stations;
}

vector<FuelStation*> FuelStationReader::GetAllOSMXML(const string& path) {
    vector<FuelStation*> stations;
    FuelStation* station = NULL;
    QFile file(QString::fromUtf8( path.data(), path.size() ));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return stations;

    
    int errorLine, errorColumn;
    QString errorMsg;
    QDomDocument document;
    if (!document.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
            QString error("Syntax error line %1, column %2:\n%3");
            error = error
                    .arg(errorLine)
                    .arg(errorColumn)
                    .arg(errorMsg);
        return stations;
    }
    QDomElement rootElement = document.firstChild().toElement();
    for(QDomNode node = rootElement.firstChild(); !node .isNull(); node = node.nextSibling()) {
        station = new FuelStation();
        stations.push_back(station);
        QDomElement element = node.toElement();
        station->mLat = element.attribute("lat").toDouble();
        station->mLon = element.attribute("lon").toDouble();
        for(QDomNode tag = element.firstChild(); !tag.isNull(); tag = tag.nextSibling()) {
            QDomElement tagEle = tag.toElement();
            QString key = tagEle.attribute("k");
            QString value = tagEle.attribute("v");
            if (key == "operator") {
                station->mOperator = value.toLocal8Bit();
            }
            else if (key == "brand") {
                station->mBrand = value.toLocal8Bit();
            }
            else if (key == "ref") {
                station->mLocalRef = value.toInt();
            }
            else if (key == "name") {
                station->mName = value.toLocal8Bit();
            }
            else if (key == "fuel:diesel" && value == "yes") {
                station->mFuelTypes.insert(DIESEL);
            }
            else if (key == "fuel:octane_80" && value == "yes") {
                station->mFuelTypes.insert(AI80);
            }
            else if (key == "fuel:octane_92" && value == "yes") {
                station->mFuelTypes.insert(AI92);
            }
            else if (key == "fuel:octane_95" && value == "yes") {
                station->mFuelTypes.insert(AI95);
            }
            else if (key == "fuel:octane_98" && value == "yes") {
                station->mFuelTypes.insert(AI98);
            }
            else if (key == "fuel:lpg" && value == "yes") {
                station->mFuelTypes.insert(PETGAS);
            }
        }
    }
    return stations;
}

vector<FuelStation*> FuelStationReader::GetData(const string& func, const string& path) {
    if (func=="GetLukoilJSON")
        return GetLukoilJSON(path);    
    return vector<FuelStation*>();
}