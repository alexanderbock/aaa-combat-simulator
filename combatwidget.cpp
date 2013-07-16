#include "combatwidget.h"

#include "controlwidget.h"
#include "factionwidget.h"
#include "simulatorapplication.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QHBoxLayout>
#include <QIcon>
#include <QMessageBox>
#include <QtConcurrentRun>
#include <QVBoxLayout>

#include <QDateTime>
#include <iostream>
#include <math.h>
#include <time.h>

CombatWidget::CombatWidget(const QString& directory, QWidget* parent)
    : QWidget(parent)
    , attackerWidget_(0)
    , defenderWidget_(0)
    //, doppelschlagWidget_(0)
    , controlWidget_(0)
    , attackerLayout_(0)
    , directory_(directory)
    , ipcFactor_(1)
{
    initXML(directory + "/" + directory + ".xml");

    QHBoxLayout* layout = new QHBoxLayout(this);
    attackerLayout_ = new QVBoxLayout;
    
    controlWidget_ = new ControlWidget(this);

    attackerWidget_ = new FactionWidget(this, "Attacker", FactionSideAttacker);
    attackerLayout_->addWidget(attackerWidget_);
    layout->addLayout(attackerLayout_);

    defenderWidget_ = new FactionWidget(this, "Defender", FactionSideDefender);
    layout->addWidget(defenderWidget_);

    connect(controlWidget_, SIGNAL(landBattleCheckboxDidChange()), attackerWidget_, SLOT(recreateUnits()));
    connect(controlWidget_, SIGNAL(landBattleCheckboxDidChange()), defenderWidget_, SLOT(recreateUnits()));
    connect(controlWidget_, SIGNAL(switchSides()), this, SLOT(switchCombatSides()));
    connect(controlWidget_, SIGNAL(startCombat()), this, SLOT(startCombat()));
    connect(controlWidget_, SIGNAL(clear()), this, SLOT(clear()));
    layout->addWidget(controlWidget_);
}

CombatWidget::~CombatWidget() {
    qDeleteAll(units_);
}


void CombatWidget::initXML(const QString& xmlFile) {
    QDomDocument doc("document");
    QFile file(xmlFile);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, "File Error", "Could not find the file '" + xmlFile + "'");
        return;
    }
    QString errorMessage;
    int errorLine;
    if (!doc.setContent(&file, &errorMessage, &errorLine)) {
        QMessageBox::critical(0, "XML Error", errorMessage + "\n" + QString::number(errorLine));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomElement unitsElem = docElem.firstChildElement("Units");
    if (unitsElem.isNull()) {
        QMessageBox::critical(0, "XML Error", "Could not find 'Units' tag in XML file '" + xmlFile + "'");
        return;
    }
    QDomNodeList units = unitsElem.childNodes();
    for (int i = 0 ; i < units.size() ; ++i) {
        const QDomNode& unit = units.at(i);
        QDomElement elem = units.at(i).toElement();
        if (elem.isNull()) {
            QMessageBox::critical(0, "XML Error", "XML format error in node '" + unit.nodeName() + "'");
            return;
        }
        Unit* u = new Unit(elem);
        units_.append(u);
        idMap_.insert(u->getID(), u);

        float ipc = u->getIPC();
        ipc -= static_cast<int>(ipc);
        if (ipc != 0.f)
            ipcFactor_ = 2;
        //if (ipc != static_cast<float>(static_cast<int>(ipc)))
    }

    QDomElement factionsElem = docElem.firstChildElement("Factions");
    if (factionsElem.isNull()) {
        QMessageBox::critical(0, "XML Error", "Could not find 'Factions' tag in XML file '" + xmlFile + "'");
        return;
    }
    QDomNodeList groups = factionsElem.childNodes();
    for (int i = 0; i < groups.size(); ++i) {
        const QDomNode& group = groups.at(i);
        QString groupName = group.nodeName();
        QStringList f;
        QDomNodeList factions = group.childNodes();
        for (int j = 0; j < factions.size(); ++j) {
            const QDomNode& faction = factions.at(j);
            QString factionName = faction.nodeName();
            f.append(factionName);
            factions_.append(factionName);
        }
        factionsDetail_.insert(groupName, f);
    }
}

QStringList CombatWidget::getFactions() const {
    return factions_;
}

QStringList CombatWidget::getGroups() const {
    return factionsDetail_.keys();
}

QStringList CombatWidget::getFactionsForGroup(const QString& group) const {
    return factionsDetail_.value(group);
}

QList<Unit*> CombatWidget::getUnits() const {
    return units_;
}

QIcon CombatWidget::getFlag(const QString& faction) const {
    return QIcon(directory_ + "/" + faction + "/" + faction + ".png");
}

QPixmap CombatWidget::getUnitIcon(const QString& faction, int unitID) const {
    QString unitName = getNameForID(unitID);
    return QPixmap(directory_ + "/" + faction + "/" + unitName + ".png");
}

QString CombatWidget::getNameForID(int id) const {
    return idMap_[id]->getName();
}

bool CombatWidget::isLandBattle() const {
    return controlWidget_->isLandBattle();
}

bool CombatWidget::isAmphibiousCombat() const {
    return controlWidget_->isAmphibiousCombat();
}

