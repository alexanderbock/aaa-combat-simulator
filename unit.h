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

#ifndef BOCK_UNIT_H
#define BOCK_UNIT_H

#include <QDomElement>
#include <QMap>
#include <QString>

class Unit {
public:
    Unit() {}
    Unit(const QDomElement& element);

    bool operator==(const Unit& rhs) const;
    bool operator!=(const Unit& rhs) const;

    int id() const;
    QString name() const;
    int attackValue() const;
    int defenseValue() const;
    float ipcValue() const;
    bool canAttack() const;
    bool isAA() const;
    bool isArtillerySupportable() const;
    bool isArtillery() const;
    bool isAir() const;
    bool isSea() const;
    bool isLand() const;
    bool canBombard() const;
    bool isTwoHit() const;
    bool isHit() const;
    void setHit();
    bool isDestroyer() const;
    bool isSub() const;
    bool hasTwoRolls() const;
    int numRolls() const;
    int bombardmentValue() const;
    int numArtillery() const;
    bool isMarine() const;
    
    QString description() const;

protected:
    int _id;
    QString _name;

    QMap<QString, QString> _information;
};

struct UnitLite {
public:
    UnitLite(const Unit* const unit, int ipcFactor);

    bool operator==(const UnitLite& rhs) const;
    bool operator!=(const UnitLite& rhs) const;

    int id() const;
    int attackValue() const;
    int defenseValue() const;
    float ipcValue() const;
    bool canAttack() const;
    bool isAA() const;
    bool isArtillerySupportable() const;
    bool isArtillery() const;
    bool isAir() const;
    bool isSea() const;
    bool isLand() const;
    bool canBombard() const;
    bool isTwoHit() const;
    bool isHit() const;
    void setHit();
    bool isDestroyer() const;
    bool isSub() const;
    bool hasTwoRolls() const;
    int numRolls() const;
    int bombardmentValue() const;
    int numArtillery() const;
    bool isMarine() const;

private:
    unsigned char _ipc;
    unsigned char _features;
    unsigned char _numRollsAndID; // bits: ididid##      id;num
    unsigned char _combatValue;   // bits: ##defatt     isHit;isAA;defense;attack
    unsigned char _combatValue2;  // bits: ##bomsup      ?;isMarine;bombardment value; #of supported units

    enum Features {
        FeaturesIsArtillery             = 1 << 0,
        FeaturesIsArtillerySupportable  = 1 << 1,
        FeaturesIsTwoHit                = 1 << 2,
        FeaturesIsAir                   = 1 << 3,
        FeaturesIsSea                   = 1 << 4,
        FeaturesCanBombard              = 1 << 5,
        FeaturesIsDestroyer             = 1 << 6,
        FeaturesIsSub                   = 1 << 7
    };
};

#endif
