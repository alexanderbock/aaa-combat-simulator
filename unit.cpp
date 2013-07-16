#include "unit.h"

#include <math.h>
#include <QMessageBox>

#define CHECK(elem) \
    if (elem.isNull() || !elem.hasAttribute("value")) { \
        QMessageBox::critical(0, "XML Error", "XML format error in node '" + element.nodeName() + "/" + elem.nodeName() + "'"); \
        return; \
    }

const int ATTACKBITMASK(7);     // == 2^0 + 2^1 + 2^2
const int DEFENSEBITMASK(56);   // == 2^3 + 2^4 + 2^5
const int SUPPORTBITMASK(7);     // == 2^0 + 2^1 + 2^2
const int BOMBARDMENTBITMASK(56);   // == 2^3 + 2^4 + 2^5
const int AABITMASK(64);        // == 2^6
const int MARINEBITMASK(64)     // == 2^6
const int HITBITMASK(128);      // == 2^7

Unit::Unit(const QDomElement& element) {
    name_ = element.nodeName();

    QDomNodeList children = element.childNodes();
    for (int i = 0; i < children.count(); ++i) {
        const QDomNode& node = children.at(i);
        const QDomElement& childElem = node.toElement();

        QString key = childElem.nodeName();
        QString value;
        if (childElem.hasAttribute("value")) {
            value = childElem.attribute("value");
        }
        else {
            value = "";
        }

        if (key == "ID")
            id_ = value.toInt();
        else
            information_.insert(key, value);
    }
}

bool Unit::operator==(const Unit& rhs) const {
    return getID() == rhs.getID();
}

bool Unit::operator!=(const Unit& rhs) const {
    return !(*this == rhs);
}

int Unit::getID() const {
    return id_;
}

QString Unit::getName() const {
    return name_;
}

int Unit::getAttack() const {
    return information_.value("Attack").toInt();
}

int Unit::getDefense() const {
    return information_.value("Defense").toInt();
}

float Unit::getIPC() const {
    return information_.value("IPC").toFloat();
}

bool Unit::canAttack() const {
    return information_.contains("canAttack");
}

bool Unit::isAA() const {
    return information_.contains("isAA");
}

bool Unit::isArtillerySupportable() const {
    return information_.contains("isArtillerySupportable");
}

bool Unit::isArtillery() const {
    return information_.contains("isArtillery");
}

bool Unit::isAir() const {
    return information_.contains("isAir");
}

bool Unit::isSea() const {
    return information_.contains("isSea");
}

bool Unit::isLand() const {
    return !isSea() && !isAir();
}

bool Unit::canBombard() const {
    return information_.contains("canBombard");
}

bool Unit::isTwoHit() const {
    return information_.contains("isTwoHit");
}

bool Unit::isHit() const {
    return information_.value("isHit", "false") == "true";
}

void Unit::setHit() {
    information_["isHit"] = "true";
}

bool Unit::isDestroyer() const {
    return information_.contains("isDestroyer");
}

bool Unit::isSub() const {
    return information_.contains("isSub");
}

bool Unit::hasTwoRolls() const {
    return (getNumRolls() == 2);
}

int Unit::getNumRolls() const {
    return information_.value("NumRolls", "1").toInt();
}

int Unit::getBombardmentValue() const {
    return information_.value("Bombard", information_.value("Attack")).toInt();
}

int Unit::getNumArtillery() const {
    return information_.value("UnitSupportCount", "1").toInt();
}

bool Unit::isMarine() const {
    return information_.value("isMarine", "0").toBool();
}

QString Unit::getDescription() const {
    QString result;
    result += name_ + "\n";
    result += "\nAttack: " + information_["Attack"];
    result += "\nDefense: " + information_["Defense"];
    result += "\nIPC: " + information_["IPC"];
    foreach (const QString& key, information_.keys()) {
        if (key == "Attack" || key == "Defense" || key == "IPC")
            continue;

        result += "\n" + key;
        if (!information_[key].isEmpty())
            result += ": " + information_[key];
    }
    return result;
}