bool CombatWidget::landUnitMustLive() const {
    return controlWidget_->landUnitMustLive();
}

OrderOfLoss CombatWidget::orderOfLoss() const {
    return controlWidget_->orderOfLoss();
}

const QString& CombatWidget::getDirectory() const {
    return directory_;
}

void CombatWidget::switchCombatSides() {
    QList<QPair<Unit*, int> > attackerUnits = attackerWidget_->getUnits();
    QList<QPair<Unit*, int> > defenderUnits = defenderWidget_->getUnits();

    QString faction = attackerWidget_->getFaction();
    attackerWidget_->setFaction(defenderWidget_->getFaction());
    defenderWidget_->setFaction(faction);

    attackerWidget_->setUnits(defenderUnits);
    defenderWidget_->setUnits(attackerUnits);
}

void CombatWidget::clear() {
    attackerWidget_->clear();
    defenderWidget_->clear();
}

QList<CombatThread*> CombatWidget::startCombat(const QList<UnitLite>& attackerUnits, const QList<UnitLite>& defenderUnits) {
    attackerWidget_->clearResults();
    defenderWidget_->clearResults();
    qApp->processEvents();

    QList<CombatThread*> results;
    for (int i = 0; i < 30000; ++i) {
        CombatThread* result = new CombatThread(attackerUnits, defenderUnits, isLandBattle(), isAmphibiousCombat(), landUnitMustLive(), orderOfLoss(), qrand());
        results.append(result);
        QThreadPool::globalInstance()->start(result);
    }
    QThreadPool::globalInstance()->waitForDone();

    return results;
}

CombatWidget::CombatResult CombatWidget::computeCombatResults(const QList<CombatThread*>& results) {
    int attackerWins = 0;
    int averageAttackerUnit = 0;
    int averageAttackerIPCLoss = 0;
    int defenderWins = 0;
    int averageDefenderUnit = 0;
    int averageDefenderIPCLoss = 0;
    int draw = 0;
    foreach (const CombatThread* result, results) {
        const Batallion& attacker = result->getAttacker();
        const Batallion& attackerCas = result->getAttackerCasualities();
        const Batallion& defender = result->getDefender();
        const Batallion& defenderCas = result->getDefenderCasualities();
        
        averageAttackerUnit += (attacker.size());
        
        foreach (const UnitLite& unit, attackerCas)
            averageAttackerIPCLoss += unit.getIPC();
        
        averageDefenderUnit += (defender.size());
        
        foreach (const UnitLite& unit, defenderCas)
            averageDefenderIPCLoss += unit.getIPC();
        
        if ((attacker.size() == 0) && (defender.size() > 0))
            defenderWins++;
        else if ((attacker.size() > 0) && (defender.size() == 0))
            attackerWins++;
        else
            draw++;
    }

    float attIpc = static_cast<float>(averageAttackerIPCLoss) / static_cast<float>(ipcFactor_);
    float defIpc = static_cast<float>(averageDefenderIPCLoss) / static_cast<float>(ipcFactor_);
    
    CombatResult result;
    result.attackerWins = static_cast<float>(attackerWins)/results.size();
    result.defenderWins = static_cast<float>(defenderWins)/results.size();
    result.draw = static_cast<float>(draw)/results.size();
    result.averageAttackerIPC = attIpc/results.size();
    result.averageAttackerUnit = static_cast<float>(averageAttackerUnit)/results.size();
    result.averageDefenderIPC = defIpc/results.size();
    result.averageDefenderUnit = static_cast<float>(averageDefenderUnit)/results.size();
    return result;
}

//#define TIMING

void CombatWidget::startCombat() {
    QList<UnitLite> attackerUnits;
    
    QPair<Unit*, int> p;
    foreach (p, attackerWidget_->getUnits()) {
        for (int i = 0; i < p.second; ++i)
            attackerUnits.append(UnitLite(p.first, ipcFactor_));
    }
    QList<UnitLite> defenderUnits;
    foreach (p, defenderWidget_->getUnits()) {
        for (int i = 0; i < p.second; ++i)
            defenderUnits.append(UnitLite(p.first, ipcFactor_));
    }

    qsrand(QDateTime::currentMSecsSinceEpoch());

#ifdef TIMING
    QTime t;
    t.start();
#endif

    QList<CombatThread*> results = startCombat(attackerUnits, defenderUnits);

#ifdef TIMING
    int combatTime = t.elapsed();
#endif

    CombatResult combatResult = computeCombatResults(results);

#ifdef TIMING
    int computeTime = t.elapsed();
#endif

#ifdef TIMING
    qDebug("Time elapsed (Combat): %d ms", combatTime);
    qDebug("Time elapsed (Result): %d ms", computeTime- combatTime);
#endif
    
    attackerWidget_->setResults(&results, combatResult.attackerWins, combatResult.draw, combatResult.attackerWins > combatResult.defenderWins,
        combatResult.averageAttackerUnit, attackerUnits.size(), combatResult.averageAttackerIPC);

    defenderWidget_->setResults(&results, combatResult.defenderWins, combatResult.draw, combatResult.defenderWins > combatResult.attackerWins, 
        combatResult.averageDefenderUnit, defenderUnits.size(), combatResult.averageDefenderIPC);
}
