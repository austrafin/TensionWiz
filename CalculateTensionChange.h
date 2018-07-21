#ifndef CALCULATETENSIONCHANGE_H
#define CALCULATETENSIONCHANGE_H

#include <QTableWidget>

class CalculateTensionChange
{
public:
    //CalculateTensionChange();
    void setInitialTension(const double tension);
    void setInitialWind(const double wind);
    void setFinalWind(const double wind);
    void setInitialTemperature(const double temperature);
    void setSpans(const QVector<double> &newSpans);
    void setConductorDiameter(const double diameter);
    void setModulusOfElasticy(const double moeNew);
    void setCoefficientOfExpansion(const double coeNew);
    void setCrossSectionalArea(const double csaNew);
    void setGravity(const double value);
    void setMass(const double value);

    double calculateTensionChange(const double tension, const double weightNoWind, const double weightWithWind, const double rulingSpan, const double tempFinal);
    double calcSag(const double tension, const double span);
    double calcSwing(const double tension, const double span, const double swingWind);
    double calculateWind(const double wind);   
    double getInitialTension();
    double getInitialWind();
    double getFinalWind();
    double getInitialTemperature();
    static double calculateRulingSpan(const QVector<double> &spansRS);
    QVector<double> getResults(const QTableWidget *table, const int actualSpanIndex, const int tableIndex, double settlingIn); // Don't reference settlingIn
    QVector<double> getSpans();

private:
    double calculateConductorWeight();
    double initialTension;
    double initialWind;
    double finalWind;
    double initialTemperature;
    double conductorDiameter;
    double mass;
    double moe;
    double csa;
    double coe;
    double gravity;
    QVector<double> spans;
};

#endif // CALCULATETENSIONCHANGE_H
