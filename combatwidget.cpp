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
    , _attackerWidget(nullptr)
    , _defenderWidget(nullptr)
    , _controlWidget(nullptr)
    , _attackerLayout(nullptr)
    , _directory(directory)
    , _ipcFactor(1)
{
    initXML(directory + "/" + directory + ".xml");

    QHBoxLayout* layout = new QHBoxLayout(this);
    _attackerLayout = new QVBoxLayout;
    
    _controlWidget = new ControlWidget(this);

    _attackerWidget = new FactionWidget(this, "Attacker", FactionSideAttacker);
    _attackerLayout->addWidget(_attackerWidget);
    layout->addLayout(_attackerLayout);

    _defenderWidget = new FactionWidget(this, "Defender", FactionSideDefender);
    layout->addWidget(_defenderWidget);

    connect(_controlWidget, SIGNAL(landBattleCheckboxDidChange()), _attackerWidget, SLOT(recreateUnits()));
    connect(_controlWidget, SIGNAL(landBattleCheckboxDidChange()), _defenderWidget, SLOT(recreateUnits()));
    connect(_controlWidget, SIGNAL(switchSides()), this, SLOT(switchCombatSides()));
    connect(_controlWidget, SIGNAL(startCombat()), this, SLOT(startCombat()));
    connect(_controlWidget, SIGNAL(clear()), this, SLOT(clear()));
    layout->addWidget(_controlWidget);
}

CombatWidget::~CombatWidget() {
    qDeleteAll(_units);
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
        _units.append(u);
        _idMap.insert(u->id(), u);

        float ipc = u->ipcValue();
        ipc -= static_cast<int>(ipc);
        if (ipc != 0.f)
            _ipcFactor = 2;
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
            _factions.append(factionName);
        }
        _factionsDetail.insert(groupName, f);
    }
}

QStringList CombatWidget::factions() const {
    return _factions;
}

QStringList CombatWidget::groups() const {
    return _factionsDetail.keys();
}

QStringList CombatWidget::factionsForGroup(const QString& group) const {
    return _factionsDetail.value(group);
}

QList<Unit*> CombatWidget::units() const {
    return _units;
}

QIcon CombatWidget::flag(const QString& faction) const {
    return QIcon(_directory + "/" + faction + "/" + faction + ".png");
}

QPixmap CombatWidget::unitIcon(const QString& faction, int unitID) const {
    QString unitName = nameForID(unitID);
    return QPixmap(_directory + "/" + faction + "/" + unitName + ".png");
}

QString CombatWidget::nameForID(int id) const {
    return _idMap[id]->name();
}

bool CombatWidget::isLandBattle() const {
    return _controlWidget->isLandBattle();
}

bool CombatWidget::isAmphibiousCombat() const {
    return _controlWidget->isAmphibiousCombat();
}

bool CombatWidget::landUnitMustLive() const {
    return _controlWidget->landUnitMustLive();
}

OrderOfLoss CombatWidget::orderOfLoss() const {
    return _controlWidget->orderOfLoss();
}

const QString& CombatWidget::directory() const {
    return _directory;
}

void CombatWidget::switchCombatSides() {
    QList<QPair<Unit*, int> > attackerUnits = _attackerWidget->getUnits();
    QList<QPair<Unit*, int> > defenderUnits = _defenderWidget->getUnits();

    QString faction = _attackerWidget->faction();
    _attackerWidget->setFaction(_defenderWidget->faction());
    _defenderWidget->setFaction(faction);

    _attackerWidget->setUnits(defenderUnits);
    _defenderWidget->setUnits(attackerUnits);
}

void CombatWidget::clear() {
    _attackerWidget->clear();
    _defenderWidget->clear();
}

QList<CombatThread*> CombatWidget::startCombat(const QList<UnitLite>& attackerUnits, const QList<UnitLite>& defenderUnits) {
    _attackerWidget->clearResults();
    _defenderWidget->clearResults();
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
        const Batallion& attacker = result->attacker();
        const Batallion& attackerCas = result->attackerCasualities();
        const Batallion& defender = result->defender();
        const Batallion& defenderCas = result->defenderCasualities();
        
        averageAttackerUnit += (attacker.size());
        
        foreach (const UnitLite& unit, attackerCas)
            averageAttackerIPCLoss += unit.ipcValue();
        
        averageDefenderUnit += (defender.size());
        
        foreach (const UnitLite& unit, defenderCas)
            averageDefenderIPCLoss += unit.ipcValue();
        
        if ((attacker.size() == 0) && (defender.size() > 0))
            ++defenderWins;
        else if ((attacker.size() > 0) && (defender.size() == 0))
            ++attackerWins;
        else
            ++draw;
    }

    float attIpc = static_cast<float>(averageAttackerIPCLoss) / static_cast<float>(_ipcFactor);
    float defIpc = static_cast<float>(averageDefenderIPCLoss) / static_cast<float>(_ipcFactor);
    
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
    foreach (p, _attackerWidget->getUnits()) {
        for (int i = 0; i < p.second; ++i)
            attackerUnits.append(UnitLite(p.first, _ipcFactor));
    }
    QList<UnitLite> defenderUnits;
    foreach (p, _defenderWidget->getUnits()) {
        for (int i = 0; i < p.second; ++i)
            defenderUnits.append(UnitLite(p.first, _ipcFactor));
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
    
    _attackerWidget->setResults(&results, combatResult.attackerWins, combatResult.draw, combatResult.attackerWins > combatResult.defenderWins,
        combatResult.averageAttackerUnit, attackerUnits.size(), combatResult.averageAttackerIPC);

    _defenderWidget->setResults(&results, combatResult.defenderWins, combatResult.draw, combatResult.defenderWins > combatResult.attackerWins, 
        combatResult.averageDefenderUnit, defenderUnits.size(), combatResult.averageDefenderIPC);
}
