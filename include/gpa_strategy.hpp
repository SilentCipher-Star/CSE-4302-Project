#pragma once
#include <QString>

namespace Acadence {

class IGPAStrategy
{
public:
    virtual ~IGPAStrategy() = default;
    virtual double calculate(double percentage) const = 0;
    virtual QString schemeName() const = 0;
};

class PercentageGPAStrategy : public IGPAStrategy
{
public:
    double calculate(double pct) const override
    {
        if (pct >= 90.0) return 4.0;
        if (pct >= 85.0) return 3.7;
        if (pct >= 80.0) return 3.3;
        if (pct >= 75.0) return 3.0;
        if (pct >= 70.0) return 2.7;
        if (pct >= 65.0) return 2.3;
        if (pct >= 60.0) return 2.0;
        if (pct >= 55.0) return 1.7;
        if (pct >= 50.0) return 1.3;
        return 0.0;
    }
    QString schemeName() const override { return "4.0 Scale (Percentage-based)"; }
};

class LetterGradeGPAStrategy : public IGPAStrategy
{
public:
    double calculate(double pct) const override
    {
        if (pct >= 90.0) return 4.0;
        if (pct >= 85.0) return 4.0;
        if (pct >= 80.0) return 3.5;
        if (pct >= 75.0) return 3.0;
        if (pct >= 70.0) return 3.0;
        if (pct >= 65.0) return 2.5;
        if (pct >= 60.0) return 2.0;
        if (pct >= 55.0) return 2.0;
        if (pct >= 50.0) return 1.0;
        return 0.0;
    }
    QString schemeName() const override { return "Letter Grade (A/B/C/D/F)"; }

    QString getLetterGrade(double pct) const
    {
        if (pct >= 90.0) return "A+";
        if (pct >= 85.0) return "A";
        if (pct >= 80.0) return "A-";
        if (pct >= 75.0) return "B+";
        if (pct >= 70.0) return "B";
        if (pct >= 65.0) return "B-";
        if (pct >= 60.0) return "C+";
        if (pct >= 55.0) return "C";
        if (pct >= 50.0) return "D";
        return "F";
    }
};

} // namespace Acadence
