/**************************************************************************************************
 *                                                                                                *
 * AAA Combat Simulator                                                                           *
 *                                                                                                *
 * Copyright (c) 2011 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *                                                                                                *
 *************************************************************************************************/

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
const int MARINEBITMASK(64);    // == 2^6
const int HITBITMASK(128);      // == 2^7

Unit::Unit(const QDomElement& element) {
    _name = element.nodeName();

    QDomNodeList children = element.childNodes();
    for (int i = 0; i < children.count(); ++i) {
        const QDomNode& node = children.at(i);
        const QDomElement& childElem = node.toElement();

        QString key = childElem.nodeName();
        QString value;
        if (childElem.hasAttribute("value"))
            value = childElem.attribute("value");
        else
            value = "";

        if (key == "ID")
            _id = value.toInt();
        else
            _information.insert(key, value);
    }
}

bool Unit::operator==(const Unit& rhs) const {
    return id() == rhs.id();
}

bool Unit::operator!=(const Unit& rhs) const {
    return !(*this == rhs);
}

int Unit::id() const {
    return _id;
}

QString Unit::name() const {
    return _name;
}

int Unit::attackValue() const {
    return _information.value("Attack").toInt();
}

int Unit::defenseValue() const {
    return _information.value("Defense").toInt();
}

float Unit::ipcValue() const {
    return _information.value("IPC").toFloat();
}

bool Unit::canAttack() const {
    return _information.contains("canAttack");
}

bool Unit::isAA() const {
    return _information.contains("isAA");
}

bool Unit::isArtillerySupportable() const {
    return _information.contains("isArtillerySupportable");
}

bool Unit::isArtillery() const {
    return _information.contains("isArtillery");
}

bool Unit::isAir() const {
    return _information.contains("isAir");
}

bool Unit::isSea() const {
    return _information.contains("isSea");
}

bool Unit::isLand() const {
    return !isSea() && !isAir();
}

bool Unit::canBombard() const {
    return _information.contains("canBombard");
}

bool Unit::isTwoHit() const {
    return _information.contains("isTwoHit");
}

bool Unit::isHit() const {
    return _information.value("isHit", "false") == "true";
}

void Unit::setHit() {
    _information["isHit"] = "true";
}

bool Unit::isDestroyer() const {
    return _information.contains("isDestroyer");
}

bool Unit::isSub() const {
    return _information.contains("isSub");
}

bool Unit::hasTwoRolls() const {
    return (numRolls() == 2);
}

int Unit::numRolls() const {
    return _information.value("NumRolls", "1").toInt();
}

int Unit::bombardmentValue() const {
    return _information.value("Bombard", _information.value("Attack")).toInt();
}

int Unit::numArtillery() const {
    return _information.value("UnitSupportCount", "1").toInt();
}

bool Unit::isMarine() const {
    return static_cast<bool>(_information.value("isMarine", "0").toInt());
}

QString Unit::description() const {
    QString result;
    result += _name + "\n";
    result += "\nAttack: " + _information["Attack"];
    result += "\nDefense: " + _information["Defense"];
    result += "\nIPC: " + _information["IPC"];
    foreach (const QString& key, _information.keys()) {
        if (key == "Attack" || key == "Defense" || key == "IPC")
            continue;

        result += "\n" + key;
        if (!_information[key].isEmpty())
            result += ": " + _information[key];
    }
    return result;
}

UnitLite::UnitLite(const Unit* const unit, int ipcFactor) {
    int id = unit->id();
    int numRolls = unit->numRolls();
    if (id > 63)
        QMessageBox::critical(0, "XML Error", "A maximum number of 63 units is supported");
    if (numRolls > 3)
        QMessageBox::critical(0, "XML Error", "A maximum number of 3 rolls per unit is supported");
    _numRollsAndID = numRolls + 4*id;

    int attackValue = unit->attackValue();
    int defenseValue = unit->defenseValue();
    
    _combatValue = (defenseValue << 3) + attackValue;
    _combatValue2 = (unit->bombardmentValue() << 3) + unit->numArtillery();

    float ipc = unit->ipcValue();
    _ipc = static_cast<unsigned char>(ipc * ipcFactor);

    _features = 0;

    if (unit->isArtillery())
        _features |= FeaturesIsArtillery;
    if (unit->isArtillerySupportable())
        _features |= FeaturesIsArtillerySupportable;
    if (unit->isTwoHit())
        _features |= FeaturesIsTwoHit;
    if (unit->isAA())
        _combatValue |= AABITMASK;
    if (unit->isAir())
        _features |= FeaturesIsAir;
    if (unit->isSea())
        _features |= FeaturesIsSea;
    if (unit->canBombard())
        _features |= FeaturesCanBombard;
    if (unit->isDestroyer())
        _features |= FeaturesIsDestroyer;
    if (unit->isSub())
        _features |= FeaturesIsSub;
    if (unit->isMarine())
        _combatValue2 |= MARINEBITMASK;
}

bool UnitLite::operator==(const UnitLite& rhs) const {
    return (this->id() == rhs.id());
}

bool UnitLite::operator!=(const UnitLite& rhs) const {
    return !(*this == rhs);
}

int UnitLite::id() const {
    return _numRollsAndID / 4;
}

int UnitLite::attackValue() const {
    return _combatValue & ATTACKBITMASK;
}

int UnitLite::defenseValue() const {
    return (_combatValue & DEFENSEBITMASK) >> 3;
}

float UnitLite::ipcValue() const {
    return _ipc;
}

bool UnitLite::isAA() const {
    return _combatValue & AABITMASK;
}

bool UnitLite::isArtillerySupportable() const {
    return _features & FeaturesIsArtillerySupportable;
}

bool UnitLite::isArtillery() const {
    return _features & FeaturesIsArtillery;
}

bool UnitLite::isAir() const {
    return _features & FeaturesIsAir;
}

bool UnitLite::isSea() const {
    return _features & FeaturesIsSea;
}

bool UnitLite::isLand() const {
    return !(_features & (FeaturesIsSea | FeaturesIsAir));
}

bool UnitLite::canBombard() const {
    return _features & FeaturesCanBombard;
}

bool UnitLite::isTwoHit() const {
    return _features & FeaturesIsTwoHit;
}

bool UnitLite::isHit() const {
    return _combatValue & HITBITMASK;
}

void UnitLite::setHit() {
    _combatValue |= HITBITMASK;
}

bool UnitLite::isDestroyer() const {
    return _features & FeaturesIsDestroyer;
}

bool UnitLite::isSub() const {
    return _features & FeaturesIsSub;
}

bool UnitLite::hasTwoRolls() const {
    return numRolls() == 2;
}

int UnitLite::numRolls() const {
    return _numRollsAndID % 4;
}

int UnitLite::bombardmentValue() const {
    return _combatValue2 & BOMBARDMENTBITMASK;
}

int UnitLite::numArtillery() const {
    return _combatValue2 & SUPPORTBITMASK;
}

bool UnitLite::isMarine() const {
    return _combatValue2 & MARINEBITMASK;
}
