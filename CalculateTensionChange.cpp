#include "CalculateTensionChange.h"
#include "SheetSetup.h"

void CalculateTensionChange::setInitialTension(const double tension)
{
    initialTension = tension;
}

void CalculateTensionChange::setInitialWind(const double wind)
{
    initialWind = wind;
}

void CalculateTensionChange::setFinalWind(const double wind)
{
    finalWind = wind;
}

void CalculateTensionChange::setInitialTemperature(const double temperature)
{
    initialTemperature = temperature;
}

void CalculateTensionChange::setSpans(const QVector<double> &newSpans)
{
    spans = newSpans;
}

void CalculateTensionChange::setConductorDiameter(const double diameter)
{
    conductorDiameter = diameter / 1000;
}

void CalculateTensionChange::setModulusOfElasticy(const double moeNew)
{
    moe = moeNew;
}

void CalculateTensionChange::setCoefficientOfExpansion(const double coeNew)
{
    coe = coeNew;
}

void CalculateTensionChange::setCrossSectionalArea(const double csaNew)
{
    csa = csaNew / 1000;
}

void CalculateTensionChange::setGravity(const double value)
{
    gravity = value;
}

void CalculateTensionChange::setMass(const double value)
{
    mass = value;
}

double CalculateTensionChange::getInitialTension()
{
    return initialTension;
}

double CalculateTensionChange::getInitialWind()
{
    return initialWind;
}

double CalculateTensionChange::getFinalWind()
{
    return finalWind;
}

double CalculateTensionChange::getInitialTemperature()
{
    return initialTemperature;
}

QVector<double> CalculateTensionChange::getSpans()
{
    return spans;
}

QVector<double> CalculateTensionChange::getResults(const QTableWidget *table, const int actualSpanIndex, const int tableIndex, double settlingIn)
{

    int row = 0;
    const int totalRows = table->rowCount();
    const int spansTotal = spans.size();
    double actualSpan;

    double temperatureFinal = 0;
    const double rulingSpan = calculateRulingSpan(spans);

    QVector<double> results;

    // If actual span not selected, using ruling span
    if(actualSpanIndex == 0)
        actualSpan = rulingSpan;
    else
        actualSpan = spans[actualSpanIndex - 1];

    if(settlingIn == 0)
        settlingIn = 1;
    else
        settlingIn = settlingIn / 100 + 1; 

    switch(tableIndex)
    {
        case 0:
        {
            const double condWeightNoWind = calculateConductorWeight();
            const double condWeightWithWind = calculateWind(initialWind); // Initial Wind
            const double condWeightWithWindT500 = calculateWind(finalWind); // Final Wind
            const double settlingInKG = settlingIn / gravity; // Convert Newtons to kilograms (Newtons divated by Earth's gravity)

            double tensionNWtemp;
            double tensionWind500temp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionNWtemp = calculateTensionChange(initialTension, condWeightWithWind, condWeightNoWind, rulingSpan, temperatureFinal);
                tensionWind500temp = calculateTensionChange(initialTension, condWeightWithWind, condWeightWithWindT500, rulingSpan, temperatureFinal);

                // Setting Tension cells for Stringing Chart
                results.push_back(tensionNWtemp * settlingIn);
                results.push_back(tensionWind500temp * settlingIn);
                results.push_back(tensionNWtemp * settlingInKG);
                results.push_back(tensionWind500temp * settlingInKG);

                // Setting Sag cells.
                results.push_back(calcSag(tensionNWtemp * settlingIn, actualSpan));
                results.push_back(calcSag(tensionWind500temp * settlingIn, actualSpan));
            }
        }
        break;

        case 1:
        {
            const double condWeightNoWind = calculateConductorWeight();
            const double condWeightWithWind = calculateWind(initialWind);

            double tensionNWtemp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionNWtemp = calculateTensionChange(initialTension, condWeightWithWind, condWeightNoWind, rulingSpan, temperatureFinal) * settlingIn;
                // Setting Tension cells for No Wind
                results.push_back(tensionNWtemp);

                // Setting Sag cells.
                results.push_back(calcSag(tensionNWtemp, actualSpan));
            }
        }
        break;

        // Initial Wind
        case 2:
        {
            const double condWeightWithWind = calculateWind(initialWind);

            double tensionWindtemp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionWindtemp = calculateTensionChange(initialTension, condWeightWithWind, condWeightWithWind, rulingSpan, temperatureFinal) * settlingIn;

                // Setting Tension cells for Init Wind
                results.push_back(tensionWindtemp);

                // Setting Sag & Swing cells.
                results.push_back( calcSag(tensionWindtemp, actualSpan) );
                results.push_back( calcSwing(tensionWindtemp, actualSpan, initialWind) );
            }
        }
        break;

        // T + final wind
        case 3:
        {
            const double condWeightWithWind = calculateWind(initialWind);
            const double condWeightWithWindT500 = calculateWind(finalWind);

            double tensionWind500temp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionWind500temp = calculateTensionChange(initialTension, condWeightWithWind, condWeightWithWindT500, rulingSpan, temperatureFinal) * settlingIn;

                // Setting Tension cells for Final Wind
                results.push_back(tensionWind500temp);

                // Setting Sag & Swing cells.
                results.push_back(calcSag(tensionWind500temp, actualSpan));
                results.push_back(calcSwing(tensionWind500temp, actualSpan, finalWind));

            }
        }
        break;

        // Sags No Wind
        case 4:
        {
            const double condWeightNoWind = calculateConductorWeight();
            const double condWeightWithWind = calculateWind(initialWind);

            double tensionNWtemp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionNWtemp = calculateTensionChange(initialTension, condWeightWithWind, condWeightNoWind, rulingSpan, temperatureFinal) * settlingIn;

                // Setting Sag cells.
                for(int i = 0; i < spansTotal; i++)
                    results.push_back(calcSag(tensionNWtemp, spans[i]));
            }
        }
        break;

        // Sags Init Wind
        case 5:
        {
            const double condWeightWithWind = calculateWind(initialWind);

            double tensionWindtemp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionWindtemp = calculateTensionChange(initialTension, condWeightWithWind, condWeightWithWind, rulingSpan, temperatureFinal) * settlingIn;

                for(int i = 0; i < spansTotal; i++)
                    results.push_back(calcSag(tensionWindtemp, spans[i]));
            }
        }
        break;

        // Sags T + Final Wind
        case 6:
        {
            const double condWeightWithWind = calculateWind(initialWind);
            const double condWeightWithWindT500 = calculateWind(finalWind);

            double tensionWind500temp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionWind500temp = calculateTensionChange(initialTension, condWeightWithWind, condWeightWithWindT500, rulingSpan, temperatureFinal) * settlingIn;

                for(int i = 0; i < spansTotal; i++)
                    results.push_back(calcSag(tensionWind500temp, spans[i]));
            }
        }
        break;

        case 7:
        {
            const double condWeightWithWind = calculateWind(initialWind);

            double tensionWindtemp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionWindtemp = calculateTensionChange(initialTension, condWeightWithWind, condWeightWithWind, rulingSpan, temperatureFinal) * settlingIn;

                for(int i = 0; i < spansTotal; i++)
                    results.push_back(calcSwing(tensionWindtemp, spans[i], initialWind));
            }
        }
        break;

        case 8:
        {
            const double condWeightWithWind = calculateWind(initialWind);
            const double condWeightWithWindT500 = calculateWind(finalWind);

            double tensionWind500temp;

            for(row = 0; row < totalRows; row++)
            {
                temperatureFinal = SheetSetup::getTemperatureFromString(table->verticalHeaderItem(row)->text());
                tensionWind500temp = calculateTensionChange(initialTension, condWeightWithWind, condWeightWithWindT500, rulingSpan, temperatureFinal) * settlingIn;

                for(int i = 0; i < spansTotal; i++)
                    results.push_back(calcSwing(tensionWind500temp, spans[i], finalWind));
            }
        }
        break;
    }
    return results;
}

