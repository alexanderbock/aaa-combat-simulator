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

    int getID() const;
    QString getName() const;
    int getAttack() const;
    int getDefense() const;
    float getIPC() const;
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
    int getNumRolls() const;
    int getBombardmentValue() const;
    int getNumArtillery() const;
    bool isMarine() const;
    
    QString getDescription() const;

protected:
    int id_;
    QString name_;

    QMap<QString, QString> information_;
};

struct UnitLite {
public:
    UnitLite(const Unit* const unit, int ipcFactor);

    bool operator==(const UnitLite& rhs) const;
    bool operator!=(const UnitLite& rhs) const;

    int getID() const;
    int getAttack() const;
    int getDefense() const;
    float getIPC() const;
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
    int getNumRolls() const;
    int getBombardmentValue() const;
    int getNumArtillery() const;
    bool isMarine() const;

private:
    unsigned char ipc_;
    unsigned char features_;
    unsigned char numRollsAndID_; // bits: ididid##      id;num
    unsigned char combatValue_;   // bits: ##defatt     isHit;isAA;defense;attack
    unsigned char combatValue2_;  // bits: ##bomsup      ?;isMarine;bombardment value; #of supported units

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