UnitLite::UnitLite(const Unit* const unit, int ipcFactor) {
    int id = unit->getID();
    int numRolls = unit->getNumRolls();
    if (id > 63)
        QMessageBox::critical(0, "XML Error", "A maximum number of 63 units is supported");
    if (numRolls > 3)
        QMessageBox::critical(0, "XML Error", "A maximum number of 3 rolls per unit is supported");
    numRollsAndID_ = numRolls + 4*id;

    int attackValue = unit->getAttack();
    int defenseValue = unit->getDefense();
    
    combatValue_ = (defenseValue << 3) + attackValue;
    combatValue2_ = (unit->getBombardmentValue() << 3) + unit->getNumArtillery();

    float ipc = unit->getIPC();
    ipc_ = static_cast<unsigned char>(ipc * ipcFactor);

    features_ = 0;

    if (unit->isArtillery())
        features_ |= FeaturesIsArtillery;
    if (unit->isArtillerySupportable())
        features_ |= FeaturesIsArtillerySupportable;
    if (unit->isTwoHit())
        features_ |= FeaturesIsTwoHit;
    if (unit->isAA())
        combatValue_ |= AABITMASK;
    if (unit->isAir())
        features_ |= FeaturesIsAir;
    if (unit->isSea())
        features_ |= FeaturesIsSea;
    if (unit->canBombard())
        features_ |= FeaturesCanBombard;
    if (unit->isDestroyer())
        features_ |= FeaturesIsDestroyer;
    if (unit->isSub())
        features_ |= FeaturesIsSub;
    if (unit->isMarine())
        combatValue2_ |= MARINEBITMASK;
}

bool UnitLite::operator==(const UnitLite& rhs) const {
    return (this->getID() == rhs.getID());
}

bool UnitLite::operator!=(const UnitLite& rhs) const {
    return !(*this == rhs);
}

int UnitLite::getID() const {
    return numRollsAndID_ / 4;
}

int UnitLite::getAttack() const {
    return combatValue_ & ATTACKBITMASK;
}

int UnitLite::getDefense() const {
    return (combatValue_ & DEFENSEBITMASK) >> 3;
}

float UnitLite::getIPC() const {
    return ipc_;
}

bool UnitLite::isAA() const {
    return combatValue_ & AABITMASK;
}

bool UnitLite::isArtillerySupportable() const {
    return features_ & FeaturesIsArtillerySupportable;
}

bool UnitLite::isArtillery() const {
    return features_ & FeaturesIsArtillery;
}

bool UnitLite::isAir() const {
    return features_ & FeaturesIsAir;
}

bool UnitLite::isSea() const {
    return features_ & FeaturesIsSea;
}

bool UnitLite::isLand() const {
    return !(features_ & (FeaturesIsSea | FeaturesIsAir));
}

bool UnitLite::canBombard() const {
    return features_ & FeaturesCanBombard;
}

bool UnitLite::isTwoHit() const {
    return features_ & FeaturesIsTwoHit;
}

bool UnitLite::isHit() const {
    return combatValue_ & HITBITMASK;
}

void UnitLite::setHit() {
    combatValue_ |= HITBITMASK;
}

bool UnitLite::isDestroyer() const {
    return features_ & FeaturesIsDestroyer;
}

bool UnitLite::isSub() const {
    return features_ & FeaturesIsSub;
}

bool UnitLite::hasTwoRolls() const {
    return getNumRolls() == 2;
}

int UnitLite::getNumRolls() const {
    return numRollsAndID_ % 4;
}

int UnitLite::getBombardmentValue() const {
    return combatValue2_ & BOMBARDMENTBITMASK;
}

int UnitLite::getNumArtillery() const {
    return combatValue2_ & SUPPORTBITMASK;
}

bool UnitLite::isMarine() const {
    return combatValue2_ & MARINEBITMASK;
}