double CalculateTensionChange::calculateRulingSpan(const QVector<double> &spansRS)
{
    double x1 = 0;
    double x2 = 0;
    const int totalSpans = spansRS.size();
    for(int i = 0; i < totalSpans; i++)
    {
        x1 += pow(spansRS[i], 3);
        x2 += spansRS[i];
    }
    return sqrt(x1/x2);
}

double CalculateTensionChange::calculateWind(const double wind)
{
    return sqrt(pow(calculateConductorWeight(), 2) + pow((conductorDiameter * wind), 2));
}

double CalculateTensionChange::calculateConductorWeight()
{
    // kg/m * gravity
    return mass * gravity;
}

double CalculateTensionChange::calculateTensionChange(const double tension, const double weightNoWind, const double weightWithWind, const double rulingSpan, const double tempFinal)
{
    // x^3 - ax^2 - c = 0
    const double a = -(pow( (sqrt(moe * csa / 24) * weightNoWind * rulingSpan / tension) , 2) - tension) + ( coe * csa * moe / 1000000 * (initialTemperature - tempFinal) );
    double function;
    double dFunction; // derivate

    double finalTension = tension;
    double compare;
    int loopcount = 0;
    int loopMax = 100000;
    int x = 10;

    do
    {
        if(loopcount == loopMax - 1)
        {
            finalTension = tension + x;
            x = finalTension;
            loopcount = 0;
        }
        compare = finalTension;
        function = (pow(finalTension, 3) - (a * pow(finalTension, 2)) - ( pow((sqrt(moe * csa / 24) * weightWithWind * rulingSpan), 2) ));
        dFunction = 3 * pow(finalTension, 2) - (2 * a * finalTension);
        finalTension = finalTension -  (function / dFunction);
        loopcount++;
    }
    while((loopcount < loopMax) && finalTension < compare - 0.00000001 || finalTension > compare + 0.00000001);

    return finalTension;
}

double CalculateTensionChange::calcSag(const double tension, const double span)
{
    // Catenary calculation.
    const double conductorWeight = calculateConductorWeight();
    return tension / conductorWeight * (cosh(span / (2 * (tension / conductorWeight))) - 1);
}

double CalculateTensionChange::calcSwing(const double tension, const double span, const double swingWind)
{
    return ((conductorDiameter * swingWind) * pow(span, 2)) / (8 * tension);
}
